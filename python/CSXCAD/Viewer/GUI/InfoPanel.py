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

"""CSXCAD.Viewer.GUI.InfoPanel
Read-only details of the selected property, primitive, grid or item.
"""

import html
import math

import numpy as np

from PySide6.QtWidgets import QTextBrowser

from CSXCAD.Viewer.Describe import TypeFlags

# (label, getter, args) shown when the object has the getter
_PROPERTY_GETTERS = [
    ('Isotropy', 'GetIsotropy', ()),
    ('Dispersion order', 'GetDispersionOrder', ()),
    ('Excitation type', 'GetExcitType', ()),
    ('Excitation', 'GetExcitation', ()),
    ('Enabled', 'GetEnabled', ()),
    ('Propagation dir', 'GetPropagationDir', ()),
    ('Weight function', 'GetWeightFunction', ()),
    ('Weight file', 'GetWeightFile', ()),
    ('Frequency', 'GetFrequency', ()),
    ('Delay', 'GetDelay', ()),
    ('Probe type', 'GetProbeType', ()),
    ('Weighting', 'GetWeighting', ()),
    ('Normal dir', 'GetNormalDir', ()),
    ('Mode function', 'GetModeFunction', ()),
    ('Mode file', 'GetModeFile', ()),
    ('Dump type', 'GetDumpType', ()),
    ('Dump mode', 'GetDumpMode', ()),
    ('File type', 'GetFileType', ()),
    ('Sub-sampling', 'GetSubSampling', ()),
    ('Opt. resolution', 'GetOptResolution', ()),
    ('Over-sampling', 'GetOverSampling', ()),
    ('Start time', 'GetStartTime', ()),
    ('Stop time', 'GetStopTime', ()),
    ('Multi grid level', 'GetMultiGridLevel', ()),
    ('Resistance', 'GetResistance', ()),
    ('Capacity', 'GetCapacity', ()),
    ('Inductance', 'GetInductance', ()),
    ('Direction', 'GetDirection', ()),
    ('Caps', 'GetCaps', ()),
    ('LE type', 'GetLEtype', ()),
    ('Conductivity', 'GetConductivity', ()),
    ('Thickness', 'GetThickness', ()),
    ('File', 'GetFilename', ()),
    ('Scale', 'GetScale', ()),
    ('Use DB for background', 'GetUseDataBaseForBackground', ()),
    ('Absorbing BC type', 'GetAbsorbingBoundaryType', ()),
    ('Phase velocity', 'GetPhaseVelocity', ()),
    ('Normal sign positive', 'GetNormalSignPositive', ()),
]

_PRIMITIVE_GETTERS = [
    ('Coordinate', 'GetCoord'), ('Start', 'GetStart'), ('Stop', 'GetStop'), ('Center', 'GetCenter'),
    ('Radius', 'GetRadius'), ('Shell width', 'GetShellWidth'), ('Normal dir', 'GetNormDir'),
    ('Elevation', 'GetElevation'), ('Length', 'GetLength'), ('Rotation axis', 'GetRotAxisDir'),
    ('Angle', 'GetAngle'), ('Wire radius', 'GetWireRadius'), ('Points', 'GetNumberOfPoints'),
    ('Vertices', 'GetNumVertices'), ('Faces', 'GetNumFaces'), ('File', 'GetFilename'),
    ('Boxes', 'GetQtyBoxes'), ('Polygon points', 'GetQtyCoords'),
]


def _Fmt(v):
    if isinstance(v, bytes):
        v = v.decode('utf-8', 'replace')
    if isinstance(v, (list, tuple, np.ndarray)):
        a = np.asarray(v)
        if a.dtype.kind in 'fiu':
            if a.size > 12:
                return '{} values: {} ... {}'.format(a.size, _Fmt(a.ravel()[:3].tolist()), _Fmt(a.ravel()[-1]))
            return '(' + ', '.join(_Fmt(x) for x in a.ravel().tolist()) + ')'
        return ', '.join(_Fmt(x) for x in v)
    if isinstance(v, float):
        return '{:.6g}'.format(v)
    return str(v)


def _TransformArgs(name, args):
    """ Transform arguments for display: rotation angles (stored in radians) in degrees. """
    if not name.startswith('Rotate') or not args:
        return args
    try:
        angle = '{:.6g}°'.format(math.degrees(float(args[-1])))
    except ValueError:
        angle = args[-1] + ' rad'   # a parameter expression
    return args[:-1] + [angle]


def _Row(label, value):
    return '<tr><td><b>{}</b></td><td>{}</td></tr>'.format(html.escape(label), html.escape(_Fmt(value)))


