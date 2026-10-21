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

"""CSXCAD.Viewer.Actions
Registry of all user-facing viewer actions.

The GUI toolbars and menus, the key bindings of the plain VTK window and the
action table in the docs are all generated from :data:`ACTIONS`.
"""

import sys


class Action:
    """ Action(name, label, call, icon=None, shortcut=None, group='View', checkable=False, get=None, enabled=None, radio=None)

    One user-facing operation of the viewer.

    Parameters
    ----------
    name : str
        Unique identifier.
    label : str
        Text for menus and tooltips.
    call : callable
        ``call(viewer)``, or ``call(viewer, checked)`` for checkable actions.
    icon : str, optional
        Icon file base name in ``CSXCAD/Viewer/icons``.
    shortcut : str, optional
        Key in Qt notation, e.g. ``'X'``, ``'Shift+X'``, ``'Left'``.
    group : str
        Menu/toolbar group.
    checkable : bool
        Toggle action; `get` returns its state.
    get : callable, optional
        ``get(viewer) -> bool`` for checkable actions.
    enabled : callable, optional
        ``enabled(viewer) -> bool``.
    radio : str, optional
        Name of an exclusive group of checkable actions.
    """

    def __init__(self, name, label, call, icon=None, shortcut=None, group='View', checkable=False,
                 get=None, enabled=None, radio=None):
        self.name = name
        self.label = label
        self.call = call
        self.icon = icon
        self.shortcut = shortcut
        self.group = group
        self.checkable = checkable
        self.get = get
        self.enabled = enabled
        self.radio = radio

    def Trigger(self, viewer):
        if self.enabled is not None and not self.enabled(viewer):
            return
        if self.checkable:
            self.call(viewer, not self.get(viewer))
        else:
            self.call(viewer)


def _HasAnim(v):
    return v.GetAnimationSteps() > 0


def _Screenshot(v):
    fn = 'csxcad_view.png'
    v.Screenshot(fn)
    print('saved {}'.format(fn))


def _ToggleClip(v, on):
    if on:
        b = v._Bounds()
        z = 0.0 if b is None else (b[4] + b[5]) / 2
        v.SetClip(normal='z', position=z)
    else:
        v.SetClip(None)


def _Help(v):
    print(HelpText(v))


def _RenderModeAction(mode, label, key):
    return Action('render_' + mode.replace('+', '_'), label, lambda v, on: v.SetRenderMode(mode),
                  icon='render-' + mode.replace('+', '-'), shortcut=key, group='Display', checkable=True,
                  get=lambda v: v.GetRenderMode() == mode, radio='render_mode')


def _Checkable(name, label, setter, getter, icon, key, group):
    return Action(name, label, lambda v, on: getattr(v, setter)(on), icon=icon, shortcut=key, group=group,
                  checkable=True, get=lambda v: bool(getattr(v, getter)()))


