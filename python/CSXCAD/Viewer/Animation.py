# -*- coding: utf-8 -*-
#
# Copyright (C) 2026 Thorsten Liebig (Thorsten.Liebig@gmx.de)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""CSXCAD.Viewer.Animation
Generic, content-agnostic animation clock that items subscribe to.

One global loop of ``N = max(n_steps)`` ticks drives all animations. Tick
``k`` is the loop fraction ``k / N``; an animation with ``n`` steps shows step
``floor(k * n / N)`` and is called only when that number changes, so all
animations run in phase and wrap together.

The clock is frame paced: it advances exactly one tick per rendered frame
and arms the next frame only after the previous one has finished, so no step
is ever skipped. Before playback, every animation gets the chance to prepare
(cache) each of its steps via :meth:`ViewItem.OnPrepareAnimationStep`, one
step per event loop turn.
"""

import os
import time
import warnings

SLOW_CALL = 0.5   # s, a step or prepare call above this shows busy feedback


def MovieWriter(fmt):
    """ The VTK writer class for video format `fmt` (``'mp4'``, ``'ogv'``),
    or None if this VTK build has none. """
    import importlib
    cls = {'mp4': 'vtkFFMPEGWriter', 'ogv': 'vtkOggTheoraWriter'}.get(fmt)
    if cls is None:
        return None
    # the writer's own module, or the movie module of older builds
    for mod in ('vtkIOFFMPEG' if fmt == 'mp4' else 'vtkIOOggTheora', 'vtkIOMovie'):
        try:
            return getattr(importlib.import_module('vtkmodules.' + mod), cls)
        except (ImportError, AttributeError):
            pass
    return None


def ExportFormats():
    """ The animation export formats available here, see :meth:`AnimationClock.Export`. """
    out = ['png']
    try:
        import PIL.Image  # noqa: F401
        out.append('gif')
    except ImportError:
        pass
    return out + [f for f in ('mp4', 'ogv') if MovieWriter(f) is not None]


class Animation:
    """ Handle of one registered animation, returned by ``Viewer.AddAnimation``. """

    def __init__(self, clock, item, n_steps, callback):
        if int(n_steps) < 1:
            raise ValueError('n_steps must be >= 1')
        self._clock = clock
        self.item = item
        self.n_steps = int(n_steps)
        self.callback = callback
        self.current = None
        self.prepared = set()

    def GetNumberOfSteps(self):
        return self.n_steps

    def Invalidate(self, steps=None):
        """ Invalidate(steps=None)

        Forget that steps were prepared (after the item's data changed), so
        they are prepared again before the next playback.
        """
        if steps is None:
            self.prepared.clear()
        else:
            for s in steps:
                self.prepared.discard(s)
        self.current = None

    def Remove(self):
        """ Unregister this animation. """
        self._clock.Remove(self.item)

    def _Call(self, step):
        fn = self.callback if self.callback is not None else self.item.OnAnimationStep
        fn(step)
        self.current = step


class AnimationClock:
    """ The animation clock of one viewer. """

    def __init__(self, viewer):
        self.viewer = viewer
        self.anims = {}         # id(item) -> Animation
        self.tick = 0
        self.fps = 20.0
        self.playing = False
        self.preparing = False
        self._token = 0

    def NumTicks(self):
        return max((a.n_steps for a in self.anims.values()), default=0)

    def Add(self, item, n_steps, callback=None):
        anim = Animation(self, item, n_steps, callback)
        self.anims[id(item)] = anim
        self.tick %= self.NumTicks()
        self._Notify()
        return anim

    def Remove(self, item):
        if self.anims.pop(id(item), None) is not None:
            if not self.anims:
                self.Pause()
                self.tick = 0
            self._Notify()

    def _Notify(self):
        self.viewer.GetHost().AnimationChanged()

    def StepOf(self, anim, tick=None):
        N = self.NumTicks()
        if N == 0:
            return 0
        k = self.tick if tick is None else tick
        return (k * anim.n_steps) // N

    def _Apply(self, force=False):
        """ Call every due, visible animation for the current tick. """
        for anim in list(self.anims.values()):
            if not anim.item.GetVisible():
                continue
            step = self.StepOf(anim)
            if force or step != anim.current:
                t0 = time.perf_counter()
                try:
                    anim._Call(step)
                except Exception as err:
                    self.Pause()
                    warnings.warn('animation step {} of "{}" failed: {!r}'.format(step, anim.item.GetName(), err))
                    return False
                self._Busy(time.perf_counter() - t0, 'animation step {}'.format(step))
        return True

    def _Busy(self, dt, what):
        if dt > SLOW_CALL:
            self._Status('{} took {:.1f} s'.format(what, dt))

    def ItemShown(self, item):
        anim = self.anims.get(id(item))
        if anim is not None:
            anim.current = None
            self._Apply()

    def SetTick(self, tick):
        N = self.NumTicks()
        # without animations keep it, so a state applied first still works
        self.tick = int(tick) % N if N else int(tick)
        self._Apply()
        self._Notify()

    def Step(self, delta=1):
        self.SetTick(self.tick + int(delta))

    # -- preparation
    def _Pending(self):
        out = []
        for anim in self.anims.values():
            if anim.callback is not None:
                continue   # plain callbacks have no prepare hook
            for s in range(anim.n_steps):
                if s not in anim.prepared:
                    out.append((anim, s))
        return out

    def _PrepareOne(self, anim, step):
        t0 = time.perf_counter()
        anim.item.OnPrepareAnimationStep(step)
        anim.prepared.add(step)
        self._Busy(time.perf_counter() - t0, 'preparing step {}'.format(step))

    def Prepare(self, progress=None):
        """ Prepare all pending steps synchronously (offscreen/export). """
        pending = self._Pending()
        for i, (anim, s) in enumerate(pending):
            if progress is not None:
                progress(i, len(pending))
            self._PrepareOne(anim, s)
        if progress is not None and pending:
            progress(len(pending), len(pending))

    # -- playback
    def Play(self):
        if not self.anims or self.playing or self.preparing:
            return
        self._token += 1
        token = self._token
        pending = self._Pending()
        if pending:
            self.preparing = True
            self._Notify()
            self._PrepareStep(token, pending, 0)
        else:
            self._Start(token)

    def _PrepareStep(self, token, pending, i):
        # one step per event loop turn; without an event loop (offscreen)
        # everything right away
        while True:
            if token != self._token:
                return
            if i >= len(pending):
                self.preparing = False
                self._Status('')
                self._Start(token)
                return
            anim, s = pending[i]
            self._Status('preparing animation {}/{}'.format(i + 1, len(pending)))
            try:
                self._PrepareOne(anim, s)
            except Exception as err:
                self.preparing = False
                self._Notify()
                warnings.warn('preparing step {} of "{}" failed: {!r}'.format(s, anim.item.GetName(), err))
                return
            i += 1
            if self.viewer._Schedule(0, lambda: self._PrepareStep(token, pending, i)):
                return

    def _Status(self, msg):
        self.viewer.GetHost().ShowStatus(msg)

    def _Start(self, token):
        self.playing = True
        self._Notify()
        self._Frame(token)

    def _Frame(self, token):
        if token != self._token or not self.playing:
            return
        t0 = time.perf_counter()
        if not self._Apply():
            return
        self.viewer.Render()
        self._Notify()
        dt = time.perf_counter() - t0
        wait = max(0.0, 1.0 / max(self.fps, 1e-3) - dt)

        def next_frame():
            if token != self._token or not self.playing:
                return
            N = self.NumTicks()
            self.tick = (self.tick + 1) % N if N else 0
            self._Frame(token)
        if not self.viewer._Schedule(wait * 1000, next_frame):
            self.playing = False   # offscreen: no clock, step explicitly
            self._Notify()

    def Pause(self):
        was = self.playing or self.preparing
        self._token += 1
        self.playing = False
        self.preparing = False
        if was:
            self._Status('')
            self._Notify()
            self.viewer._SetFast(False)
            self.viewer.Render()

    def Stop(self):
        self.Pause()
        self.tick = 0
        self._Apply()
        self._Notify()
        self.viewer.Render()

    # -- export
    def Export(self, path, fmt='png', size=None, scale=1, loops=1, progress=None):
        N = self.NumTicks()
        if N == 0:
            raise RuntimeError('no animation registered')
        fmt = fmt.lower()
        writer = None
        frames = []
        if fmt == 'png':
            os.makedirs(path, exist_ok=True)
        elif fmt == 'gif':
            try:
                import PIL.Image  # noqa: F401
            except ImportError:
                raise RuntimeError('gif export needs Pillow (pip install pillow)')
        elif fmt in ('mp4', 'ogv'):
            cls_obj = MovieWriter(fmt)
            if cls_obj is None:
                raise RuntimeError('this VTK build has no video writer for "{}" export'.format(fmt))
            writer = cls_obj()
            writer.SetFileName(str(path))
            writer.SetRate(int(round(self.fps)))
        else:
            raise ValueError('unknown animation format "{}"'.format(fmt))

        was_playing = self.playing
        self.Pause()
        old_tick = self.tick
        self.Prepare(progress)
        from vtkmodules.vtkCommonDataModel import vtkImageData
        from vtkmodules.util.numpy_support import numpy_to_vtk
        from CSXCAD.Viewer.Core import ImageToPNG
        total = N * int(loops)
        try:
            for i in range(total):
                self.tick = i % N
                self._Apply()
                img = self.viewer.Capture(size, scale)
                if progress is not None:
                    progress(i + 1, total)
                if fmt == 'png':
                    ImageToPNG(img, os.path.join(path, 'frame_{:04d}.png'.format(i)))
                elif fmt == 'gif':
                    frames.append(img)
                else:
                    h, w, c = img.shape
                    data = vtkImageData()
                    data.SetDimensions(w, h, 1)
                    data.GetPointData().SetScalars(numpy_to_vtk(img[::-1].reshape(-1, c).copy(), deep=1))
                    writer.SetInputData(data)
                    if i == 0:
                        writer.Start()
                    writer.Write()
        finally:
            if writer is not None:
                writer.End()
            self.tick = old_tick
            self._Apply()
        if fmt == 'gif':
            from PIL import Image
            ims = [Image.fromarray(f) for f in frames]
            ims[0].save(path, save_all=True, append_images=ims[1:], loop=0,
                        duration=int(round(1000.0 / max(self.fps, 1e-3))))
        if was_playing:
            self.Play()
        return path
