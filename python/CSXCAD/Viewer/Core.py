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

"""CSXCAD.Viewer.Core
The :class:`Viewer`: render window (on or off screen), camera, view state.
"""

import contextlib
import copy
import json
import math
import os
import subprocess
import sys
import time
import warnings
import weakref

import numpy as np

from vtkmodules.vtkRenderingCore import (vtkRenderer, vtkRenderWindow, vtkRenderWindowInteractor,
                                         vtkWindowToImageFilter, vtkTextActor, vtkCellPicker,
                                         vtkCoordinate, vtkActor)
from vtkmodules.vtkRenderingAnnotation import vtkAxesActor
from vtkmodules.vtkInteractionStyle import vtkInteractorStyleTrackballCamera, vtkInteractorStyleRubberBand2D
from vtkmodules.vtkIOImage import vtkPNGWriter
from vtkmodules.vtkCommonDataModel import vtkImageData
from vtkmodules.util.numpy_support import vtk_to_numpy, numpy_to_vtk
from vtkmodules.util.misc import calldata_type
from vtkmodules.util.vtkConstants import VTK_INT
# make sure the OpenGL implementations are registered
import vtkmodules.vtkRenderingOpenGL2  # noqa: F401
import vtkmodules.vtkRenderingFreeType  # noqa: F401
import vtkmodules.vtkRenderingContextOpenGL2  # noqa: F401

from CSXCAD.Viewer import ViewState
from CSXCAD.Viewer.ViewState import ParseColor, IsRelativeSize, ResolveSize, RelativeSize
from CSXCAD.Viewer.Scene import Scene, ViewItem, EscapeName, JoinPath, SplitPath, UnionBounds
from CSXCAD.Viewer.Animation import AnimationClock

_VIEW_DIRS = {
    # name: (direction from focal point to camera, view up)
    '+x': ((1, 0, 0), (0, 0, 1)),
    '-x': ((-1, 0, 0), (0, 0, 1)),
    '+y': ((0, 1, 0), (0, 0, 1)),
    '-y': ((0, -1, 0), (0, 0, 1)),
    '+z': ((0, 0, 1), (0, 1, 0)),
    '-z': ((0, 0, -1), (0, 1, 0)),
    # isometric from (1,1,1): all axes point towards the viewer, so grid
    # planes at the lowest lines stay in the background
    'iso_z': ((1, 1, 1), (0, 0, 1)),
    'iso_x': ((1, 1, 1), (1, 0, 0)),
    'iso_y': ((1, 1, 1), (0, 1, 0)),
}
_VIEW_ALIASES = {'xy': '+z', 'yz': '+x', 'xz': '-y', 'iso': 'iso_z'}

FAST_PEELS = 2
FAST_BUDGET = 0.040   # s, adaptive budget of a full-quality render
WHEEL_SETTLE_MS = 200
# cursor shapes of vtkRenderWindow.h
VTK_CURSOR_DEFAULT = 0
VTK_CURSOR_SIZEALL = 8


###############################################################################
# offscreen backend selection

_BACKEND = None

_PROBE = r'''
import sys
import vtkmodules.vtkRenderingOpenGL2 as gl
from vtkmodules.vtkRenderingCore import vtkRenderer, vtkRenderWindow, vtkActor, vtkPolyDataMapper
from vtkmodules.vtkFiltersSources import vtkCubeSource
b = sys.argv[1]
rw = {'egl': lambda: gl.vtkEGLRenderWindow(), 'osmesa': lambda: gl.vtkOSOpenGLRenderWindow(),
      'native': lambda: vtkRenderWindow()}[b]()
rw.SetOffScreenRendering(1)
rw.SetSize(8, 8)
ren = vtkRenderer()
rw.AddRenderer(ren)
src = vtkCubeSource()
m = vtkPolyDataMapper()
m.SetInputConnection(src.GetOutputPort())
a = vtkActor()
a.SetMapper(m)
ren.AddActor(a)
rw.Render()
print('CSXCAD_VIEWER_PROBE_OK', rw.GetClassName())
'''


def _Probe(backend):
    try:
        r = subprocess.run([sys.executable, '-c', _PROBE, backend], stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=60)
    except Exception:
        return False
    return r.returncode == 0 and b'CSXCAD_VIEWER_PROBE_OK' in r.stdout


def OffscreenBackend():
    """ OffscreenBackend()

    The offscreen backend used for rendering without a window: ``'egl'``,
    ``'osmesa'`` or ``'native'``. Chosen once per process; set the
    environment variable ``CSXCAD_VIEWER_BACKEND`` to override.

    Each candidate is probed in a short subprocess first, because a broken
    OpenGL setup can crash the process instead of raising.
    """
    global _BACKEND
    if _BACKEND is not None:
        return _BACKEND
    env = os.environ.get('CSXCAD_VIEWER_BACKEND')
    if env:
        if env not in ('egl', 'osmesa', 'native'):
            raise ValueError('CSXCAD_VIEWER_BACKEND must be "egl", "osmesa" or "native", not "{}"'.format(env))
        _BACKEND = env
        return _BACKEND
    if not sys.platform.startswith('linux'):
        _BACKEND = 'native'
        return _BACKEND
    import vtkmodules.vtkRenderingOpenGL2 as gl
    candidates = []
    if hasattr(gl, 'vtkEGLRenderWindow'):
        candidates.append('egl')
    if hasattr(gl, 'vtkOSOpenGLRenderWindow'):
        import ctypes.util
        if ctypes.util.find_library('OSMesa'):
            candidates.append('osmesa')
    if os.environ.get('DISPLAY') or os.environ.get('WAYLAND_DISPLAY'):
        candidates.append('native')
    for c in candidates:
        if _Probe(c):
            _BACKEND = c
            return _BACKEND
    raise RuntimeError('No usable offscreen OpenGL backend found. Use an EGL capable driver '
                       '(e.g. Mesa), install OSMesa, or run with xvfb-run. Tried: {}'.format(
                           ', '.join(candidates) or 'none'))


def NewOffscreenWindow(backend=None):
    """ A new offscreen ``vtkRenderWindow`` of the chosen backend. """
    if backend is None:
        backend = OffscreenBackend()
    if backend == 'egl':
        from vtkmodules.vtkRenderingOpenGL2 import vtkEGLRenderWindow
        rw = vtkEGLRenderWindow()
    elif backend == 'osmesa':
        from vtkmodules.vtkRenderingOpenGL2 import vtkOSOpenGLRenderWindow
        rw = vtkOSOpenGLRenderWindow()
    else:
        rw = vtkRenderWindow()
    rw.SetOffScreenRendering(1)
    return rw


###############################################################################
# helpers

def DisplayAvailable():
    """ Whether windows can be opened (on Linux: an X display that accepts us). """
    if not sys.platform.startswith('linux'):
        return True
    if not os.environ.get('DISPLAY'):
        return False
    try:
        import ctypes
        import ctypes.util
        x11 = ctypes.CDLL(ctypes.util.find_library('X11') or 'libX11.so.6')
        x11.XOpenDisplay.restype = ctypes.c_void_p
        x11.XOpenDisplay.argtypes = [ctypes.c_char_p]
        x11.XCloseDisplay.argtypes = [ctypes.c_void_p]
        d = x11.XOpenDisplay(None)
        if not d:
            return False
        x11.XCloseDisplay(d)
        return True
    except OSError:
        return True   # cannot check, let VTK try


def ImageToPNG(img, filename=None):
    """ Write an (H, W, 3|4) uint8 array as PNG; returns the bytes without a file name. """
    img = np.ascontiguousarray(img[::-1])
    h, w, c = img.shape
    data = vtkImageData()
    data.SetDimensions(w, h, 1)
    arr = numpy_to_vtk(img.reshape(-1, c), deep=1)
    data.GetPointData().SetScalars(arr)
    writer = vtkPNGWriter()
    writer.SetInputData(data)
    if filename is None:
        writer.WriteToMemoryOn()
        writer.Write()
        return bytes(vtk_to_numpy(writer.GetResult()))
    writer.SetFileName(str(filename))
    writer.Write()
    return None


def InJupyter():
    """ True when running inside a Jupyter kernel. """
    if 'IPython' not in sys.modules:
        return False
    try:
        from IPython import get_ipython
        ip = get_ipython()
        return ip is not None and hasattr(ip, 'kernel')
    except Exception:
        return False


def _ShowInline(png):
    from IPython.display import Image, display
    display(Image(data=png))


_OUTPUT = {'dir': None, 'counts': {}}


def OutputDir():
    """ Absolute directory of the ``CSXCAD_VIEWER_OUTPUT`` output mode, or None.

    Resolved once and cached, so a later ``chdir`` does not move it.
    """
    env = os.environ.get('CSXCAD_VIEWER_OUTPUT')
    if not env:
        return None
    if _OUTPUT['dir'] is None or _OUTPUT['env'] != env:
        _OUTPUT['dir'] = os.path.abspath(env)
        _OUTPUT['env'] = env
        os.makedirs(_OUTPUT['dir'], exist_ok=True)
    return _OUTPUT['dir']


def _ScriptName():
    name = os.path.splitext(os.path.basename(sys.argv[0] if sys.argv and sys.argv[0] else ''))[0]
    if not name or name in ('-c', '-'):
        name = 'csxcad'
    return name


def _Round(v, digits=6):
    if isinstance(v, dict):
        return {k: _Round(x, digits) for k, x in v.items()}
    if isinstance(v, (list, tuple)):
        return type(v)(_Round(x, digits) for x in v)
    if isinstance(v, float):
        if v == 0 or not math.isfinite(v):
            return v
        return round(v, max(0, digits - 1 - int(math.floor(math.log10(abs(v))))))
    return v


###############################################################################
class ViewerHost:
    """ ViewerHost()

    Base of the window a viewer is shown in: the GUI's main window, the
    plain VTK window, or an application embedding the viewer (see
    :meth:`Viewer.SetHost`). The viewer calls these methods; the defaults do
    nothing, so a host overrides what it supports.
    """

    #: the host handles keys itself and shows the value under the mouse
    #: (the viewer's own key bindings and hover text are then off)
    handles_input = False

    def RenderSize(self):
        """ Size of the render area in pixels, or None. """
        return None

    def ApplySize(self, size):
        """ Resize so that the render area gets `size` pixels; False lets
        the viewer resize its render window. """
        return False

    def ScreenSize(self):
        """ Size in pixels of the screen the view is on, or None. """
        return None

    def WindowPosition(self):
        """ Top-left corner of the window in fractions of the available
        screen area, or None. """
        return None

    def ApplyPosition(self, position):
        """ Move the window, see :meth:`WindowPosition`. """

    def Schedule(self, ms, fn):
        """ Call `fn` once after `ms` milliseconds; False lets the viewer use
        VTK interactor timers. """
        return False

    def ShowStatus(self, message):
        """ Show a short message ('' clears it). """

    def OnClick(self, x, y):
        """ A left click without drag at display position (x, y), e.g. to pick. """

    def ActionAdded(self, action, toolbar):
        """ An action was added with :meth:`Viewer.AddAction`. """

    def TreeChanged(self):
        """ The scene tree or a visibility changed. """

    def AnimationChanged(self):
        """ The animation clock changed (steps, tick, playing). """

    def Close(self):
        """ Close the window; False lets the viewer stop its interactor. """
        return False


class _ScreenHost(ViewerHost):
    """ The plain VTK window: only knows its screen. """

    def __init__(self, screen):
        self._screen = screen

    def ScreenSize(self):
        return self._screen