ACTIONS = [
    # view
    Action('fit', 'Fit view', lambda v: v.Fit(), icon='zoom-fit', shortcut='R'),
    Action('view_+x', 'View from +x', lambda v: v.SetView('+x'), icon='view-px', shortcut='X'),
    Action('view_-x', 'View from -x', lambda v: v.SetView('-x'), icon='view-nx', shortcut='Shift+X'),
    Action('view_+y', 'View from +y', lambda v: v.SetView('+y'), icon='view-py', shortcut='Y'),
    Action('view_-y', 'View from -y', lambda v: v.SetView('-y'), icon='view-ny', shortcut='Shift+Y'),
    Action('view_+z', 'View from +z', lambda v: v.SetView('+z'), icon='view-pz', shortcut='Z'),
    Action('view_-z', 'View from -z', lambda v: v.SetView('-z'), icon='view-nz', shortcut='Shift+Z'),
    Action('view_iso_z', 'Isometric view, z up', lambda v: v.SetView('iso_z'), icon='view-iso-z', shortcut='I'),
    Action('view_iso_x', 'Isometric view, x up', lambda v: v.SetView('iso_x'), icon='view-iso-x', shortcut='Shift+I'),
    Action('view_iso_y', 'Isometric view, y up', lambda v: v.SetView('iso_y'), icon='view-iso-y', shortcut='Ctrl+I'),
    Action('flip', 'Look from the opposite side', lambda v: v.Flip(), icon='flip', shortcut='B'),
    Action('roll_left', 'Roll 90° counter-clockwise', lambda v: v.Roll(90), icon='rotate-ccw', shortcut='['),
    Action('roll_right', 'Roll 90° clockwise', lambda v: v.Roll(-90), icon='rotate-cw', shortcut=']'),
    Action('azimuth_left', 'Turn 90° left', lambda v: v.Azimuth(-90), icon='turn-left', shortcut='Left'),
    Action('azimuth_right', 'Turn 90° right', lambda v: v.Azimuth(90), icon='turn-right', shortcut='Right'),
    Action('elevation_up', 'Tilt 15° up', lambda v: v.Elevation(15), icon=None, shortcut='Up'),
    Action('elevation_down', 'Tilt 15° down', lambda v: v.Elevation(-15), icon=None, shortcut='Down'),
    Action('focus_cursor', 'Fly to the point under the mouse', lambda v: v.FocusAtCursor(),
           icon=None, shortcut='F'),
    Action('zoom_in', 'Zoom in', lambda v: v.Zoom(1.25), icon='zoom-in', shortcut='+'),
    Action('zoom_out', 'Zoom out', lambda v: v.Zoom(0.8), icon='zoom-out', shortcut='-'),
    _Checkable('mode_2d', '2D mode', 'SetMode2D', 'GetMode2D', 'mode-2d', '2', 'View'),
    Action('parallel', 'Parallel projection', lambda v, on: v.SetParallelProjection(on), icon='perspective-off',
           shortcut='P', checkable=True, get=lambda v: v.GetParallelProjection(),
           enabled=lambda v: not v.GetMode2D()),
    # display
    _RenderModeAction('solid', 'Solid', 'S'),
    _RenderModeAction('edges', 'Edges', 'E'),
    _RenderModeAction('solid+edges', 'Solid and edges', 'W'),
    Action('transparency', 'See-through', lambda v, on: v.SetTransparency(0.3 if on else None),
           icon='transparency', shortcut='T', group='Display', checkable=True,
           get=lambda v: v.GetTransparency() is not None),
    _Checkable('labels', 'Property labels', 'SetLabels', 'GetLabels', 'labels', 'L', 'Display'),
    _Checkable('axes', 'Axes marker', 'SetAxes', 'GetAxes', 'axes', 'A', 'Display'),
    _Checkable('disc_material', 'Discrete material models', 'SetDiscMaterial', 'GetDiscMaterial', 'disc-material',
               'D', 'Display'),
    Action('clip', 'Section plane', _ToggleClip, icon='clip', shortcut='C', group='Display', checkable=True,
           get=lambda v: v.GetClip() is not None),
    Action('picking', 'Select by clicking', lambda v, on: (v.SetPicking(on), on or v.Highlight(None)), icon='pick',
           group='Display', checkable=True, get=lambda v: v.GetPicking()),
    Action('select_primitives', 'Select primitives', lambda v, on: v.SetPrimitiveSelection(on), icon='primitives',
           group='Display', checkable=True, get=lambda v: v.GetPrimitiveSelection()),
    Action('clear_selection', 'Clear highlight and isolation',
           lambda v: (v.Highlight(None), v.Isolate(None)), icon=None, shortcut='Esc', group='Display'),
    # grid
    Action('grid', 'Mesh grid', lambda v, on: v.SetVisible('grid', on), icon='grid', shortcut='G', group='Grid',
           checkable=True, get=lambda v: bool(v.GetVisible('grid'))),
    Action('grid_outline', 'Mesh outline with ticks', lambda v, on: v.SetGrid(outline=on), icon='outline',
           shortcut='O', group='Grid', checkable=True, get=lambda v: v.GetGrid()['outline']),
    Action('grid_quality', 'Mesh quality (neighbour ratio)',
           lambda v, on: v.SetGrid(quality='ratio' if on else None), icon='quality', shortcut='M', group='Grid',
           checkable=True, get=lambda v: v.GetGrid()['quality'] is not None),
    # animation
    Action('play', 'Play / pause', lambda v, on: v.Play() if on else v.Pause(), icon='play', shortcut='Space',
           group='Animation', checkable=True, get=lambda v: v.GetAnimation()['playing'], enabled=_HasAnim),
    Action('stop', 'Stop', lambda v: v.Stop(), icon='stop', group='Animation', enabled=_HasAnim),
    Action('step_back', 'Step back', lambda v: v.StepAnimation(-1), icon='step-back', shortcut=',',
           group='Animation', enabled=_HasAnim),
    Action('step_forward', 'Step forward', lambda v: v.StepAnimation(1), icon='step-forward', shortcut='.',
           group='Animation', enabled=_HasAnim),
    # file
    Action('screenshot', 'Screenshot', _Screenshot, icon='camera', shortcut='F12', group='File'),
    Action('copy_view', 'Copy view as Python', lambda v: print(v.GetStateAsPython()), icon='copy',
           shortcut='Ctrl+C', group='File'),
    Action('help', 'Keyboard shortcuts', _Help, icon='help', shortcut='H', group='Help'),
]

