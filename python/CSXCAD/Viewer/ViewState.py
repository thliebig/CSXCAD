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

"""CSXCAD.Viewer.ViewState
View state schema: defaults, validation and JSON conversion.

The view is a flat dict. Every key maps to a setter of
:class:`CSXCAD.Viewer.Viewer`: key ``foo_bar`` to ``SetFooBar()``, except
``isolate`` (``Isolate()``), ``highlight`` (``Highlight()``), ``visible``
(``SetVisible(path, bool)``) and ``items`` (``SetItemParameter(path, name,
value)``).
"""

import copy
import json
import math
import numbers

import numpy as np

VERSION = 1

VIEWS = ('+x', '-x', '+y', '-y', '+z', '-z', 'xy', 'yz', 'xz',
         'iso', 'iso_x', 'iso_y', 'iso_z')
RENDER_MODES = ('solid', 'edges', 'solid+edges')
INTERACTIVE_QUALITY = ('auto', 'full', 'fast')
GRID_QUALITY = (None, 'size', 'ratio')
DIRS = ('x', 'y', 'z')

GRID_DEFAULT = dict(
    visible=True,
    color='black',
    opacity=0.3,
    # at the lowest lines the planes stay behind the structure in the iso views
    planes={d: dict(visible=True, index=0, position=None) for d in DIRS},
    outline=False,
    quality=None,
    quality_threshold=1.5,
)

#: Defaults of all state keys, in the order SetState applies them.
DEFAULTS = dict(
    version=VERSION,
    # the render area: 0.454 of the screen diagonal, aspect 4:3 (800 x 600
    # on the reference screen, e.g. offscreen)
    size=(0.454, 1.333),
    window_position=None,
    background='white',
    arc_resolution=32,
    disc_material=True,
    render_mode='solid',
    transparency=None,
    edge_color=None,
    edge_angle=30.0,
    antialias=True,
    max_peels=100,
    interactive_quality='auto',
    axes=True,
    labels=False,
    grid=GRID_DEFAULT,
    clip=None,
    isolate=None,
    highlight=None,
    visible={},
    items={},
    legends={},
    view=None,
    mode_2d=False,
    camera=None,
    animation=dict(step=0, fps=20, playing=False),
)

#: state key -> (setter, getter) method names of Viewer
ACCESSORS = {
    'size': ('SetSize', 'GetViewSize'),
    'window_position': ('SetWindowPosition', 'GetWindowPosition'),
    'background': ('SetBackground', 'GetBackground'),
    'arc_resolution': ('SetArcResolution', 'GetArcResolution'),
    'disc_material': ('SetDiscMaterial', 'GetDiscMaterial'),
    'render_mode': ('SetRenderMode', 'GetRenderMode'),
    'transparency': ('SetTransparency', 'GetTransparency'),
    'edge_color': ('SetEdgeColor', 'GetEdgeColor'),
    'edge_angle': ('SetEdgeAngle', 'GetEdgeAngle'),
    'antialias': ('SetAntialias', 'GetAntialias'),
    'max_peels': ('SetMaxPeels', 'GetMaxPeels'),
    'interactive_quality': ('SetInteractiveQuality', 'GetInteractiveQuality'),
    'axes': ('SetAxes', 'GetAxes'),
    'labels': ('SetLabels', 'GetLabels'),
    'grid': ('SetGrid', 'GetGrid'),
    'clip': ('SetClip', 'GetClip'),
    'isolate': ('Isolate', 'GetIsolate'),
    'highlight': ('Highlight', 'GetHighlight'),
    'visible': (None, None),   # SetVisible(path, bool) per entry
    'items': (None, None),     # SetItemParameter(path, name, value) per entry
    'legends': ('SetLegendPositions', 'GetLegendPositions'),
    'view': ('SetView', None),
    'mode_2d': ('SetMode2D', 'GetMode2D'),
    'camera': ('SetCamera', 'GetCamera'),
    'animation': ('SetAnimation', 'GetAnimation'),
}


#: the screen (pixels) a relative view size refers to without a real one,
#: e.g. offscreen
REFERENCE_SCREEN = (1920, 1080)


def IsRelativeSize(size):
    """ Whether a view size is ``(fraction, aspect)`` rather than pixels. """
    return float(size[0]) <= 1.0