###############################################################################
class Viewer:
    """ Viewer(CSX=None, offscreen=False, name=None, render_window=None, interactor=None, gui=None, block=True, **view)

    Viewer for a CSXCAD structure.

    Everything the viewer can show is reachable from this API, and the
    whole view is a plain dict of keyword arguments (the *view state*, see
    :meth:`GetState`), so a script can bake in a reproducible view.

    Parameters
    ----------
    CSX : ContinuousStructure or str, optional
        The structure, or an XML file name.
    offscreen : bool
        Render without any window (for screenshots on headless machines).
        Screenshots work without it too: a viewer that has no window renders
        off screen when needed.
    name : str, optional
        Base name of the files written in output mode (see :meth:`Show`).
    render_window, interactor : optional
        Host-supplied VTK render window and interactor to attach to
        (embedding).
    gui, block : optional
        Defaults for :meth:`Show`.
    **view
        View state keys, e.g. ``size=(1200, 800)``, ``render_mode='edges'``
        or ``camera=dict(...)``. Unknown keys raise ``ValueError``.

    Examples
    --------
    >>> v = Viewer(CSX, offscreen=True, render_mode='solid+edges')
    >>> v.SetView('iso')
    >>> v.Screenshot('iso.png', size=(1600, 1000))
    """

    def __init__(self, CSX=None, offscreen=False, name=None, render_window=None, interactor=None,
                 gui=None, block=True, **view):
        ViewState.CheckKeys(view)
        self._name = name
        self._filename = None       # the XML file the structure was read from
        self._gui_default = gui
        self._block_default = block
        self._renderer = vtkRenderer()
        self._renderer.SetUseDepthPeeling(True)
        self._renderer.SetOcclusionRatio(0.0)
        self._axes_renderer = vtkRenderer()
        self._axes_renderer.SetLayer(1)
        self._axes_renderer.SetViewport(0, 0, 0.18, 0.18)
        self._axes_renderer.InteractiveOff()
        self._axes_actor = vtkAxesActor()
        self._axes_actor.SetShaftTypeToCylinder()
        self._axes_actor.SetTotalLength(1, 1, 1)
        self._axes_renderer.AddActor(self._axes_actor)
        self._renderer.AddObserver('StartEvent', self._SyncAxes)
        self._renderer.AddObserver('StartEvent', self._PlaceLegends)
        # the depth range must cover what is shown now: items added or grown
        # since the last camera change would be cut by the near plane
        self._renderer.AddObserver('StartEvent', lambda o, e: o.ResetCameraClippingRange())
        self._scene = Scene(self._renderer)
        self._clock = AnimationClock(self)
        self._rw = None
        self._iren = None
        self._own_window = None     # window created by the viewer itself (offscreen)
        self._onscreen = False
        self._observers = []
        self._timers = {}
        self._host = ViewerHost()   # the window the view is shown in, see SetHost
        self._window_position = None
        self._fit_size = None       # size Fit() fits for, while capturing another size
        self._extra_actions = []    # [(Action, toolbar)] added with AddAction()
        self._click_start = None
        self._picking = True
        self._select_primitives = False
        self._batch = 0
        self._dirty = False
        self._fast = False
        self._last_full_time = 0.0
        self._wheel_token = 0
        self._camera_3d = None
        self._camera_set = False
        self._auto_fit = False      # the camera is only fitted, not set by the user
        self._looks = {}
        self._pending_visible = {}
        self._pending_items = {}    # {path: {name: value}} for items added later
        self._legends = []          # [(item, scalar bar)]
        self._legend_drag = None    # ((item, bar), press position, bar position)
        self._legend_cursor = False
        self._hover_text = None
        self._hover_time = 0.0
        self._interacting = False
        self._hover_actor = None
        self._band = None
        self._band_start = None
        self._edge_color_raw = None
        self._s = copy.deepcopy({k: ViewState.DEFAULTS[k] for k in
                                 ('size', 'background', 'axes', 'antialias', 'max_peels',
                                  'interactive_quality', 'mode_2d', 'grid', 'legends')})
        self._size_spec = tuple(self._s['size'])   # as set: pixels or (fraction, aspect)
        self._s['size'] = ResolveSize(self._size_spec)
        self._grid_info = {}
        self.SetBackground(self._s['background'], render=False)
        self.SetAntialias(True, render=False)
        self.SetMaxPeels(self._s['max_peels'], render=False)
        self.CSX = None
        if CSX is not None:
            self.SetCSX(CSX, render=False)
        if render_window is not None:
            self.Attach(render_window, interactor)
        elif offscreen:
            self._EnsureWindow()
        if view:
            self.SetState(view, render=False)
        if not self._camera_set:
            self.SetView('iso', render=False)
        self.Render()

    # ------------------------------------------------------------------
    # structure
    def SetCSX(self, CSX, render=True):
        """ SetCSX(CSX)

        Show another structure (a ``ContinuousStructure`` or an XML file name).
        """
        if isinstance(CSX, (str, os.PathLike)):
            from CSXCAD.CSXCAD import ContinuousStructure
            fn = os.fspath(CSX)
            if not os.path.isfile(fn):
                raise FileNotFoundError('file not found: {}'.format(fn))
            CSX = ContinuousStructure()
            err = CSX.ReadFromXML(fn)
            if err:
                warnings.warn('reading "{}": {}'.format(fn, err.strip()))
            if self._name is None:
                self._name = os.path.splitext(os.path.basename(fn))[0]
            self._filename = fn
        else:
            self._filename = None
        self.CSX = CSX
        self._scene.SetCSX(CSX)
        self._SnapshotLooks()
        self._ReportUnrendered()
        self._RebuildGrid()
        self._Changed(render)

    def GetCSX(self):
        """ The shown ``ContinuousStructure``. """
        return self.CSX

    def GetFileName(self):
        """ GetFileName()

        The XML file the structure was read from (see :meth:`SetCSX`) or
        last saved to (:meth:`SetFileName`), or None.
        """
        return self._filename

    def SetFileName(self, filename):
        """ SetFileName(filename)

        The XML file the shown structure belongs to, e.g. after saving it
        there; :meth:`GetFileName` returns it.
        """
        self._filename = None if filename is None else os.fspath(filename)

    def Update(self, prop=None, render=True):
        """ Update(prop=None)

        Re-read the structure after it was changed: rebuild properties whose
        primitives changed, add new ones and drop removed ones. With `prop`
        (property or name) only that one is rebuilt.
        """
        if self.CSX is None:
            return
        known = {id(e.prop) for e in self._scene.entries}
        self._scene.Update(prop)
        for e in self._scene.entries:
            if id(e.prop) not in known:
                self._looks[id(e.prop)] = self._Look(e.prop)
        self._RebuildGrid()
        self._Changed(render)

    def _ReportUnrendered(self):
        rep = self._scene.report
        if rep:
            kinds = sorted(set('{} ({})'.format(r['type'], r['reason']) for r in rep))
            warnings.warn('{} primitive(s) not rendered: {}'.format(len(rep), ', '.join(kinds)), stacklevel=3)

    @staticmethod
    def _Look(prop):
        return (prop.GetFillColor(), prop.GetEdgeColor(), bool(prop.GetVisibility()), prop.GetRenderMode())

    def _SnapshotLooks(self):
        self._looks = {id(e.prop): self._Look(e.prop) for e in self._scene.entries}

    # ------------------------------------------------------------------
    # windows
    def GetRenderer(self):
        """ The main ``vtkRenderer``. """
        return self._renderer

    def GetRenderWindow(self):
        """ The attached ``vtkRenderWindow``, or None. """
        return self._rw

    def GetInteractor(self):
        """ The attached interactor, or None (offscreen). """
        return self._iren

    def Attach(self, render_window, interactor=None, onscreen=True):
        """ Attach(render_window, interactor=None)

        Render into a host-supplied window (embedding), detaching from the
        current one.
        """
        self.Detach()
        self._rw = render_window
        self._onscreen = onscreen
        render_window.SetNumberOfLayers(2)
        render_window.SetAlphaBitPlanes(1)
        render_window.SetMultiSamples(0)   # MSAA breaks depth peeling
        render_window.AddRenderer(self._renderer)
        render_window.AddRenderer(self._axes_renderer)
        if not onscreen:
            render_window.SetSize(*self._s['size'])
        self._iren = interactor
        if interactor is not None:
            if interactor.GetRenderWindow() is not render_window:
                interactor.SetRenderWindow(render_window)
            self._InstallInteraction()

    def Detach(self):
        """ Detach from the current render window (if any). """
        self._RemoveInteraction()
        if self._rw is not None:
            self._rw.RemoveRenderer(self._renderer)
            self._rw.RemoveRenderer(self._axes_renderer)
        self._rw = None
        self._iren = None
        self._onscreen = False

    def _EnsureWindow(self):
        if self._rw is None:
            if self._own_window is None:
                self._own_window = NewOffscreenWindow()
            self.Attach(self._own_window, None, onscreen=False)
        return self._rw

    def SetHost(self, host):
        """ SetHost(host)

        The window the view is shown in, a :class:`ViewerHost` (None: none);
        the GUI sets itself while it is open.
        """
        self._host = host if host is not None else ViewerHost()

    def GetHost(self):
        return self._host

    def _Schedule(self, ms, fn):
        if self._host.Schedule(int(ms), fn):
            return True
        if self._iren is not None:
            tid = self._iren.CreateOneShotTimer(max(1, int(ms)))
            self._timers[tid] = fn
            return True
        return False

    @calldata_type(VTK_INT)
    def _OnTimer(self, obj, event, tid):
        fn = self._timers.pop(tid, None)
        if fn is not None:
            fn()

    def _AddObserver(self, obj, event, fn, priority=0.0):
        self._observers.append((obj, obj.AddObserver(event, fn, priority)))

    def _InstallInteraction(self):
        iren = self._iren
        self._AddObserver(iren, 'TimerEvent', self._OnTimer)
        self._AddObserver(iren, 'StartInteractionEvent', self._OnStartInteraction)
        self._AddObserver(iren, 'EndInteractionEvent', self._OnEndInteraction)
        self._AddObserver(iren, 'MouseWheelForwardEvent', self._OnWheel)
        self._AddObserver(iren, 'MouseWheelBackwardEvent', self._OnWheel)
        self._AddObserver(iren, 'MouseMoveEvent', self._OnHover, -1.0)
        # the trackball style grabs the mouse on a press, so the release never
        # reaches interactor observers in 3D; it ends its interaction instead
        self._AddObserver(iren, 'LeftButtonPressEvent', self._OnClickStart, 10.0)
        self._AddObserver(iren, 'LeftButtonReleaseEvent', self._OnClickEnd, 10.0)
        self._AddObserver(iren, 'EndInteractionEvent', self._OnClickEnd, 10.0)
        self._SetInteractorStyle()

    def _RemoveInteraction(self):
        for obj, tag in self._observers:
            obj.RemoveObserver(tag)
        self._observers = []
        self._timers = {}

    def _SetInteractorStyle(self):
        if self._iren is None:
            return
        if self._s['mode_2d']:
            # the style draws its band by copying window pixels, which does
            # not work reliably with this window setup (alpha planes, depth
            # peeling); it is drawn as a 2D actor instead (_OnBand*), VTK
            # keeps pan and zoom
            style = vtkInteractorStyleRubberBand2D()
        else:
            style = vtkInteractorStyleTrackballCamera()
        # the left button drags legends, else does what the style does
        style.AddObserver('LeftButtonPressEvent', self._OnLeftPress)
        style.AddObserver('MouseMoveEvent', self._OnStyleMove)
        style.AddObserver('LeftButtonReleaseEvent', self._OnLeftRelease)
        # observers on the style replace VTK's default key handling
        # (e.g. 'w'/'s' would turn everything into wireframe/surface)
        style.AddObserver('CharEvent', self._OnChar)
        style.AddObserver('KeyPressEvent', lambda o, e: None)
        style.SetDefaultRenderer(self._renderer)
        self._iren.SetInteractorStyle(style)

    def _OnChar(self, obj, event):
        if self._host.handles_input:
            return   # e.g. the GUI, through its actions
        from CSXCAD.Viewer import Actions
        key = self._iren.GetKeySym()
        if key is None:
            return
        Actions.HandleKey(self, key, self._iren.GetControlKey(), self._iren.GetShiftKey())

    def _OnLeftPress(self, style, event):
        x, y = self._iren.GetEventPosition()
        hit = self._LegendAt(x, y)
        if hit is not None:
            self._click_start = None   # no pick
            if self._iren.GetRepeatCount():
                # double click: back into the automatic layout
                self._s['legends'].pop(self._LegendKey(*hit), None)
                self.Render()
                return
            self._legend_drag = (hit, (x, y), tuple(hit[1].GetPosition()))
        elif self._s['mode_2d']:
            self._OnBandStart(style, event)
        else:
            style.OnLeftButtonDown()

    def _OnStyleMove(self, style, event):
        drag = self._legend_drag
        if drag is None:
            if self._s['mode_2d']:
                self._OnBandMove(style, event)
            else:
                style.OnMouseMove()
            if not self._interacting and self._rw is not None:
                over = self._LegendAt(*self._iren.GetEventPosition()) is not None
                if over != self._legend_cursor:
                    self._legend_cursor = over
                    self._rw.SetCurrentCursor(VTK_CURSOR_SIZEALL if over else VTK_CURSOR_DEFAULT)
            return
        (item, bar), (x0, y0), (px, py) = drag
        x, y = self._iren.GetEventPosition()
        w, h = (max(n, 1) for n in self._renderer.GetSize())
        nx = float(np.clip(px + (x - x0) / w, 0.0, 1.0 - bar.GetWidth()))
        ny = float(np.clip(py + (y - y0) / h, 0.0, 1.0 - bar.GetHeight()))
        self._s['legends'][self._LegendKey(item, bar)] = (nx, ny)
        self.Render()

    def _OnLeftRelease(self, style, event):
        if self._legend_drag is not None:
            self._legend_drag = None
        elif self._s['mode_2d']:
            self._OnBandEnd(style, event)
        else:
            style.OnLeftButtonUp()

    def _OnBandStart(self, style, event):
        self._band_start = self._iren.GetEventPosition()

    def _OnBandMove(self, style, event):
        start = self._band_start
        if start is None:
            style.OnMouseMove()   # pan and the like
            return
        x, y = self._iren.GetEventPosition()
        if self._band is None:
            from vtkmodules.vtkRenderingCore import vtkActor2D, vtkPolyDataMapper2D
            from CSXCAD.Viewer.Geometry import PolyDataFromArrays
            pd = PolyDataFromArrays(np.zeros((4, 3)), lines=[[0, 1, 2, 3, 0]])
            mapper = vtkPolyDataMapper2D()
            mapper.SetInputData(pd)
            self._band = vtkActor2D()
            self._band.SetMapper(mapper)
            self._band.GetProperty().SetColor(self._scene.TextColor())
            self._band.GetProperty().SetLineWidth(1.5)
            self._renderer.AddViewProp(self._band)
            self._SetFast(self._last_full_time > FAST_BUDGET)
        pts = self._band.GetMapper().GetInput().GetPoints()
        for i, (px, py) in enumerate(((start[0], start[1]), (x, start[1]), (x, y), (start[0], y))):
            pts.SetPoint(i, px, py, 0)
        pts.Modified()
        self.Render()

    def _OnBandEnd(self, style, event):
        start = self._band_start
        self._band_start = None
        if self._band is not None:
            self._renderer.RemoveViewProp(self._band)
            self._band = None
            self._SetFast(False)
        if start is None:
            return
        end = self._iren.GetEventPosition()
        if abs(start[0] - end[0]) < 3 or abs(start[1] - end[1]) < 3:
            self.Render()
            return
        self.ZoomToRect(start, end)

    def _OnClickStart(self, obj, event):
        self._click_start = obj.GetEventPosition()

    def _OnClickEnd(self, obj, event):
        start, self._click_start = self._click_start, None
        if start is None or not self._picking:
            return
        x, y = obj.GetEventPosition()
        if abs(x - start[0]) + abs(y - start[1]) <= 3:
            self._host.OnClick(x, y)

    # -- interactive quality
    def _OnStartInteraction(self, obj=None, event=None):
        self._interacting = True
        self._auto_fit = False
        q = self._s['interactive_quality']
        if q == 'full' or (q == 'auto' and self._last_full_time <= FAST_BUDGET):
            return
        self._SetFast(True)

    def _OnEndInteraction(self, obj=None, event=None):
        self._interacting = False
        if self._fast:
            self._SetFast(False)
            self.Render()

    def _OnWheel(self, obj, event):
        q = self._s['interactive_quality']
        if q == 'full' or (q == 'auto' and self._last_full_time <= FAST_BUDGET):
            return
        self._SetFast(True)
        self._wheel_token += 1
        token = self._wheel_token

        def settle():
            if token == self._wheel_token:
                self._OnEndInteraction()
        self._Schedule(WHEEL_SETTLE_MS, settle)

    def _SetFast(self, fast):
        self._fast = fast
        r = self._renderer
        if fast:
            r.SetMaximumNumberOfPeels(FAST_PEELS)
            r.SetUseFXAA(False)
        else:
            r.SetMaximumNumberOfPeels(self._s['max_peels'])
            r.SetUseFXAA(self._s['antialias'])

    # ------------------------------------------------------------------
    # rendering
    def Render(self):
        """ Render the attached window now (after changing something). """
        if self._batch:
            self._dirty = True
            return
        self._dirty = False
        if self._rw is None or not self._onscreen:
            return
        t0 = time.perf_counter()
        self._rw.Render()
        if not self._fast:
            self._last_full_time = time.perf_counter() - t0

    def _Changed(self, render):
        if render:
            self.Render()
        else:
            self._dirty = True

    @contextlib.contextmanager
    def Batch(self, render=True):
        """ Batch(render=True)

        Context manager that suspends rendering and renders once at the end
        (with `render` False not even then: a later :meth:`Render` does).

        Examples
        --------
        >>> with viewer.Batch():
        ...     viewer.SetRenderMode('edges')
        ...     viewer.SetView('+z')
        """
        self._batch += 1
        try:
            yield self
        finally:
            self._batch -= 1
            if self._batch == 0 and self._dirty and render:
                self.Render()

    def _SyncAxes(self, obj=None, event=None):
        cam = self._renderer.GetActiveCamera()
        acam = self._axes_renderer.GetActiveCamera()
        d = np.array(cam.GetPosition()) - np.array(cam.GetFocalPoint())
        n = np.linalg.norm(d)
        if n == 0:
            return
        acam.SetFocalPoint(0, 0, 0)
        acam.SetPosition(*(d / n * 5))
        acam.SetViewUp(cam.GetViewUp())
        acam.ParallelProjectionOff()
        self._axes_renderer.ResetCamera()

    # ------------------------------------------------------------------
    # view state
    def GetState(self):
        """ GetState()

        The complete view state as a JSON-serialisable dict.
        ``SetState(GetState())`` restores exactly this view.
        """
        st = {'version': ViewState.VERSION}
        for key, (setter, getter) in ViewState.ACCESSORS.items():
            if key == 'visible':
                st[key] = dict(self._pending_visible)
                st[key].update({p: it.GetVisible() for p, it in self._scene.items.items()})
            elif key == 'items':
                st[key] = {p: dict(v) for p, v in self._pending_items.items()}
                for p, it in self._scene.items.items():
                    st[key][p] = {q['name']: q['value'] for q in self.GetItemParameters(it)
                                  if q.get('state', True)}
            elif key == 'view':
                st[key] = None
            else:
                st[key] = getattr(self, getter)()
        return ViewState.ToJSON(st)

    def SetState(self, state=None, render=True, **kw):
        """ SetState(state=None, **kw)

        Apply a (partial) view state dict; omitted keys keep their current
        values. Unknown keys raise ``ValueError``.
        """
        st = dict(state or {})
        st.update(kw)
        ViewState.CheckKeys(st)
        with self.Batch(render):
            for key in ViewState.DEFAULTS:
                if key not in st or key == 'version':
                    continue
                val = st[key]
                setter = ViewState.ACCESSORS[key][0]
                if key == 'visible':
                    for path, v in (val or {}).items():
                        if path in self._scene.tree.nodes:
                            self.SetVisible(path, v, render=False)
                        else:
                            # an item added later picks it up, see AddItem
                            self._pending_visible[path] = bool(v)
                elif key == 'items':
                    for path, params in (val or {}).items():
                        if path in self._scene.items:
                            self._ApplyItemParameters(self._scene.items[path], params)
                        else:
                            # an item added later picks them up, see AddItem
                            self._pending_items.setdefault(path, {}).update(params)
                elif key == 'grid':
                    self.SetGrid(val, render=False)
                else:
                    getattr(self, setter)(val, render=False)
            self._dirty = True

    def SaveState(self, filename):
        """ Save the view state as JSON. """
        with open(filename, 'w') as f:
            f.write(ViewState.Dumps(self.GetState()))

    def LoadState(self, filename, render=True):
        """ Load a view state from a JSON file. """
        with open(filename) as f:
            self.SetState(json.load(f), render=render)

    def GetStateAsPython(self):
        """ GetStateAsPython()

        A snippet to paste into a script: the ``view = dict(...)`` literal of
        everything that differs from the defaults (size and camera always),
        plus the property setter calls for every colour, opacity, visibility
        or render mode changed since loading.
        """
        st = self.GetState()
        lines = ['view = dict(']
        for key, default in ViewState.DEFAULTS.items():
            if key in ('version', 'view'):
                continue
            val = st[key]
            if key not in ('size', 'camera') and ViewState.ToJSON(default) == val:
                continue
            if key == 'grid':
                val = self._GridDiff(val)
            if key == 'items':
                val = self._ItemsDiff(val)
                if not val:
                    continue
            lines.append('    {}={},'.format(key, repr(_Round(_Tuples(val)))))
        lines.append(')')
        for e in self._scene.entries:
            p = e.prop
            old = self._looks.get(id(p))
            if old is None or old == self._Look(p):
                continue
            ref = "CSX.GetPropertiesByName({!r})[0]".format(e.name)
            fill, edge, vis, mode = self._Look(p)
            if fill != old[0] and edge != old[1] and fill == edge:
                lines.append('{}.SetColor({!r}, {})'.format(ref, _Hex(fill), fill[3]))
            else:
                if fill != old[0]:
                    lines.append('{}.SetFillColor({!r}, {})'.format(ref, _Hex(fill), fill[3]))
                if edge != old[1]:
                    lines.append('{}.SetEdgeColor({!r}, {})'.format(ref, _Hex(edge), edge[3]))
            if vis != old[2]:
                lines.append('{}.SetVisibility({})'.format(ref, vis))
            if mode != old[3]:
                lines.append('{}.SetRenderMode({!r})'.format(ref, mode))
        return '\n'.join(lines)

    def _ItemsDiff(self, items):
        out = {}
        for path, params in items.items():
            item = self._scene.items.get(path)
            defaults = {} if item is None else {q['name']: q['default'] for q in self.GetItemParameters(item)
                                                 if 'default' in q}
            diff = {k: v for k, v in params.items()
                    if k not in defaults or ViewState.ToJSON(defaults[k]) != ViewState.ToJSON(v)}
            if diff:
                out[path] = diff
        return out

    def _GridDiff(self, grid):
        d = ViewState.GRID_DEFAULT
        out = {k: v for k, v in grid.items() if k != 'planes' and v != d[k]}
        planes = {k: v for k, v in grid['planes'].items() if v != d['planes'][k]}
        if planes:
            out['planes'] = planes
        return out

    # -- simple settings
    def SetSize(self, size, render=True):
        """ SetSize(size)

        Size of the render area: ``(fraction, aspect)``, a diagonal of that
        fraction of the screen diagonal with width / height `aspect` (the
        same share of any screen), or ``(width, height)`` in pixels (e.g. an
        exact image size). Without a screen (offscreen) a relative size
        refers to a 1920 x 1080 one. A GUI resizes its VTK widget, not the
        whole window.
        """
        if IsRelativeSize(size):
            self._size_spec = (float(size[0]), float(size[1]))
        else:
            self._size_spec = (int(size[0]), int(size[1]))
        size = ResolveSize(self._size_spec, self._Screen())
        self._s['size'] = size
        if not self._host.ApplySize(size) and self._rw is not None and (not self._onscreen or self._iren is not None):
            self._rw.SetSize(*size)
        self._Changed(render)

    def _Screen(self):
        """ The screen size in pixels the view is on, or None. """
        s = self._host.ScreenSize()
        return tuple(s) if s and s[0] > 0 and s[1] > 0 else None

    def GetViewSize(self):
        """ GetViewSize()

        The size as kept in the view state: relative to the screen while the
        view is on one, else as set (see :meth:`SetSize`). :meth:`GetSize`
        gives pixels.
        """
        screen = self._Screen()
        if screen is not None:
            return RelativeSize(self.GetSize(), screen)
        return tuple(self._size_spec)

    def SetWindowPosition(self, position, render=True):
        """ SetWindowPosition(position)

        Position ``(x, y)`` of the window's top-left corner in fractions of
        the available screen area, for the GUI and the plain VTK window (a
        Wayland compositor may ignore it); None leaves it to the window
        system.
        """
        self._window_position = None if position is None else (float(position[0]), float(position[1]))
        if self._window_position is not None:
            self._host.ApplyPosition(self._window_position)

    def GetWindowPosition(self):
        pos = self._host.WindowPosition()
        if pos is not None:
            self._window_position = tuple(pos)
        return self._window_position

    def GetSize(self):
        s = self._host.RenderSize()
        if s is not None:
            s = tuple(int(x) for x in s)
            if s[0] > 0 and s[1] > 0:
                self._s['size'] = s
            return tuple(self._s['size'])
        if self._rw is not None and self._onscreen:
            s = tuple(self._rw.GetSize())
            if s[0] > 0 and s[1] > 0:
                self._s['size'] = s
        return tuple(self._s['size'])

    def SetBackground(self, color, render=True):
        """ SetBackground(color)

        Background colour; a 2-tuple of colours gives a vertical gradient
        (bottom, top).
        """
        self._s['background'] = color
        r = self._renderer
        if isinstance(color, (list, tuple)) and len(color) == 2 and not isinstance(color[0], (int, float)):
            c0, _ = ParseColor(color[0])
            c1, _ = ParseColor(color[1])
            r.GradientBackgroundOn()
            r.SetBackground(*c0)
            r.SetBackground2(*c1)
            bg = tuple((np.array(c0) + np.array(c1)) / 2)
        else:
            c, _ = ParseColor(color)
            r.GradientBackgroundOff()
            r.SetBackground(*c)
            bg = c
        self._scene.background = bg
        tc = self._scene.TextColor()
        for cap in (self._axes_actor.GetXAxisCaptionActor2D(), self._axes_actor.GetYAxisCaptionActor2D(),
                    self._axes_actor.GetZAxisCaptionActor2D()):
            tp = cap.GetCaptionTextProperty()
            tp.SetColor(tc)
            tp.ShadowOff()
            tp.ItalicOff()
        self._scene.UpdateLabels()
        if self._scene.grid_layer.scalar_bar is not None:
            self._RebuildGrid()
        self._Changed(render)

    def GetBackground(self):
        return self._s['background']

    def SetArcResolution(self, val, render=True):
        """ Number of segments of a full circle (default 32). """
        val = int(val)
        if val != self._scene.arc_resolution:
            self._scene.arc_resolution = val
            if self.CSX is not None:
                self._scene.Rebuild()
        self._Changed(render)

    def GetArcResolution(self):
        return self._scene.arc_resolution

    def SetDiscMaterial(self, val, render=True):
        """ Render the surface model of discrete materials (default True). """
        val = bool(val)
        if val != self._scene.disc_material:
            self._scene.disc_material = val
            if self.CSX is not None:
                self._scene.Rebuild()
        self._Changed(render)

    def GetDiscMaterial(self):
        return self._scene.disc_material

    def SetRenderMode(self, mode, render=True):
        """ SetRenderMode(mode)

        Default render mode, ``'solid'``, ``'edges'`` or ``'solid+edges'``,
        for properties without one of their own (``prop.SetRenderMode()``).
        """
        if mode not in ViewState.RENDER_MODES:
            raise ValueError('invalid render mode "{}", use one of {}'.format(mode, ViewState.RENDER_MODES))
        self._scene.render_mode = mode
        self._scene.ApplyAll()
        self._Changed(render)

    def GetRenderMode(self):
        return self._scene.render_mode

    def SetTransparency(self, val, render=True):
        """ SetTransparency(val)

        Global see-through switch: cap the opacity of every property at
        `val` (0..1). None turns it off. The structure is not changed.
        """
        self._scene.transparency = None if val is None else float(np.clip(val, 0, 1))
        self._scene.ApplyAll()
        self._Changed(render)

    def GetTransparency(self):
        return self._scene.transparency

    def SetEdgeColor(self, color, render=True):
        """ SetEdgeColor(color)

        Edge colour for all properties. None (default): each property's
        EdgeColor in ``'solid+edges'`` mode, and its fill colour in
        ``'edges'`` mode, where no faces show the colour.
        """
        self._edge_color_raw = color
        self._scene.edge_color = None if color is None else ParseColor(color)
        self._scene.ApplyAll()
        self._Changed(render)

    def GetEdgeColor(self):
        return self._edge_color_raw

    def SetEdgeAngle(self, angle, render=True):
        """ Feature angle in degrees above which an edge is drawn (default 30). """
        self._scene.edge_angle = float(angle)
        for e in self._scene.entries:
            e.SetEdgeAngle(angle)
        self._Changed(render)

    def GetEdgeAngle(self):
        return self._scene.edge_angle

    def SetAntialias(self, val, render=True):
        """ FXAA anti-aliasing (MSAA is off, it breaks depth peeling). """
        self._s['antialias'] = bool(val)
        if not self._fast:
            self._renderer.SetUseFXAA(bool(val))
        self._Changed(render)

    def GetAntialias(self):
        return self._s['antialias']

    def SetMaxPeels(self, val, render=True):
        """ SetMaxPeels(val)

        Safety cap of the depth peeling passes for transparency (default 100).
        Peeling stops by itself once a pass changes no pixel.
        """
        val = int(val)
        if val < 1:
            raise ValueError('max_peels must be >= 1')
        self._s['max_peels'] = val
        if not self._fast:
            self._renderer.SetMaximumNumberOfPeels(val)
        self._Changed(render)

    def GetMaxPeels(self):
        return self._s['max_peels']

    def SetInteractiveQuality(self, val, render=True):
        """ SetInteractiveQuality(val)

        Quality while the view moves: ``'auto'`` (reduce only if a full
        render is slow), ``'full'`` or ``'fast'``.
        """
        if val not in ViewState.INTERACTIVE_QUALITY:
            raise ValueError('invalid interactive_quality "{}"'.format(val))
        self._s['interactive_quality'] = val

    def GetInteractiveQuality(self):
        return self._s['interactive_quality']

    def SetAxes(self, val, render=True):
        """ Show the axes orientation marker. """
        self._s['axes'] = bool(val)
        self._axes_renderer.SetDraw(bool(val))
        self._Changed(render)

    def GetAxes(self):
        return self._s['axes']

    def SetLabels(self, val, render=True):
        """ Show property names in front of the scene, each with a leader to the
        point of its property nearest the camera (an exposed corner for boxes). """
        self._scene.labels = bool(val)
        self._scene.UpdateLabels()
        self._Changed(render)

    def GetLabels(self):
        return self._scene.labels

    # ------------------------------------------------------------------
    # property looks (write-through to the structure)
    def SetPropertyColor(self, name, fill=None, edge=None, opacity=None, render=True):
        """ SetPropertyColor(name, fill=None, edge=None, opacity=None)

        Change colours and opacity of a property, written to the property
        itself (and thus saved with the XML).

        Parameters
        ----------
        name : str or CSProperties
        fill, edge : colour, optional
        opacity : float, optional
            Fill opacity 0..1 (the fill alpha).
        """
        e = self._scene.Entry(name)
        p = e.prop
        if fill is not None:
            rgb, a = ParseColor(fill)
            alpha = p.GetFillColor()[3] if a is None else int(round(a * 255))
            p.SetFillColor(tuple(int(round(c * 255)) for c in rgb), alpha)
        if edge is not None:
            rgb, a = ParseColor(edge)
            alpha = p.GetEdgeColor()[3] if a is None else int(round(a * 255))
            p.SetEdgeColor(tuple(int(round(c * 255)) for c in rgb), alpha)
        if opacity is not None:
            f = p.GetFillColor()
            p.SetFillColor(f[:3], int(round(np.clip(opacity, 0, 1) * 255)))
        e.Apply()
        self._Changed(render)

    def SetPropertyVisible(self, name, val, render=True):
        """ Show or hide a property by name, see :meth:`SetVisible`. """
        self.SetVisible(self._scene.Entry(name).path, val, render)

    def SetPropertyRenderMode(self, name, mode, render=True):
        """ Render mode of one property (``prop.SetRenderMode``); None = default. """
        e = self._scene.Entry(name)
        e.prop.SetRenderMode(mode)
        e.Apply()
        self._Changed(render)

    def GetPropertyNames(self):
        """ Names of all properties, in structure order. """
        return [e.name for e in self._scene.entries]

    # ------------------------------------------------------------------
    # camera
    def GetCamera(self):
        """ GetCamera()

        Camera as dict: position, focal_point, view_up, view_angle,
        parallel_scale and parallel_projection.
        """
        c = self._renderer.GetActiveCamera()
        return dict(position=tuple(c.GetPosition()), focal_point=tuple(c.GetFocalPoint()),
                    view_up=tuple(c.GetViewUp()), view_angle=c.GetViewAngle(),
                    parallel_scale=c.GetParallelScale(),
                    parallel_projection=bool(c.GetParallelProjection()))

    def SetCamera(self, camera, render=True):
        """ SetCamera(camera)

        Set the camera from a dict (see :meth:`GetCamera`, partial dicts
        allowed); None fits the view.
        """
        self._auto_fit = False
        if camera is None:
            self.Fit(render=render)
            return
        c = self._renderer.GetActiveCamera()
        for key in camera:
            if key not in ('position', 'focal_point', 'view_up', 'view_angle', 'parallel_scale', 'parallel_projection'):
                raise ValueError('unknown camera key "{}"'.format(key))
        if 'focal_point' in camera:
            c.SetFocalPoint(*camera['focal_point'])
        if 'position' in camera:
            c.SetPosition(*camera['position'])
        if 'view_up' in camera:
            c.SetViewUp(*camera['view_up'])
        if 'view_angle' in camera:
            c.SetViewAngle(camera['view_angle'])
        if 'parallel_scale' in camera:
            c.SetParallelScale(camera['parallel_scale'])
        if 'parallel_projection' in camera:
            c.SetParallelProjection(bool(camera['parallel_projection']))
        self._camera_set = True
        self._renderer.ResetCameraClippingRange()
        self._Changed(render)

    def SetView(self, view, render=True, fit=True):
        """ SetView(view)

        Look at the structure from a named direction.

        Parameters
        ----------
        view : str or None
            ``'+x'`` ... ``'-z'`` look along that axis (``'+z'`` looks from +z
            down); ``'xy'``, ``'yz'``, ``'xz'`` are ``'+z'``, ``'+x'``, ``'-y'``.
            Isometric views look from (1,1,1), so all axes point towards the
            viewer and grid planes at the lowest lines are behind the
            structure; they are named by the axis pointing up: ``'iso_z'``
            (alias ``'iso'``), ``'iso_x'``, ``'iso_y'``. None does nothing.
        """
        if view is None:
            return
        name = _VIEW_ALIASES.get(view, view)
        if name not in _VIEW_DIRS:
            raise ValueError('invalid view "{}", use one of {}'.format(view, ', '.join(ViewState.VIEWS)))
        d, up = _VIEW_DIRS[name]
        c = self._renderer.GetActiveCamera()
        d = np.array(d, float)
        c.SetFocalPoint(0, 0, 0)
        c.SetPosition(*(d / np.linalg.norm(d)))
        c.SetViewUp(*up)
        self._camera_set = True
        self._auto_fit = fit
        if fit:
            self.Fit(render=False)
        self._Changed(render)

    def _Bounds(self):
        b = self._scene.VisibleBounds()
        g = self._s['grid']
        gb = self._scene.grid_layer.GetBounds()
        # with the outline shown, the mesh domain is part of the picture
        if g['visible'] and g['outline']:
            return UnionBounds([b, gb])
        return b

    def Fit(self, margin=0.05, bounds=None, render=True):
        """ Fit(margin=0.05, bounds=None)

        Fit the camera on all visible content (or `bounds`) without changing
        the view direction: as close as the content's points allow, with a
        `margin` (fraction of the half view) around them.
        """
        pts = None
        if bounds is None:
            pts = self._FitPoints()
            if pts is None:
                bounds = self._Bounds()
        if pts is None:
            if bounds is None:
                self._renderer.ResetCamera()
                self._Changed(render)
                return
            b = np.asarray(bounds, float).reshape(3, 2)
            pts = np.array([[x, y, z] for x in b[0] for y in b[1] for z in b[2]])
        c = self._renderer.GetActiveCamera()
        center = (pts.min(axis=0) + pts.max(axis=0)) / 2
        d = np.array(c.GetPosition()) - np.array(c.GetFocalPoint())
        if np.linalg.norm(d) == 0:
            d = np.array([1.0, -1.0, 1.0])
        d /= np.linalg.norm(d)
        up = np.array(c.GetViewUp())
        right = np.cross(-d, up)
        if np.linalg.norm(right) < 1e-9:
            up = np.array([0, 1.0, 0]) if abs(d[1]) < 0.9 else np.array([0, 0, 1.0])
            right = np.cross(-d, up)
        right /= np.linalg.norm(right)
        up = np.cross(right, -d)
        rel = pts - center
        u = rel @ right
        v = rel @ up
        w = rel @ d
        w_size, h_size = self._fit_size or self.GetSize()
        aspect = w_size / max(1, h_size)
        # the legends on the right are not covered
        share = self._LegendShare(w_size)
        diag = float(np.linalg.norm(pts.max(axis=0) - pts.min(axis=0)))
        half_h = max((v.max() - v.min()) / 2, (u.max() - u.min()) / 2 / (aspect * (1 - share))) * (1 + margin)
        if half_h <= 0:
            half_h = max(diag / 2, 1e-6)
        if c.GetParallelProjection():
            su = (u.max() + u.min()) / 2 + half_h * aspect * share
            sv = (v.max() + v.min()) / 2
            dist = w.max() + diag + half_h
        else:
            su, sv, dist = self._FitPerspective(u, v, w, math.tan(math.radians(c.GetViewAngle()) / 2) / (1 + margin),
                                                aspect, share, max(diag, half_h))
        focal = center + su * right + sv * up
        c.SetFocalPoint(*focal)
        c.SetViewUp(*up)
        c.SetParallelScale(half_h)
        c.SetPosition(*(focal + d * dist))
        self._camera_set = True
        self._renderer.ResetCameraClippingRange()
        self._Changed(render)

    @staticmethod
    def _FitPerspective(u, v, w, t, aspect, share, size):
        """ The closest camera distance (and lateral shift su, sv) at which
        all points (u, v right/up, w towards the camera) are inside the view
        of half angle tangent `t` (vertical) and aspect `aspect`, without
        the `share` of the width on the right. """
        t_left, t_right = t * aspect, t * aspect * (1 - 2 * share)

        def shift(dist):
            z = dist - w
            lo_u, hi_u = (u - t_right * z).max(), (u + t_left * z).min()
            lo_v, hi_v = (v - t * z).max(), (v + t * z).min()
            if lo_u > hi_u or lo_v > hi_v:
                return None
            return (lo_u + hi_u) / 2, (lo_v + hi_v) / 2
        lo = w.max()
        hi = lo + size / t
        while shift(hi) is None:
            hi = lo + 2 * (hi - lo)
        for _ in range(50):
            mid = (lo + hi) / 2
            if shift(mid) is None:
                lo = mid
            else:
                hi = mid
        su, sv = shift(hi)
        return su, sv, hi

    def _FitPoints(self):
        """ Points of the visible content (structure, items counted for the
        bounds, grid outline) for a tight fit, or None. """
        sc = self._scene
        entries = [e for e in sc.entries if e.IsVisible()]
        items = [it for it in sc.items.values() if it.GetVisible() and it.IsIncludedInBounds()]
        if not entries and not items:
            entries = list(sc.entries)
        parts = []
        for e in entries:
            for pd in (e.raw, e.disc_pd):
                if pd is not None and pd.GetNumberOfPoints():
                    parts.append(vtk_to_numpy(pd.GetPoints().GetData()))
        for it in items:
            for a in it.GetProps():
                if not isinstance(a, vtkActor) or not a.GetVisibility() or a.GetMapper() is None:
                    continue
                a.GetMapper().Update()
                ds = a.GetMapper().GetInputDataObject(0, 0)
                if ds is None or not hasattr(ds, 'GetPoints') or ds.GetPoints() is None:
                    continue
                m = a.GetMatrix()
                m = np.array([[m.GetElement(i, j) for j in range(4)] for i in range(4)])
                parts.append(vtk_to_numpy(ds.GetPoints().GetData()) @ m[:3, :3].T + m[:3, 3])
        g = self._s['grid']
        gb = sc.grid_layer.GetBounds()
        if g['visible'] and g['outline'] and gb is not None:
            b = np.asarray(gb, float).reshape(3, 2)
            parts.append(np.array([[x, y, z] for x in b[0] for y in b[1] for z in b[2]]))
        parts = [np.asarray(p, float).reshape(-1, 3) for p in parts]
        if not parts:
            return None
        pts = np.concatenate(parts)
        pts = pts[np.isfinite(pts).all(axis=1)]
        if len(pts) > 2000000:
            pts = pts[::len(pts) // 2000000 + 1]
        return pts if len(pts) else None

    def Reset(self, render=True):
        """ Reset to the isometric view, fitted. """
        if self._s['mode_2d']:
            self.SetMode2D(False, render=False)
        self.GetRenderer().GetActiveCamera().ParallelProjectionOff()
        self.SetView('iso', render=render)

    def Focus(self, target, view=None, margin=0.1, render=True):
        """ Focus(target, view=None, margin=0.1)

        Fit the camera on a detail.

        Parameters
        ----------
        target : str, list, tuple
            A property name, a list of names, ``(name, primitive_id)`` or a
            bounding box ``(xmin, xmax, ymin, ymax, zmin, zmax)``.
        view : str, optional
            Named view direction to use (see :meth:`SetView`).
        """
        self._auto_fit = False
        if view is not None:
            self.SetView(view, render=False, fit=False)
        self.Fit(margin=margin, bounds=self._TargetBounds(target), render=render)

    def _TargetBounds(self, target):
        if isinstance(target, (list, tuple, np.ndarray)) and len(target) == 6 and all(
                isinstance(v, (int, float, np.number)) for v in target):
            return tuple(float(v) for v in target)
        if isinstance(target, tuple) and len(target) == 2 and isinstance(target[1], (int, np.integer)):
            e = self._scene.Entry(target[0])
            pd, cells = e.CellsOf(int(target[1]))
            if pd is None or len(cells) == 0:
                raise KeyError('no primitive {} in "{}"'.format(target[1], target[0]))
            from CSXCAD.Viewer.Scene import ExtractCells
            return ExtractCells(pd, cells).GetBounds()
        bounds = UnionBounds(e.GetBounds() for e in self._scene.Entries(target))
        if bounds is None:
            raise ValueError('nothing to focus on')
        return bounds

    def SetFocalPoint(self, point, render=True):
        """ SetFocalPoint(point)

        Make `point` the centre of rotation and zoom. The camera moves along,
        keeping its direction and distance, so the point ends up in the
        middle of the view.
        """
        self._auto_fit = False
        c = self._renderer.GetActiveCamera()
        shift = np.asarray(point, dtype=float) - c.GetFocalPoint()
        c.SetFocalPoint(*(c.GetFocalPoint() + shift))
        c.SetPosition(*(c.GetPosition() + shift))
        self._renderer.ResetCameraClippingRange()
        self._Changed(render)

    def FocusAtCursor(self):
        """ Make the point under the mouse (structure or item) the centre of
        rotation and zoom, like VTK's 'f' key: the camera turns towards it and
        moves a bit closer, animated. In 2D mode it slides within the plane
        instead (see :meth:`SetFocalPoint`). Returns False if nothing is under
        the mouse. """
        if self._iren is None:
            return False
        hit = self.Pick(*self._iren.GetEventPosition())
        if hit is None:
            return False
        if self._s['mode_2d']:
            self.SetFocalPoint(hit['world_pos'])
            return True
        self._OnStartInteraction()
        self._iren.FlyTo(self._renderer, *hit['world_pos'])
        self._OnEndInteraction()
        self._Changed(True)
        return True

    def Zoom(self, factor, render=True):
        """ Zoom in (factor > 1) or out. """
        self._auto_fit = False
        c = self._renderer.GetActiveCamera()
        if c.GetParallelProjection():
            c.SetParallelScale(c.GetParallelScale() / factor)
        else:
            c.Dolly(factor)
            self._renderer.ResetCameraClippingRange()
        self._Changed(render)

    def Dolly(self, factor, render=True):
        """ Move the camera towards the focal point (factor > 1) or away. """
        self._auto_fit = False
        self._renderer.GetActiveCamera().Dolly(factor)
        self._renderer.ResetCameraClippingRange()
        self._Changed(render)

    def Azimuth(self, deg, render=True):
        """ Rotate the camera about the view-up vector through the focal point. """
        self._renderer.GetActiveCamera().Azimuth(deg)
        self._renderer.ResetCameraClippingRange()
        self._Changed(render)

    def Elevation(self, deg, render=True):
        """ Rotate the camera about the horizontal screen axis. """
        c = self._renderer.GetActiveCamera()
        c.Elevation(deg)
        c.OrthogonalizeViewUp()
        self._renderer.ResetCameraClippingRange()
        self._Changed(render)

    def Roll(self, deg, render=True):
        """ Rotate the image in-plane. """
        self._renderer.GetActiveCamera().Roll(deg)
        self._Changed(render)

    def Flip(self, render=True):
        """ Look from the opposite side (180 degrees about the view up). """
        self.Azimuth(180, render=render)

    def Pan(self, dx, dy, render=True):
        """ Pan(dx, dy)

        Move the view by a fraction of its width (dx) and height (dy).
        """
        self._auto_fit = False
        c = self._renderer.GetActiveCamera()
        fp = np.array(c.GetFocalPoint())
        pos = np.array(c.GetPosition())
        d = fp - pos
        dist = np.linalg.norm(d)
        up = np.array(c.GetViewUp())
        right = np.cross(d / dist, up)
        if c.GetParallelProjection():
            h = 2 * c.GetParallelScale()
        else:
            h = 2 * dist * math.tan(math.radians(c.GetViewAngle()) / 2)
        w_size, h_size = self.GetSize()
        w = h * w_size / max(1, h_size)
        shift = -dx * w * right - dy * h * up
        c.SetFocalPoint(*(fp + shift))
        c.SetPosition(*(pos + shift))
        self._Changed(render)

    def ZoomToRect(self, p0, p1, render=True):
        """ Zoom onto a rectangle given in display coordinates. """
        self._auto_fit = False
        c = self._renderer.GetActiveCamera()
        coord = vtkCoordinate()
        coord.SetCoordinateSystemToDisplay()
        cx = (p0[0] + p1[0]) / 2
        cy = (p0[1] + p1[1]) / 2
        coord.SetValue(cx, cy, 0)
        fp = np.array(c.GetFocalPoint())
        # move the focal point within the view plane
        self._renderer.SetWorldPoint(*fp, 1)
        self._renderer.WorldToDisplay()
        z = self._renderer.GetDisplayPoint()[2]
        self._renderer.SetDisplayPoint(cx, cy, z)
        self._renderer.DisplayToWorld()
        w = np.array(self._renderer.GetWorldPoint())
        new_fp = w[:3] / w[3]
        shift = new_fp - fp
        c.SetFocalPoint(*new_fp)
        c.SetPosition(*(np.array(c.GetPosition()) + shift))
        w_size, h_size = self.GetSize()
        f = min(w_size / max(1, abs(p1[0] - p0[0])), h_size / max(1, abs(p1[1] - p0[1])))
        self.Zoom(f, render=render)

    def SetParallelProjection(self, val, render=True):
        """ Parallel (orthographic) instead of perspective projection. """
        c = self._renderer.GetActiveCamera()
        val = bool(val)
        if val and not c.GetParallelProjection():
            # keep the apparent size at the focal point
            dist = c.GetDistance()
            c.SetParallelScale(dist * math.tan(math.radians(c.GetViewAngle()) / 2))
        c.SetParallelProjection(val)
        self._Changed(render)

    def GetParallelProjection(self):
        return bool(self._renderer.GetActiveCamera().GetParallelProjection())

    def SetMode2D(self, val, render=True):
        """ SetMode2D(val)

        2D mode: snap to the nearest axis view with parallel projection;
        left-drag zooms into a rectangle, the middle button pans within the
        plane and the wheel zooms, there is no rotation. Leaving it restores
        the 3D camera.
        """
        val = bool(val)
        if val == self._s['mode_2d']:
            return
        c = self._renderer.GetActiveCamera()
        if val:
            self._camera_3d = self.GetCamera()
            d = np.array(c.GetPosition()) - np.array(c.GetFocalPoint())
            n = int(np.argmax(np.abs(d)))
            dn = np.zeros(3)
            dn[n] = np.sign(d[n]) or 1.0
            up = np.array(c.GetViewUp())
            up[n] = 0
            k = int(np.argmax(np.abs(up)))
            un = np.zeros(3)
            un[k] = np.sign(up[k]) or 1.0
            if k == n or np.linalg.norm(up) == 0:
                un = np.array(_VIEW_DIRS[('+' if dn[n] > 0 else '-') + 'xyz'[n]][1], float)
            dist = c.GetDistance()
            fp = np.array(c.GetFocalPoint())
            if not c.GetParallelProjection():
                c.SetParallelScale(dist * math.tan(math.radians(c.GetViewAngle()) / 2))
            c.SetPosition(*(fp + dn * dist))
            c.SetViewUp(*un)
            c.ParallelProjectionOn()
        else:
            if self._camera_3d is not None:
                self.SetCamera(self._camera_3d, render=False)
            else:
                c.ParallelProjectionOff()
        self._s['mode_2d'] = val
        self._SetInteractorStyle()
        self._renderer.ResetCameraClippingRange()
        self._Changed(render)

    def GetMode2D(self):
        return self._s['mode_2d']

    # ------------------------------------------------------------------
    # grid
    def SetGrid(self, grid=None, render=True, **kw):
        """ SetGrid(grid=None, **kw)

        Change the mesh-grid display, from a (partial) dict and/or keywords:
        ``visible``, ``color``, ``opacity`` (0..1), ``planes`` (per direction
        ``'x'``/``'y'``/``'z'``: ``visible``, ``index`` or ``position``),
        ``outline``, ``quality`` (None, ``'size'``, ``'ratio'``) and
        ``quality_threshold``.

        Returns
        -------
        dict
            ``{dir: (index, coordinate, number of lines, clamped)}`` of the
            planes, see :meth:`GetGridPlaneInfo`.

        Examples
        --------
        >>> viewer.SetGrid(planes={'z': dict(position=1.6)}, opacity=0.5)
        """
        upd = dict(grid or {})
        upd.update(kw)
        self._s['grid'] = ViewState.MergeGrid(self._s['grid'], upd)
        info = self._RebuildGrid()
        self._Changed(render)
        return info

    def GetGrid(self):
        g = copy.deepcopy(self._s['grid'])
        # report resolved indices, so that the state reproduces exactly
        for d, plane in g['planes'].items():
            if d in self._grid_info:
                plane['index'] = self._grid_info[d][0]
                plane['position'] = None
        return g

    def _RebuildGrid(self):
        info = self._scene.grid_layer.Rebuild(self._s['grid'])
        if info:
            self._grid_info = info
        elif self._scene.grid_layer.lines is not None:
            gl = self._scene.grid_layer
            self._grid_info = {}
            for n, d in enumerate('xyz'):
                idx = gl.ResolveIndex(n, self._s['grid']['planes'][d])[0]
                self._grid_info[d] = (idx, float(gl.lines[n][idx]), len(gl.lines[n]), False)
        return dict(self._grid_info)

    def GetGridPlaneInfo(self):
        """ ``{dir: (index, coordinate, number of lines, clamped)}`` of the grid planes. """
        return dict(self._grid_info)

    def GetNearestGridLines(self, world_pos):
        """ Index and coordinate of the nearest mesh line per direction. """
        return self._scene.grid_layer.Nearest(world_pos)

    # ------------------------------------------------------------------
    # clip, isolate, highlight, pick
    def SetClip(self, clip=None, render=True, **kw):
        """ SetClip(clip=None, **kw)

        Section plane: ``normal`` (``'x'``, ``'y'``, ``'z'`` or a vector),
        ``position`` along it and ``invert``; None disables it. Applies to
        the properties, not to the grid.

        Examples
        --------
        >>> viewer.SetClip(normal='z', position=1.6)
        """
        if clip is None and kw:
            clip = kw
        elif clip is not None:
            clip = dict(clip, **kw)
        if clip is not None:
            for k in clip:
                if k not in ('normal', 'position', 'invert'):
                    raise ValueError('unknown clip key "{}"'.format(k))
            clip = dict(normal=clip.get('normal', 'z'), position=float(clip.get('position', 0.0)),
                        invert=bool(clip.get('invert', False)))
        self._scene.SetClip(clip)
        self._Changed(render)

    def GetClip(self):
        c = self._scene.clip_state
        return None if c is None else dict(c)

    def Isolate(self, names, render=True):
        """ Isolate(names)

        Show only the given properties (a name or list), without changing
        the structure. None shows all again.
        """
        if names is not None:
            if isinstance(names, str):
                names = [names]
            names = [self._scene.Entry(n).name for n in names]
        self._scene.isolate = None if names is None else list(names)
        self._scene.ApplyAll()
        self._Changed(render)

    def GetIsolate(self):
        return None if self._scene.isolate is None else list(self._scene.isolate)

    def Highlight(self, target, dim_others=False, render=True):
        """ Highlight(target, dim_others=False)

        Outline properties or single primitives in a highlight colour.

        Parameters
        ----------
        target : str, dict, tuple, list or None
            A property name, ``(name, primitive_id)``, a dict
            ``{'prop': name, 'primitive': id}``, or a list of these. None
            clears the highlight.
        dim_others : bool
            Draw everything else nearly transparent.
        """
        if isinstance(target, dict) and 'target' in target:
            dim_others = target.get('dim_others', dim_others)
            target = target['target']
        targets = []
        if target is not None:
            if isinstance(target, (str, dict)) or (isinstance(target, tuple) and len(target) == 2
                                                   and isinstance(target[1], (int, np.integer))):
                target = [target]
            for t in target:
                if isinstance(t, dict):
                    targets.append((self._scene.Entry(t['prop']).name, t.get('primitive')))
                elif isinstance(t, (tuple, list)):
                    targets.append((self._scene.Entry(t[0]).name, int(t[1])))
                else:
                    targets.append((self._scene.Entry(t).name, None))
        self._scene.highlight = targets
        self._scene.dim_others = bool(dim_others) and bool(targets)
        self._scene.ApplyAll()
        self._Changed(render)

    def GetHighlight(self):
        t = self._scene.highlight
        if not t:
            return None
        lst = [n if pid is None else dict(prop=n, primitive=pid) for n, pid in t]
        if self._scene.dim_others:
            return dict(target=lst, dim_others=True)
        return lst

    def SetPicking(self, val):
        """ SetPicking(val)

        Whether a left click in the window selects the object under the mouse
        (in the GUI). An interaction setting, not part of the view state.
        """
        self._picking = bool(val)

    def GetPicking(self):
        return self._picking

    def SetPrimitiveSelection(self, val):
        """ SetPrimitiveSelection(val)

        Whether selecting (by clicking, and in the GUI tree) goes down to
        single primitives; off, it selects whole properties and a
        highlighted primitive becomes its property. An interaction setting,
        not part of the view state.
        """
        self._select_primitives = bool(val)
        if not val and any(pid is not None for _, pid in self._scene.highlight):
            names = list(dict.fromkeys(n for n, _ in self._scene.highlight))
            self.Highlight(names, dim_others=self._scene.dim_others)
        self._TreeChanged()

    def GetPrimitiveSelection(self):
        return self._select_primitives

    def Pick(self, x, y):
        """ Pick(x, y)

        The nearest object at display position (x, y), as dict ``{prop,
        primitive_id, primitive, world_pos}`` for the structure, ``{item,
        world_pos}`` (the item's path) for a shown item, or None.
        """
        picker = vtkCellPicker()
        picker.SetTolerance(0.002)
        actors = {}
        for e in self._scene.entries:
            for a in (e.surf_actor, e.wire_actor):
                if a is not None and a.GetVisibility():
                    actors[a.GetAddressAsString('vtkActor')] = (e, a)
                    picker.AddPickList(a)
        for path, it in self._scene.items.items():
            if not it.GetVisible():
                continue
            for a in it.GetProps():
                if isinstance(a, vtkActor) and a.GetVisibility():
                    actors[a.GetAddressAsString('vtkActor')] = (path, a)
                    picker.AddPickList(a)
        if not actors:
            return None
        picker.PickFromListOn()
        if not picker.Pick(x, y, 0, self._renderer):
            return None
        a = picker.GetActor()
        if a is None:
            return None
        e, actor = actors.get(a.GetAddressAsString('vtkActor'), (None, None))
        if e is None:
            return None
        if isinstance(e, str):
            return dict(item=e, world_pos=tuple(picker.GetPickPosition()))
        cid = picker.GetCellId()
        data = actor.GetMapper().GetInput()
        pid = int(data.GetCellData().GetArray('primitive_id').GetValue(cid))
        pidx = int(data.GetCellData().GetArray('primitive_index').GetValue(cid))
        prim = e.prop.GetPrimitive(pidx) if pidx >= 0 else None
        return dict(prop=e.name, primitive_id=pid, primitive=prim, world_pos=tuple(picker.GetPickPosition()))

    # ------------------------------------------------------------------
    # scene tree and items
    def AddAction(self, action, toolbar=None):
        """ AddAction(action, toolbar=None)

        Add a user-facing operation to this viewer, e.g. from an application
        building on it. It gets its key binding in the plain VTK window and
        the GUI, an entry in the GUI menu named after ``action.group``, and a
        toolbar button if `toolbar` names a toolbar (an existing one such as
        ``'View'``, or a new one). An action with the same name as an
        existing one replaces it.

        Parameters
        ----------
        action : :class:`CSXCAD.Viewer.Actions.Action`
        toolbar : str, optional
            Toolbar name.

        Examples
        --------
        >>> from CSXCAD.Viewer.Actions import Action
        >>> v.AddAction(Action('run', 'Run simulation', lambda v: start(), shortcut='Ctrl+R',
        ...                    group='Simulation'), toolbar='Simulation')
        """
        self._extra_actions = [(a, t) for a, t in self._extra_actions if a.name != action.name]
        self._extra_actions.append((action, toolbar))
        self._host.ActionAdded(action, toolbar)

    def GetActions(self):
        """ All actions of this viewer: the built-in ones and those added with
        :meth:`AddAction` (which replace built-in ones of the same name). """
        from CSXCAD.Viewer import Actions
        extra = [a for a, t in self._extra_actions]
        names = {a.name for a in extra}
        return [a for a in Actions.ACTIONS if a.name not in names] + extra

    def GetActionToolbar(self, name):
        """ Toolbar name given to an action added with :meth:`AddAction`, or None. """
        return next((t for a, t in self._extra_actions if a.name == name), None)

    def AddGroup(self, path, label=None):
        """ AddGroup(path, label=None)

        Add a group to the scene tree (parents are created on demand).
        """
        node = self._scene.tree.Add(path, label or SplitPath(path)[-1], 'group')
        if label is not None:
            node.label = label
        self._TreeChanged()
        return path

    def RemoveGroup(self, path, render=True):
        """ Remove a group and all items in it. """
        node = self._scene.tree.Get(path)
        if node.kind != 'group' or path in ('properties', 'items') or path.startswith('properties/'):
            raise ValueError('"{}" is no removable group'.format(path))
        for leaf in self._scene.tree.Leaves(node):
            if leaf.kind == 'item':
                self.RemoveItem(self._scene.items[leaf.path], render=False)
        self._scene.tree.Remove(path)
        self._TreeChanged()
        self._Changed(render)

    def GetTree(self, path=None):
        """ GetTree(path=None)

        The scene tree as nested dicts ``{path, label, kind, visible,
        children}``; the whole tree as a list of roots without `path`.
        """
        def node_dict(n):
            return dict(path=n.path, label=n.label, kind=n.kind, visible=self.GetVisible(n.path),
                        children=[node_dict(c) for c in n.children])
        if path is None:
            return [node_dict(n) for n in self._scene.tree.roots]
        return node_dict(self._scene.tree.Get(path))

    def GetVisible(self, path):
        """ GetVisible(path)

        Visibility of a scene-tree node: True, False, or None for a group
        with mixed (or no) content.
        """
        node = self._scene.tree.Get(path)
        if node.kind == 'property':
            return self._scene.Entry(path).IsVisible()
        if node.kind == 'item':
            return self._scene.items[path].GetVisible()
        if node.kind == 'grid':
            g = self._s['grid']
            if path == 'grid':
                return bool(g['visible'])
            part = path.split('/')[1]
            if part in ('x', 'y', 'z'):
                return bool(g['planes'][part]['visible'])
            if part == 'outline':
                return bool(g['outline'])
            return g['quality'] is not None
        vals = [self.GetVisible(l.path) for l in self._scene.tree.Leaves(node)]
        vals = [v for v in vals if v is not None]
        if not vals:
            return None
        if all(vals):
            return True
        if not any(vals):
            return False
        return None

    def SetVisible(self, path, val, render=True):
        """ SetVisible(path, val)

        Show or hide any scene-tree node. A group applies the value to every
        leaf below it. Property visibility is written to the property; a
        property shown while others are isolated (:meth:`Isolate`) is added
        to them.

        Examples
        --------
        >>> viewer.SetVisible('properties/dumps', False)
        >>> viewer.SetVisible('grid', False)
        """
        val = bool(val)
        node = self._scene.tree.Get(path)
        with self.Batch(render):
            if node.kind == 'property':
                e = self._scene.Entry(path)
                e.prop.SetVisibility(val)
                # shown while others are isolated: it joins the isolated ones
                iso = self._scene.isolate
                if val and iso is not None and e.name not in iso:
                    iso.append(e.name)
                e.Apply()
            elif node.kind == 'item':
                it = self._scene.items[path]
                it._visible = val
                for p in it.GetProps():
                    p.SetVisibility(val)
                if val:
                    self._clock.ItemShown(it)
                self._CallHook(it.OnVisibilityChanged, val)
            elif node.kind == 'grid':
                part = path.split('/')[1] if '/' in path else None
                if part is None:
                    self.SetGrid(visible=val, render=False)
                elif part in ('x', 'y', 'z'):
                    self.SetGrid(planes={part: dict(visible=val)}, render=False)
                elif part == 'outline':
                    self.SetGrid(outline=val, render=False)
                else:
                    q = self._s['grid']['quality']
                    self.SetGrid(quality=(q or 'ratio') if val else None, render=False)
            else:
                for leaf in self._scene.tree.Leaves(node):
                    self.SetVisible(leaf.path, val, render=False)
            self._scene.UpdateLabels()
            self._dirty = True
        self._TreeChanged()

    def AddItem(self, name, props=None, group=None, visible=True, include_in_bounds=False,
                exportable=True, render=True):
        """ AddItem(name, props=None, group=None, visible=True, include_in_bounds=False, exportable=True)

        Show external VTK content as a named item in the scene tree. A name
        already used in the group gets a number, e.g. ``'arrow (2)'``.

        Parameters
        ----------
        name : str or ViewItem
            Item name, or a ready :class:`ViewItem` (subclass) instance.
        props : vtkProp or list of vtkProp
            The actors (2D actors such as scalar bars allowed).
        group : str, optional
            Scene-tree group path; missing groups are created. Default: the
            item's :attr:`ViewItem.DEFAULT_GROUP` (``'items'``).

        Returns
        -------
        ViewItem
        """
        if isinstance(name, ViewItem):
            item = name
            if props is not None:
                for p in (props if isinstance(props, (list, tuple)) else [props]):
                    item._props.append(p)
        else:
            item = ViewItem(name, props, include_in_bounds=include_in_bounds, exportable=exportable)
        if item.GetViewer() is not None:
            raise ValueError('item "{}" already belongs to a viewer'.format(item.GetName()))
        if group is None:
            group = item.DEFAULT_GROUP
        base, n = item.GetName(), 1
        path = JoinPath(group, EscapeName(base))
        while path in self._scene.tree.nodes:
            n += 1
            item._name = '{} ({})'.format(base, n)
            path = JoinPath(group, EscapeName(item.GetName()))
        if group not in self._scene.tree.nodes:
            self._scene.tree.Add(group, SplitPath(group)[-1], 'group')
        item._viewer = weakref.ref(self)
        item._group = group
        item._visible = bool(visible)
        parts = SplitPath(path)
        for n in range(len(parts), 0, -1):
            pending = self._pending_visible.get('/'.join(parts[:n]))
            if pending is not None:
                item._visible = pending
                break
        self._scene.tree.Add(path, item.GetName(), 'item')
        self._scene.items[path] = item
        for p in item.GetProps():
            self._scene.AddItemProp(item, p)
        self._CallHook(item.OnAdded, self)
        pending = self._pending_items.pop(path, None)
        if pending:
            self._ApplyItemParameters(item, pending)
        if item.IsIncludedInBounds() and self._auto_fit:
            # a view that was only fitted so far includes the new item
            self.Fit(render=False)
        self._TreeChanged()
        self._Changed(render)
        return item

    def RemoveItem(self, item, render=True):
        """ Remove an item (a ViewItem or its tree path). """
        if not isinstance(item, ViewItem):
            item = self._scene.items[item]
        path = item.GetPath()
        self._clock.Remove(item)
        for _, bar in [l for l in self._legends if l[0] is item]:
            self.RemoveLegend(item, bar, render=False)
        for p in item.GetProps():
            self._scene.RemoveItemProp(p)
        self._scene.items.pop(path, None)
        self._scene.tree.Remove(path)
        item._viewer = None
        item._group = None
        self._CallHook(item.OnRemoved)
        self._TreeChanged()
        self._Changed(render)

    def GetItems(self):
        """ ``{path: ViewItem}`` of all items. """
        return dict(self._scene.items)

    def _Item(self, item):
        if isinstance(item, ViewItem):
            return item
        if item not in self._scene.items:
            raise KeyError('unknown item path "{}"'.format(item))
        return self._scene.items[item]

    def GetItemParameters(self, item):
        """ GetItemParameters(item)

        Settings of an item (a ViewItem or its tree path): its
        :meth:`ViewItem.GetParameters` plus the generic ``opacity``, with
        ``help`` texts from :attr:`ViewItem.HELP` where a setting has none.
        """
        item = self._Item(item)
        params = [dict(p) for p in item.GetParameters() or []]
        if not any(p['name'] == 'opacity' for p in params):
            params.append(dict(name='opacity', type='float', value=item.GetOpacity(), min=0.0, max=1.0,
                               step=0.05, default=1.0, section='Display',
                               help='Opacity of the whole item (1: opaque).'))
        for p in params:
            if 'help' not in p and p['name'] in item.HELP:
                p['help'] = item.HELP[p['name']]
        return params

    def SetItemParameter(self, item, name, value, render=True):
        """ SetItemParameter(item, name, value)

        Change a setting of an item (a ViewItem or its tree path), see
        :meth:`GetItemParameters`. A ``'slider'`` value is snapped to the
        nearest of its values.

        Examples
        --------
        >>> viewer.SetItemParameter('results/fields/Et', 'opacity', 0.5)
        """
        item = self._Item(item)
        own = list(item.GetParameters() or [])
        p = next((q for q in own if q['name'] == name), None)
        if p is not None and p.get('type') == 'slider' and len(p.get('values', ())):
            vals = np.asarray(p['values'], dtype=float)
            value = float(vals[np.argmin(np.abs(vals - float(value)))])
        if name == 'opacity' and p is None:
            item.SetOpacity(value)
        else:
            item.SetParameter(name, value)
        self._TreeChanged()
        self._Changed(render)

    def _ApplyItemParameters(self, item, params):
        for name, value in params.items():
            try:
                self.SetItemParameter(item, name, value, render=False)
            except Exception as err:
                warnings.warn('item "{}": cannot set "{}": {!r}'.format(item.GetPath(), name, err))

    def GetStructureBounds(self):
        """ GetStructureBounds()

        Bounds ``(xmin, xmax, ymin, ymax, zmin, zmax)`` of the visible
        properties (all properties if none is visible), without grid and
        items, e.g. as size reference for an item; None without properties.
        """
        return self._scene.StructureBounds()

    # -- legends
    def AddLegend(self, item, scalar_bar, render=True):
        """ AddLegend(item, scalar_bar)

        Show a colour legend (a ``vtkScalarBarActor``) of an item. The viewer
        places all legends side by side on the right, stacked when there are
        many, hides them with their item and colours their text to suit the
        background. The mouse drags a legend elsewhere and a double click
        puts it back (see :meth:`SetLegendPositions`). The bar becomes one of
        the item's props.
        """
        item = self._Item(item)
        if scalar_bar not in item.GetProps():
            item.AddProp(scalar_bar)
        if not any(b is scalar_bar for _, b in self._legends):
            self._legends.append((item, scalar_bar))
        self._Changed(render)

    def RemoveLegend(self, item, scalar_bar=None, render=True):
        """ RemoveLegend(item, scalar_bar=None)

        Remove a legend of an item (all of its legends without `scalar_bar`).
        """
        item = self._Item(item)
        for it, bar in list(self._legends):
            if it is item and (scalar_bar is None or bar is scalar_bar):
                self._legends.remove((it, bar))
                if bar in item.GetProps():
                    item.RemoveProp(bar)
        self._Changed(render)

    def GetLegends(self):
        """ List of ``(item, scalar_bar)`` of all legends. """
        return list(self._legends)

    def SetLegendPositions(self, positions, render=True):
        """ SetLegendPositions(positions)

        Legends moved out of the automatic layout (by dragging them; a double
        click puts one back): ``{item path: (x, y)}``, the lower left corner
        in fractions of the view; an item's further legends are
        ``'<path>#1'``, ... Empty: all placed automatically.
        """
        self._s['legends'] = {str(k): (float(v[0]), float(v[1])) for k, v in dict(positions or {}).items()}
        self._Changed(render)

    def GetLegendPositions(self):
        return dict(self._s['legends'])

    def _LegendKey(self, item, bar):
        n = [b for it, b in self._legends if it is item].index(bar)
        return item.GetPath() if n == 0 else '{}#{}'.format(item.GetPath(), n)

    def _LegendAt(self, x, y):
        """ (item, bar) of the shown legend at display position x, y, or None. """
        for item, bar in reversed(self._legends):
            if not (item.GetVisible() and bar.GetVisibility()):
                continue
            x0, y0 = bar.GetPositionCoordinate().GetComputedDisplayValue(self._renderer)
            x1, y1 = bar.GetPosition2Coordinate().GetComputedDisplayValue(self._renderer)
            if min(x0, x1) <= x <= max(x0, x1) and min(y0, y1) <= y <= max(y0, y1):
                return item, bar
        return None

    def _LegendLayout(self, w_px):
        """ (shown (item, bar), laid-out bars, cols, width, height) of the
        legends in a view `w_px` pixels wide; widths and heights are fractions. """
        shown = [(it, b) for it, b in self._legends if it.GetVisible() and b.GetVisibility()]
        if not shown:
            return shown, [], 0, 0.0, 0.0
        # a column needs about 120 px for its title; stack when they do not fit
        width = max(0.1, 120.0 / max(w_px, 1))
        rows = 1 if len(shown) * width <= 0.5 else 2
        width = min(width, 0.6 / -(-len(shown) // rows))
        height = 0.7 if rows == 1 else 0.4
        # dragged ones keep their place, the others are laid out
        moved = self._s['legends']
        bars = [b for it, b in shown if self._LegendKey(it, b) not in moved]
        cols = max(1, -(-len(bars) // rows))
        return shown, bars, cols, width, height

    def _LegendShare(self, w_px):
        """ Fraction of the view width the laid-out legends take on the right. """
        _, bars, cols, width, _ = self._LegendLayout(w_px)
        # the margin on the right, the columns and a gap for wider titles
        return 0.04 + cols * width + 0.02 if bars else 0.0

    def _PlaceLegends(self, obj=None, event=None):
        if self._scene.freeze_2d:
            return
        shown, bars, cols, width, height = self._LegendLayout(self._renderer.GetSize()[0])
        if not shown:
            return
        moved = self._s['legends']
        color = self._scene.TextColor()
        for n, bar in enumerate(bars):
            row, col = divmod(n, cols)
            # titles are centred over the bar and may be wider: keep a margin
            bar.SetPosition(1.0 - 0.04 - (cols - col) * width, 0.95 - (row + 1) * (height + 0.03))
            bar.SetWidth(width)
            bar.SetHeight(height)
        for it, bar in shown:
            pos = moved.get(self._LegendKey(it, bar))
            if pos is not None:
                bar.SetPosition(*pos)
                bar.SetWidth(width)
                bar.SetHeight(height)
            for tp in (bar.GetTitleTextProperty(), bar.GetLabelTextProperty()):
                tp.SetColor(color)
                tp.ShadowOff()

    # -- values under the mouse
    def GetItemValueText(self, x, y):
        """ GetItemValueText(x, y)

        Text of the item value at display position (x, y), from
        :meth:`ViewItem.GetValueText` of the visible item under it, or None.
        A prop whose item has no text (e.g. a decoration) does not hide
        the props behind it.
        """
        picker = vtkCellPicker()
        picker.SetTolerance(0.0005)
        actors = {}
        for it in self._scene.items.values():
            if not it.GetVisible():
                continue
            for p in it.GetProps():
                if isinstance(p, vtkActor) and p.GetVisibility():
                    actors[p.GetAddressAsString('vtkActor')] = (it, p)
                    picker.AddPickList(p)
        picker.PickFromListOn()
        for _ in range(4):
            if not actors or not picker.Pick(x, y, 0, self._renderer):
                return None
            a = picker.GetActor()
            it, prop = actors.pop(a.GetAddressAsString('vtkActor'), (None, None)) if a is not None else (None, None)
            if it is None:
                return None
            text = self._CallHook(it.GetValueText, tuple(picker.GetPickPosition()), prop, picker.GetCellId())
            if text:
                return text
            picker.DeletePickList(prop)
        return None

    def _OnHover(self, obj, event):
        if self._host.handles_input or self._interacting or not self._scene.items:
            return
        now = time.monotonic()
        if now - self._hover_time < 0.05:
            return
        self._hover_time = now
        text = self.GetItemValueText(*obj.GetEventPosition())
        if text == self._hover_text:
            return
        self._hover_text = text
        if self._hover_actor is None:
            self._hover_actor = vtkTextActor()
            tp = self._hover_actor.GetTextProperty()
            tp.SetFontSize(14)
            tp.SetVerticalJustificationToTop()
            self._hover_actor.GetPositionCoordinate().SetCoordinateSystemToNormalizedViewport()
            self._hover_actor.SetPosition(0.01, 0.99)
            self._renderer.AddViewProp(self._hover_actor)
        self._hover_actor.GetTextProperty().SetColor(self._scene.TextColor())
        self._hover_actor.SetInput(text or '')
        self._hover_actor.SetVisibility(bool(text))
        self.Render()

    def _CallHook(self, fn, *args):
        try:
            return fn(*args)
        except Exception as err:
            warnings.warn('{} failed: {!r}'.format(getattr(fn, '__qualname__', fn), err))
            return None

    def _TreeChanged(self):
        self._host.TreeChanged()

    # ------------------------------------------------------------------
    # animation
    def AddAnimation(self, target, n_steps, callback=None):
        """ AddAnimation(target, n_steps, callback=None)

        Register an item on the global animation clock.

        Parameters
        ----------
        target : ViewItem or str
            The item, or its scene-tree path.
        n_steps : int
            Number of steps of one loop of this item.
        callback : callable, optional
            ``callback(step)``; must be cheap and must not render. Without it
            the item's :meth:`ViewItem.OnAnimationStep` is called.

        Returns
        -------
        Animation
            Handle with ``Remove()`` and ``Invalidate(steps=None)``.
        """
        if not isinstance(target, ViewItem):
            if target not in self._scene.items:
                raise KeyError('unknown item path "{}"'.format(target))
            target = self._scene.items[target]
        return self._clock.Add(target, n_steps, callback)

    def Play(self):
        """ Start the animation (prepares all steps first). """
        self._clock.Play()

    def Pause(self):
        """ Pause the animation. """
        self._clock.Pause()

    def Stop(self):
        """ Stop the animation (pause and go to step 0); cancels preparation. """
        self._clock.Stop()

    def SetAnimationStep(self, step, render=True):
        """ Go to the global animation tick `step`. """
        self._clock.SetTick(step)
        self._Changed(render)

    def GetAnimationStep(self):
        return self._clock.tick

    def StepAnimation(self, delta=1, render=True):
        """ Step the animation by `delta` ticks (wraps). """
        self._clock.Step(delta)
        self._Changed(render)

    def SetFps(self, fps, render=True):
        """ Animation speed in frames per second. """
        self._clock.fps = float(fps)

    def GetFps(self):
        return self._clock.fps

    def SetAnimation(self, anim, render=True):
        """ SetAnimation(anim)

        Animation state dict ``{step, fps, playing}``.
        """
        for k in anim:
            if k not in ('step', 'fps', 'playing'):
                raise ValueError('unknown animation key "{}"'.format(k))
        if 'fps' in anim:
            self.SetFps(anim['fps'])
        if 'step' in anim:
            self.SetAnimationStep(int(anim['step']), render=False)
        if anim.get('playing'):
            self._clock.Play()
        elif 'playing' in anim:
            self._clock.Pause()
        self._Changed(render)

    def GetAnimation(self):
        c = self._clock
        return dict(step=int(c.tick), fps=c.fps, playing=bool(c.playing))

    def GetAnimationSteps(self):
        """ Number of ticks of one animation loop (0 without animations). """
        return self._clock.NumTicks()

    def ExportAnimation(self, path, fmt='png', size=None, scale=1, loops=1, progress=None):
        """ ExportAnimation(path, fmt='png', size=None, scale=1, loops=1)

        Render every tick of the animation off screen at full quality.

        Parameters
        ----------
        path : str
            Directory for ``fmt='png'`` (``frame_0000.png``, ...), otherwise
            the movie file.
        fmt : str
            ``'png'``, ``'gif'`` (needs Pillow), ``'mp4'`` or ``'ogv'`` (need a
            VTK build with FFMPEG / Ogg Theora).
        """
        return self._clock.Export(path, fmt, size, scale, loops, progress)

    # ------------------------------------------------------------------
    # output
    @contextlib.contextmanager
    def _Capturing(self, size=None, scale=1, transparent=False):
        """ Prepare a capture of `size` x `scale` pixels; yields ``(window,
        tiles)``. An attached window renders it itself, in tiles and in a
        part of the window for another aspect ratio (moving the scene to
        another window would mean another OpenGL context). Without one, the
        viewer's own offscreen window gets that size. """
        size = tuple(int(s) for s in (size or self.GetSize()))
        rw = self._rw
        in_window = rw is not None and self._onscreen
        fit_old = self._fit_size
        fast = self._fast
        if fast:
            self._SetFast(False)
        r, ar = self._renderer, self._axes_renderer
        grad = r.GetGradientBackground()
        vp, avp = r.GetViewport(), ar.GetViewport()
        if transparent:
            r.GradientBackgroundOff()
            r.SetBackgroundAlpha(0.0)
        try:
            if in_window:
                win = tuple(rw.GetSize())
                w, h = size[0] * scale, size[1] * scale
                tiles = max(1, int(math.ceil(max(w / win[0], h / win[1]))))
                fx, fy = w / (tiles * win[0]), h / (tiles * win[1])
                r.SetViewport(0, 0, fx, fy)
                ar.SetViewport(0, 0, avp[2] * fx, avp[3] * fy)
                rw.SetDPI(int(round(72 * scale)))
                rw.SwapBuffersOff()   # nothing of this shows in the window
                target = rw
            else:
                target = self._EnsureWindow()
                target.SetSize(size[0] * scale, size[1] * scale)
                target.SetDPI(int(72 * scale))
                tiles = 1
            self._fit_size = size
            yield target, tiles
        finally:
            self._fit_size = fit_old
            if transparent:
                r.SetBackgroundAlpha(1.0)
                r.SetGradientBackground(grad)
            if in_window:
                r.SetViewport(*vp)
                ar.SetViewport(*avp)
                rw.SetDPI(72)
                rw.SwapBuffersOn()
            elif self._rw is not None:
                self._rw.SetDPI(72)
                self._rw.SetSize(*self.GetSize())
            if fast:
                self._SetFast(True)

    def Capture(self, size=None, scale=1, transparent_background=False):
        """ Capture(size=None, scale=1, transparent_background=False)

        Render the current view at full quality and return it as (H, W, 3|4)
        uint8 numpy array (RGBA with a transparent background).
        """
        size = tuple(int(s) for s in (size or self.GetSize()))
        with self._Capturing(size, scale, transparent_background) as (rw, tiles):
            # places labels and legends; in tiles VTK then moves them per tile
            rw.Render()
            w2i = vtkWindowToImageFilter()
            w2i.SetInput(rw)
            w2i.ReadFrontBufferOff()
            if tiles > 1:
                w2i.SetScale(tiles)
                w2i.SetFixBoundary(True)
            if transparent_background:
                w2i.SetInputBufferTypeToRGBA()
            else:
                w2i.SetInputBufferTypeToRGB()
            self._scene.freeze_2d = tiles > 1
            try:
                w2i.Update()
            finally:
                self._scene.freeze_2d = False
            img = w2i.GetOutput()
            w, h, _ = img.GetDimensions()
            arr = vtk_to_numpy(img.GetPointData().GetScalars()).reshape(h, w, -1)
            # the image is bottom-up; the view is its lower-left part
            arr = arr[:size[1] * scale, :size[0] * scale][::-1].copy()
        return arr

    def Screenshot(self, filename=None, size=None, scale=1, transparent_background=False):
        """ Screenshot(filename=None, size=None, scale=1, transparent_background=False)

        Save the current view as PNG, in 2D and 3D, on and off screen.

        Parameters
        ----------
        filename : str, optional
            Without it the image is returned as (H, W, 3|4) uint8 array, or
            shown inline in Jupyter.
        size : (int, int), optional
            Image size; default is the current size.
        scale : int
            Render at `scale` times the size (sharper documentation images).
        transparent_background : bool
            Write RGBA with a transparent background.
        """
        img = self.Capture(size, scale, transparent_background)
        if filename is None:
            if InJupyter():
                _ShowInline(ImageToPNG(img))
                return None
            return img
        ImageToPNG(img, filename)
        return filename

    def RenderViews(self, filename=None, views=('iso', '+x', '-x', '+y', '-y', '+z', '-z'),
                    tile_size=(600, 450), columns=4, labels=True, **state_overrides):
        """ RenderViews(filename=None, views=('iso', '+x', '-x', '+y', '-y', '+z', '-z'), tile_size=(600, 450), columns=4, labels=True, **state_overrides)

        Render the structure from several directions into one labelled
        contact sheet, e.g. to inspect all sides with a single image.

        Parameters
        ----------
        views : sequence
            Named views (see :meth:`SetView`) or camera dicts.
        **state_overrides
            View state keys applied for these renders only.

        Returns
        -------
        list of ndarray
            The tiles. The sheet is written to `filename` if given.
        """
        saved = self.GetState()
        tiles = []
        label_actor = None
        try:
            if state_overrides:
                self.SetState(state_overrides, render=False)
            if labels:
                label_actor = vtkTextActor()
                tp = label_actor.GetTextProperty()
                tp.SetFontSize(18)
                tp.SetColor(self._scene.TextColor())
                tp.SetVerticalJustificationToTop()
                label_actor.GetPositionCoordinate().SetCoordinateSystemToNormalizedViewport()
                label_actor.SetPosition(0.02, 0.97)
                self._renderer.AddViewProp(label_actor)
            self._s['size'] = tuple(tile_size)
            self._fit_size = tuple(tile_size)
            for v in views:
                if isinstance(v, dict):
                    self.SetCamera(v, render=False)
                    text = 'camera'
                else:
                    self.SetView(v, render=False)
                    text = v
                if label_actor is not None:
                    label_actor.SetInput(text)
                tiles.append(self.Capture(tile_size))
        finally:
            self._fit_size = None
            if label_actor is not None:
                self._renderer.RemoveViewProp(label_actor)
            self.SetState(saved, render=False)
            self.SetSize(saved['size'], render=False)
            self.Render()
        if filename is not None or InJupyter():
            rows = int(math.ceil(len(tiles) / float(columns)))
            cols = min(columns, len(tiles))
            h, w = tiles[0].shape[:2]
            bg = np.array([int(c * 255) for c in self._scene.background], dtype=np.uint8)
            sheet = np.empty((rows * h, cols * w, 3), dtype=np.uint8)
            sheet[:] = bg
            for i, t in enumerate(tiles):
                r, c = divmod(i, cols)
                sheet[r * h:(r + 1) * h, c * w:(c + 1) * w] = t[:, :, :3]
            if filename is not None:
                ImageToPNG(sheet, filename)
            else:
                _ShowInline(ImageToPNG(sheet))
        return tiles

    def Describe(self, fmt='dict'):
        """ Describe(fmt='dict')

        Machine-readable summary of the scene: units, bounding box,
        properties, grid statistics, unrendered primitives, the scene tree,
        camera and size. ``fmt='text'`` gives a compact text version.
        """
        from CSXCAD.Viewer import Describe
        return Describe.Describe(self, fmt)

    def Export(self, fmt, path, **opts):
        """ Export(fmt, path, **opts)

        Export geometry or the scene, see :mod:`CSXCAD.Viewer.Export`
        (``'stl'``, ``'ply'``, ``'vtp'``, ``'gltf'``, ``'glb'``, ``'svg'``,
        ``'pdf'``, ``'eps'``, ``'png'``).
        """
        from CSXCAD.Viewer import Export
        return Export.Export(self, fmt, path, **opts)

    # ------------------------------------------------------------------
    # showing
    def Show(self, gui=None, block=None, window_class=None):
        """ Show(gui=None, block=True, window_class=None)

        Open a window on this viewer. The viewer outlives the window: scene,
        state and items are there again on the next ``Show()``, and a changed
        structure is updated first.

        Parameters
        ----------
        gui : bool, optional
            True: the PySide6 GUI, False: a plain VTK window. Default: the GUI
            if PySide6 can be used.
        block : bool
            False opens the GUI and returns at once; a later ``plt.show()``
            or :func:`Exec` then runs one event loop for all windows. Only
            the Qt GUI supports this; the plain VTK window always blocks.
        window_class : type, optional
            A subclass of :class:`CSXCAD.Viewer.GUI.MainWindow.MainWindow`
            for the GUI, e.g. from an application extending it.

        Returns
        -------
        dict or None
            The final view state (GUI and plain window).

        Notes
        -----
        With the environment variable ``CSXCAD_VIEWER_OUTPUT=<dir>`` no
        window opens: ``<name>_view.png``, ``<name>_views.png`` and
        ``<name>_describe.json`` are written to that directory instead,
        numbered on repeated calls. In Jupyter an image is shown inline.
        """
        if gui is None:
            gui = self._gui_default
        if block is None:
            block = self._block_default
        if self.CSX is not None:
            self.Update(render=False)
        out = OutputDir()
        if out is not None:
            return self._WriteOutput(out)
        if InJupyter():
            self.Screenshot()
            return None
        if gui is None or gui:
            try:
                from CSXCAD.Viewer import GUI
                ok, reason = GUI.CanUseGUI()
            except ImportError as err:
                ok, reason = False, str(err)
            if ok:
                return GUI.ShowGUI(self, block=block, window_class=window_class)
            if gui:
                raise RuntimeError('cannot open the GUI: {}'.format(reason))
            if reason:
                warnings.warn('{}, using the plain VTK window'.format(reason))
        return self._ShowPlain()

    def _WriteOutput(self, out):
        name = self._name or _ScriptName()
        n = _OUTPUT['counts'].get(name, 0) + 1
        _OUTPUT['counts'][name] = n
        base = os.path.join(out, name if n == 1 else '{}_{}'.format(name, n))
        self.Screenshot(base + '_view.png')
        self.RenderViews(base + '_views.png')
        with open(base + '_describe.json', 'w') as f:
            json.dump(self.Describe('dict'), f, indent=1)
        return self.GetState()

    def _ShowPlain(self):
        if not DisplayAvailable():
            raise RuntimeError('no usable display to open a window; use Screenshot()/RenderViews(), '
                               'or set CSXCAD_VIEWER_OUTPUT=<dir> to write the images instead')
        rw = vtkRenderWindow()
        try:
            screen = tuple(rw.GetScreenSize())
        except Exception:
            screen = (0, 0)
        if screen[0] > 0 and screen[1] > 0:
            self.SetHost(_ScreenHost(screen))
        self.SetSize(self._size_spec, render=False)
        rw.SetSize(*self._s['size'])
        pos = self._window_position
        if pos is not None and screen[0] > 0:
            rw.SetPosition(int(round(pos[0] * screen[0])), int(round(pos[1] * screen[1])))
        rw.SetWindowName('CSXCAD Viewer' + (' - ' + self._name if self._name else ''))
        iren = vtkRenderWindowInteractor()
        iren.SetRenderWindow(rw)
        self.Attach(rw, iren)
        iren.Initialize()
        self.Render()
        iren.Start()
        state = self.GetState()
        self.Detach()
        rw.Finalize()
        iren.TerminateApp()
        del iren, rw
        return state

    def Close(self):
        """ Close the window opened by :meth:`Show` (plain window or GUI). """
        if not self._host.Close() and self._iren is not None:
            self._iren.TerminateApp()
            self._iren.ExitCallback()

    @staticmethod
    def Exec():
        """ Run the GUI event loop until the last viewer window is closed. """
        from CSXCAD.Viewer import GUI
        return GUI.Exec()


def _Tuples(v):
    if isinstance(v, dict):
        return {k: _Tuples(x) for k, x in v.items()}
    if isinstance(v, list):
        return tuple(_Tuples(x) for x in v)
    return v


def _Hex(rgba):
    return '#{:02x}{:02x}{:02x}'.format(int(rgba[0]), int(rgba[1]), int(rgba[2]))


def Show(CSX, gui=None, block=True, name=None, window_class=None, **view):
    """ Show(CSX, gui=None, block=True, name=None, window_class=None, **view)

    Show a structure: shorthand for ``Viewer(CSX, **view).Show()``.

    Parameters
    ----------
    CSX : ContinuousStructure or str
        The structure, or an XML file name.
    gui : bool, optional
        True: PySide6 GUI, False: plain VTK window; default: the GUI if
        PySide6 can be used.
    block : bool
        See :meth:`Viewer.Show`.
    name : str, optional
        Base name of output-mode files (default: the script name).
    window_class : type, optional
        See :meth:`Viewer.Show`.
    **view
        View state keys, see :meth:`Viewer.GetState`.

    Returns
    -------
    dict or None
        The final view state.

    Examples
    --------
    >>> from CSXCAD.Viewer import Show
    >>> Show(CSX, render_mode='solid+edges', grid=dict(opacity=0.5))
    """
    v = Viewer(CSX, name=name, gui=gui, block=block, **view)
    return v.Show(window_class=window_class)