ACTION_MAP = {a.name: a for a in ACTIONS}


def GetAction(name):
    """ The :class:`Action` named `name`. """
    return ACTION_MAP[name]


_KEYSYMS = {
    'bracketleft': '[', 'bracketright': ']', 'space': 'Space', 'period': '.', 'comma': ',',
    'Escape': 'Esc', 'plus': '+', 'minus': '-', 'KP_Add': '+', 'KP_Subtract': '-', 'equal': '+',
    'Return': 'Return',
}


def KeyName(keysym, ctrl=False, shift=False):
    """ Qt-style shortcut name of a VTK key symbol. """
    key = _KEYSYMS.get(keysym, keysym)
    if len(key) == 1 and key.isalpha():
        if key.isupper():
            shift = True
        key = key.upper()
    if key in ('+', '-', '[', ']', '.', ','):
        shift = False
    return ('Ctrl+' if ctrl else '') + ('Shift+' if shift else '') + key


def HandleKey(viewer, keysym, ctrl=False, shift=False):
    """ Run the action bound to a key (plain VTK window). Returns True if handled. """
    name = KeyName(keysym, ctrl, shift)
    if name == 'Q':
        viewer.Close()
        return True
    for a in viewer.GetActions():
        if a.shortcut == name:
            try:
                a.Trigger(viewer)
            except Exception as err:
                print('action "{}" failed: {}'.format(a.name, err), file=sys.stderr)
            return True
    return False


def ActionTable():
    """ The actions as reStructuredText table (for the docs). """
    rows = [(a.label, '``{}``'.format(a.name), '``{}``'.format(a.shortcut) if a.shortcut else '', a.group)
            for a in ACTIONS]
    head = ('Action', 'Name', 'Key', 'Group')
    widths = [max(len(r[i]) for r in rows + [head]) for i in range(4)]
    line = '  '.join('=' * w for w in widths)
    fmt = lambda r: '  '.join(c.ljust(w) for c, w in zip(r, widths)).rstrip()
    return '\n'.join([line, fmt(head), line] + [fmt(r) for r in rows] + [line])


__doc__ += """
Actions
-------

.. table::

""" + '\n'.join('   ' + l for l in ActionTable().splitlines()) + '\n'


def HelpText(viewer=None):
    """ Table of all actions (of `viewer`, if given) and their shortcuts. """
    actions = ACTIONS if viewer is None else viewer.GetActions()
    lines = ['{:<12} {}'.format(a.shortcut or '', a.label) for a in actions]
    lines.append('{:<12} {}'.format('Q', 'Close the window'))
    return '\n'.join(lines)