class InfoPanel(QTextBrowser):
    """ InfoPanel(main_window) """

    def __init__(self, win):
        super().__init__(win)
        self.win = win
        self.setOpenLinks(False)

    def _Show(self, title, rows, item=None):
        self.setHtml('<h3>{}</h3><table cellspacing="2">{}</table>'.format(html.escape(title), ''.join(rows)))
        editor = getattr(self.win, 'item_editor', None)
        if editor is not None and editor.item is not item:
            editor.SetItem(item)

    def clear(self):
        super().clear()
        editor = getattr(self.win, 'item_editor', None)
        if editor is not None:
            editor.SetItem(None)

    def ShowProperty(self, prop):
        v = self.win.viewer
        e = v._scene.Entry(prop)
        rows = [_Row('Type', prop.GetTypeString()), _Row('Flags', ', '.join(TypeFlags(prop))),
                _Row('ID', prop.GetID()), _Row('Fill colour', prop.GetFillColor()),
                _Row('Edge colour', prop.GetEdgeColor()), _Row('Visible', bool(prop.GetVisibility())),
                _Row('Render mode', prop.GetRenderMode() or 'default ({})'.format(e.GetMode())),
                _Row('Primitives', prop.GetQtyPrimitives()), _Row('Bounding box', e.GetBounds() or '-')]
        if hasattr(prop, 'GetMaterialProperty'):
            for name in ('epsilon', 'mue', 'kappa', 'sigma', 'density'):
                try:
                    val = prop.GetMaterialProperty(name)
                    rows.append(_Row(name, val))
                    w = prop.GetMaterialWeight(name)
                    if any(x for x in np.atleast_1d(w) if x not in ('', '1', b'', b'1')):
                        rows.append(_Row(name + ' weight', w))
                except Exception:
                    pass
        if hasattr(prop, 'GetDispersionOrder'):
            try:
                for order in range(prop.GetDispersionOrder()):
                    for name in ('eps_plasma', 'eps_pole_freq', 'eps_relax', 'mue_plasma', 'mue_pole_freq',
                                 'mue_relax', 'eps_delta'):
                        try:
                            rows.append(_Row('{} [{}]'.format(name, order),
                                             prop.GetDispersiveMaterialProperty(name, order)))
                        except Exception:
                            pass
            except Exception:
                pass
        for label, getter, args in _PROPERTY_GETTERS:
            fn = getattr(prop, getter, None)
            if fn is None:
                continue
            try:
                rows.append(_Row(label, fn(*args)))
            except Exception:
                pass
        attrs = prop.GetAttributes()
        if attrs:
            rows.append(_Row('Attributes', ', '.join('{}={}'.format(k, a) for k, a in attrs.items())))
        self._Show(prop.GetName(), rows)

    def ShowPrimitive(self, prim):
        rows = [_Row('Type', prim.GetTypeName()), _Row('ID', prim.GetID()), _Row('Priority', prim.GetPriority())]
        cs = prim.GetCoordinateSystem()
        rows.append(_Row('Coordinate system', {None: 'structure default', 0: 'Cartesian', 1: 'cylindrical'}.get(cs, cs)))
        for label, getter in _PRIMITIVE_GETTERS:
            fn = getattr(prim, getter, None)
            if fn is None:
                continue
            try:
                rows.append(_Row(label, fn()))
            except Exception:
                pass
        if hasattr(prim, 'GetCoords'):
            try:
                x0, x1 = prim.GetCoords()
                rows.append(_Row('x0', x0))
                rows.append(_Row('x1', x1))
            except Exception:
                pass
        if prim.HasTransform():
            # the individual steps in order, as in the XML
            steps = ['{}({})'.format(name, ', '.join(_TransformArgs(name, args)))
                     for name, args in prim.GetTransform().GetTransformList()]
            rows.append('<tr><td><b>Transform</b></td><td>{}</td></tr>'.format(
                '<br>'.join(html.escape(t) for t in steps)))
        try:
            rows.append(_Row('Bounding box', np.asarray(prim.GetBoundBox()).ravel()))
        except Exception:
            pass
        self._Show('Primitive', rows)

    def ShowGrid(self):
        v = self.win.viewer
        stats = v._scene.grid_layer.Stats(v.GetGrid()['quality_threshold'])
        if stats is None:
            self._Show('Grid', [_Row('Grid', 'none')])
            return
        rows = [_Row('Coordinate system', stats['coord_system'])]
        if v.CSX is not None:
            rows.append(_Row('Delta unit', v.CSX.GetGrid().GetDeltaUnit()))
        for name, info in stats['directions'].items():
            rows.append(_Row(name + ' lines', info['lines']))
            rows.append(_Row(name + ' range', info['range']))
            if 'min_cell' in info:
                rows.append(_Row(name + ' cell min/max', (info['min_cell'], info['max_cell'])))
                rows.append(_Row(name + ' worst ratio', '{:.3g} at {:.6g} ({} over threshold)'.format(
                    info['max_ratio'], info.get('max_ratio_at', float('nan')), info['n_over_threshold'])))
        self._Show('Grid', rows)

    def ShowItem(self, item):
        """ Info of a ViewItem; its settings go to the Settings dock. """
        rows = [_Row('Path', item.GetPath()), _Row('Visible', item.GetVisible()),
                _Row('Props', len(item.GetProps()))]
        try:
            for k, val in (item.GetInfo() or {}).items():
                rows.append(_Row(str(k), val))
        except Exception as err:
            rows.append(_Row('GetInfo failed', err))
        self._Show(item.GetName(), rows, item)
