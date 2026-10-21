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

"""CSXCAD.Viewer.Describe
Machine-readable summary of a viewer's scene, for scripts and AI agents.
"""

import json

import numpy as np

from CSXCAD.CSProperties import PropertyType
from CSXCAD.Viewer.ViewState import ToJSON

_FLAGS = [n for n in ('MATERIAL', 'METAL', 'EXCITATION', 'PROBEBOX', 'RESBOX', 'DUMPBOX',
                      'DISPERSIVEMATERIAL', 'LORENTZMATERIAL', 'DEBYEMATERIAL', 'DISCRETE_MATERIAL',
                      'LUMPED_ELEMENT', 'CONDUCTINGSHEET', 'ABSORBING_BC')]


def TypeFlags(prop):
    t = prop.GetType()
    return [n for n in _FLAGS if t & int(getattr(PropertyType, n))]


def MaterialValues(prop):
    """ eps, mue, kappa, sigma of a material (isotropic: scalar), or None. """
    if not hasattr(prop, 'GetMaterialProperty'):
        return None
    out = {}
    for name in ('epsilon', 'mue', 'kappa', 'sigma'):
        try:
            v = np.asarray(prop.GetMaterialProperty(name), float)
        except Exception:
            continue
        v = v.ravel()
        out[name] = float(v[0]) if np.all(v == v[0]) else v.tolist()
    return out or None


def _Settings(viewer, item):
    """ {name: {value, type, help, ...}} of the item's view-state settings,
    the keys of the view state's ``items`` entry. """
    params = viewer._CallHook(viewer.GetItemParameters, item) or []
    out = {}
    for p in params:
        if not p.get('state', True):
            continue
        out[p['name']] = {k: p[k] for k in ('value', 'type', 'choices', 'min', 'max', 'default', 'help')
                          if k in p}
    return out


def _Range(b):
    return None if b is None else [float(x) for x in b]


def DescribeDict(viewer):
    sc = viewer._scene
    CSX = viewer.CSX
    props = []
    for e in sc.entries:
        p = e.prop
        prims = p.GetAllPrimitives()
        prios = [pr.GetPriority() for pr in prims]
        d = dict(name=e.name, path=e.path, type=p.GetTypeString(), flags=TypeFlags(p),
                 fill_color=list(p.GetFillColor()), edge_color=list(p.GetEdgeColor()),
                 opacity=p.GetFillColor()[3] / 255.0, visible=bool(p.GetVisibility()),
                 shown=e.IsVisible(), render_mode=p.GetRenderMode(), effective_render_mode=e.GetMode(),
                 primitives=len(prims), primitive_types=[pr.GetTypeName() for pr in prims],
                 bbox=_Range(e.GetBounds()),
                 priority=[min(prios), max(prios)] if prios else None)
        mat = MaterialValues(p)
        if mat:
            d['material'] = mat
        props.append(d)
    out = dict(
        units=dict(delta_unit=CSX.GetGrid().GetDeltaUnit() if CSX is not None else None,
                   note='coordinates are in drawing units; multiply by delta_unit for metres'),
        coord_system='cylindrical' if (CSX is not None and CSX.GetCoordInputType() == 1) else 'cartesian',
        bbox=_Range(sc.VisibleBounds(include_items=False)),
        properties=props,
        grid=sc.grid_layer.Stats(viewer.GetGrid()['quality_threshold']),
        unrendered=list(sc.report),
        tree=viewer.GetTree(),
        # an item's own hooks may fail; that must not break the summary
        items={path: dict(name=it.GetName(), visible=it.GetVisible(),
                          description=viewer._CallHook(it.GetDescription), info=viewer._CallHook(it.GetInfo),
                          settings=_Settings(viewer, it))
               for path, it in sc.items.items()},
        figures={vf.GetPath(): dict(name=vf.GetName(), shown=vf.IsShown(),
                                    curves={c: vf.GetCurveVisible(c) for c in vf.GetCurves()})
                 for vf in viewer.GetFigures().values()},
        camera=viewer.GetCamera(),
        size=list(viewer.GetSize()),
    )
    return ToJSON(out)