def ResolveSize(size, screen=None):
    """ ResolveSize(size, screen=None)

    Pixel size ``(w, h)`` of a view size: pixels as given, or ``(fraction,
    aspect)``: a diagonal of `fraction` of the screen diagonal with width /
    height `aspect`. `screen` is the screen size in pixels, default
    :data:`REFERENCE_SCREEN`.
    """
    if not IsRelativeSize(size):
        return int(round(size[0])), int(round(size[1]))
    frac, aspect = float(size[0]), float(size[1])
    h = frac * math.hypot(*(screen or REFERENCE_SCREEN)) / math.hypot(aspect, 1.0)
    return max(1, int(round(aspect * h))), max(1, int(round(h)))


def RelativeSize(pixels, screen):
    """ The ``(fraction, aspect)`` of a pixel size on `screen`. """
    w, h = pixels
    return round(math.hypot(w, h) / math.hypot(*screen), 3), round(w / max(h, 1), 3)


def CheckKeys(state):
    """ Raise ``ValueError`` naming the first unknown key. """
    for key in state:
        if key not in DEFAULTS:
            raise ValueError('unknown view state key "{}" (valid keys: {})'.format(
                key, ', '.join(k for k in DEFAULTS)))
    if 'version' in state and state['version'] != VERSION:
        raise ValueError('unsupported view state version {}, expected {}'.format(state['version'], VERSION))


def ParseColor(color):
    """ ParseColor(color)

    Convert a colour into an RGB tuple of floats in 0..1, plus alpha.

    Parameters
    ----------
    color : str or sequence
        A colour name, ``'#rrggbb'``, or an RGB(A) tuple, either in 0..255
        (if any value is above 1 or all are integers) or as floats in 0..1.

    Returns
    -------
    rgb : tuple of 3 float
    alpha : float or None
        None if no alpha was given.
    """
    if isinstance(color, str):
        from matplotlib import colors
        rgba = colors.to_rgba(color)
        has_alpha = color.startswith('#') and len(color) in (5, 9)
        return tuple(float(c) for c in rgba[:3]), (float(rgba[3]) if has_alpha else None)
    vals = [float(c) for c in color]
    if len(vals) not in (3, 4):
        raise ValueError('invalid colour {!r}'.format(color))
    ints = all(isinstance(c, (numbers.Integral, np.integer)) for c in color)
    if ints or any(v > 1 for v in vals):
        vals = [v / 255.0 for v in vals]
    return tuple(vals[:3]), (vals[3] if len(vals) == 4 else None)


def Luminance(rgb):
    return 0.2126 * rgb[0] + 0.7152 * rgb[1] + 0.0722 * rgb[2]


def ToJSON(obj):
    """ Convert tuples, numpy scalars and arrays into plain JSON types. """
    if isinstance(obj, dict):
        return {str(k): ToJSON(v) for k, v in obj.items()}
    if isinstance(obj, (list, tuple)):
        return [ToJSON(v) for v in obj]
    if isinstance(obj, np.ndarray):
        return ToJSON(obj.tolist())
    if isinstance(obj, np.bool_):
        return bool(obj)
    if isinstance(obj, np.integer):
        return int(obj)
    if isinstance(obj, np.floating):
        return float(obj)
    return obj


def Dumps(state, indent=2):
    return json.dumps(ToJSON(state), indent=indent)


def MergeGrid(current, update):
    """ Merge a partial grid dict into a full one, validating it. """
    grid = copy.deepcopy(current)
    for key, val in update.items():
        if key not in GRID_DEFAULT:
            raise ValueError('unknown grid key "{}" (valid keys: {})'.format(key, ', '.join(GRID_DEFAULT)))
        if key == 'planes':
            for d, pv in val.items():
                if d not in DIRS:
                    d = {0: 'x', 1: 'y', 2: 'z'}.get(d, d)
                if d not in DIRS:
                    raise ValueError('invalid grid plane "{}"'.format(d))
                if isinstance(pv, bool):
                    pv = dict(visible=pv)
                for k in pv:
                    if k not in ('visible', 'index', 'position'):
                        raise ValueError('unknown grid plane key "{}"'.format(k))
                plane = grid['planes'][d]
                if 'index' in pv and pv['index'] is not None:
                    plane['position'] = None
                if 'position' in pv and pv['position'] is not None:
                    plane['index'] = None
                plane.update(pv)
        elif key == 'quality':
            if val not in GRID_QUALITY:
                raise ValueError('invalid grid quality "{}", use None, "size" or "ratio"'.format(val))
            grid[key] = val
        elif key == 'opacity':
            grid[key] = float(np.clip(val, 0, 1))
        else:
            grid[key] = val
    return grid