def _Fmt(v):
    if isinstance(v, float):
        return '{:.6g}'.format(v)
    if isinstance(v, (list, tuple)):
        return '(' + ', '.join(_Fmt(x) for x in v) + ')'
    return str(v)


def DescribeText(d):
    lines = []
    u = d['units']['delta_unit']
    lines.append('units: delta_unit={} ({})'.format(_Fmt(u), d['coord_system']))
    lines.append('bbox: {}'.format(_Fmt(d['bbox'])))
    lines.append('properties ({}):'.format(len(d['properties'])))
    for p in d['properties']:
        vis = 'shown' if p['shown'] else ('hidden' if not p['visible'] else 'isolated-out')
        s = '  {name} [{type}] {path}: {n} prim ({prims}), {vis}, {mode}, opacity {op:.2f}, color #{c}, bbox {bb}'.format(
            name=p['name'], type=p['type'], path=p['path'], prims=', '.join('{} {}'.format(i, t) for i, t in enumerate(p['primitive_types'])), n=p['primitives'], vis=vis, mode=p['effective_render_mode'],
            op=p['opacity'], c=''.join('{:02x}'.format(int(x)) for x in p['fill_color'][:3]),
            bb=_Fmt(p['bbox']))
        if 'material' in p:
            s += ', ' + ', '.join('{}={}'.format(k, _Fmt(v)) for k, v in p['material'].items())
        lines.append(s)
    g = d['grid']
    if g:
        lines.append('grid ({}):'.format(g['coord_system']))
        for name, info in g['directions'].items():
            s = '  {}: {} lines, range {}'.format(name, info['lines'], _Fmt(info['range']))
            if 'min_cell' in info:
                s += ', cell {} @{} .. {} @{}, max ratio {} @{} ({} > threshold)'.format(
                    _Fmt(info['min_cell']), _Fmt(info['min_cell_at']), _Fmt(info['max_cell']),
                    _Fmt(info['max_cell_at']), _Fmt(info['max_ratio']), _Fmt(info.get('max_ratio_at', '-')),
                    info['n_over_threshold'])
            lines.append(s)
        lines.append('  tree paths: grid, grid/x, grid/y, grid/z, grid/outline, grid/quality')
    else:
        lines.append('grid: none')
    if d['unrendered']:
        lines.append('unrendered primitives:')
        for r in d['unrendered']:
            lines.append('  {prop}: {type} #{primitive_id}: {reason}'.format(**r))
    if d['items']:
        lines.append('items:')
        for path, it in d['items'].items():
            lines.append('  {} ({}){}'.format(path, 'shown' if it['visible'] else 'hidden',
                                              ': ' + it['description'] if it['description'] else ''))
            if it['settings']:
                lines.append('    settings: ' + ', '.join('{}={}'.format(k, json.dumps(ToJSON(s.get('value'))))
                                                          for k, s in it['settings'].items()))
    if d['figures']:
        lines.append('figures:')
        for path, f in d['figures'].items():
            lines.append('  {} ({}): {}'.format(path, 'shown' if f['shown'] else 'hidden', ', '.join(
                c if vis else c + ' (hidden)' for c, vis in f['curves'].items())))
    c = d['camera']
    lines.append('camera: position {} focal_point {} view_up {}{}'.format(
        _Fmt(c['position']), _Fmt(c['focal_point']), _Fmt(c['view_up']),
        ' parallel' if c['parallel_projection'] else ''))
    lines.append('size: {}x{}'.format(*d['size']))
    return '\n'.join(lines)


def Describe(viewer, fmt='dict'):
    """ Describe(viewer, fmt='dict')

    See :meth:`CSXCAD.Viewer.Viewer.Describe`.
    """
    d = DescribeDict(viewer)
    if fmt == 'dict':
        return d
    if fmt == 'text':
        return DescribeText(d)
    raise ValueError('fmt must be "dict" or "text"')
