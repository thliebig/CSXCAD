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

"""CSXCAD.Viewer.Scene
Scene content of the viewer: property actors and their render modes, the
mesh grid, clipping, labels, the highlight overlay, the scene tree and the
:class:`ViewItem` extension hook.

All of it is window-free; :class:`CSXCAD.Viewer.Viewer` drives it.
"""

import math

import numpy as np

from vtkmodules.vtkCommonCore import vtkIdList, vtkLookupTable, vtkPoints
from vtkmodules.vtkCommonDataModel import vtkCellArray, vtkPlane, vtkPolyData
from vtkmodules.vtkFiltersCore import vtkFeatureEdges, vtkPolyDataNormals
try:
    from vtkmodules.vtkFiltersCore import vtkExtractCells
except ImportError:   # VTK < 9.3
    from vtkmodules.vtkFiltersExtraction import vtkExtractCells
from vtkmodules.vtkFiltersGeometry import vtkGeometryFilter
from vtkmodules.vtkRenderingCore import (vtkMapper, vtkCoordinate, vtkActor, vtkActor2D, vtkPolyDataMapper, vtkPolyDataMapper2D,
                                         vtkProp3D, vtkTextActor)
from vtkmodules.vtkRenderingAnnotation import vtkScalarBarActor
from vtkmodules.util.numpy_support import vtk_to_numpy, numpy_to_vtk
# The rendering backends register their object factories on import: a
# vtkActor or scalar bar created before they are loaded stays a generic one
# that the OpenGL renderer does not draw. Items (ViewItem subclasses, e.g.
# from another package) may build their actors before a Viewer exists.
import vtkmodules.vtkRenderingOpenGL2  # noqa: F401
import vtkmodules.vtkRenderingFreeType  # noqa: F401

from CSXCAD.CSProperties import PropertyType
from CSXCAD.Viewer import Geometry
from CSXCAD.Viewer.ViewState import ParseColor, Luminance

PROPERTY_GROUPS = (
    ('materials', 'Materials'),
    ('ports', 'Ports & Probes'),
    ('dumps', 'Field recordings'),
    ('other', 'Other'),
)

HIGHLIGHT_COLOR = (1.0, 0.0, 1.0)
# Coplanar faces of different properties (a sheet on a substrate) have equal
# depth and flicker (z-fighting). Polygon offsets break the tie: each
# property is pulled towards the camera by its rank in primitive priority,
# so the higher priority wins as in the simulation. Offsets in depth units.
DEPTH_FACTOR = -1.0
DEPTH_UNITS = -4.0
vtkMapper.SetResolveCoincidentTopologyToPolygonOffset()
DIM_OPACITY = 0.15


def PropertyGroup(prop):
    """ Scene-tree group of a property: materials, ports, dumps or other. """
    t = prop.GetType()
    if t & PropertyType.DUMPBOX:
        return 'dumps'
    if t & (PropertyType.EXCITATION | PropertyType.PROBEBOX | PropertyType.LUMPED_ELEMENT | PropertyType.RESBOX):
        return 'ports'
    if t & (PropertyType.MATERIAL | PropertyType.METAL | PropertyType.CONDUCTINGSHEET |
            PropertyType.DISCRETE_MATERIAL | PropertyType.DISPERSIVEMATERIAL):
        return 'materials'
    return 'other'


def EscapeName(name):
    return name.replace('\\', '\\\\').replace('/', '\\/')


def SplitPath(path):
    """ Split a tree path at unescaped ``/``. """
    parts, cur, i = [], '', 0
    while i < len(path):
        c = path[i]
        if c == '\\' and i + 1 < len(path):
            cur += path[i:i + 2]
            i += 2
            continue
        if c == '/':
            parts.append(cur)
            cur = ''
        else:
            cur += c
        i += 1
    parts.append(cur)
    return parts


def JoinPath(*parts):
    return '/'.join(p for p in parts if p)


def UnionBounds(bounds):
    """ Union of bounds ``(xmin, xmax, ymin, ymax, zmin, zmax)``, None
    entries skipped; None if there are none. """
    bounds = [b for b in bounds if b is not None]
    if not bounds:
        return None
    b = np.asarray(bounds, float)
    return tuple(float(x) for x in (b[:, 0].min(), b[:, 1].max(), b[:, 2].min(),
                                    b[:, 3].max(), b[:, 4].min(), b[:, 5].max()))


def _Opacity(alpha, cap):
    o = alpha / 255.0
    if cap is not None:
        o = min(o, cap)
    return o


def _SplitPolyData(pd):
    """ Split into (polys, lines+verts) polydata, sharing points, with cell data. """
    nv, nl, npoly = pd.GetNumberOfVerts(), pd.GetNumberOfLines(), pd.GetNumberOfPolys()
    surf = wire = None
    if npoly:
        surf = vtkPolyData()
        surf.SetPoints(pd.GetPoints())
        surf.SetPolys(pd.GetPolys())
        _SliceCellData(pd, surf, nv + nl, nv + nl + npoly)
    if nv or nl:
        wire = vtkPolyData()
        wire.SetPoints(pd.GetPoints())
        if nv:
            wire.SetVerts(pd.GetVerts())
        if nl:
            wire.SetLines(pd.GetLines())
        _SliceCellData(pd, wire, 0, nv + nl)
    return surf, wire


def _SliceCellData(src, dst, start, stop):
    cd = src.GetCellData()
    for i in range(cd.GetNumberOfArrays()):
        arr = cd.GetArray(i)
        data = vtk_to_numpy(arr)[start:stop].copy()
        out = numpy_to_vtk(data, deep=1)
        out.SetName(arr.GetName())
        dst.GetCellData().AddArray(out)


def ExtractCells(pd, cells):
    """ The given cells of a polydata, as new polydata. """
    ids = vtkIdList()
    for c in cells:
        ids.InsertNextId(int(c))
    ex = vtkExtractCells()
    ex.SetInputData(pd)
    ex.SetCellList(ids)
    geo = vtkGeometryFilter()
    geo.SetInputConnection(ex.GetOutputPort())
    geo.Update()
    out = vtkPolyData()
    out.ShallowCopy(geo.GetOutput())
    return out


def _Signature(prop):
    """ Cheap fingerprint of the geometry of a property. """
    sig = []
    for prim in prop.GetAllPrimitives():
        try:
            bb = tuple(np.round(np.asarray(prim.GetBoundBox(), float).ravel(), 12))
        except Exception:
            bb = ()
        tr = tuple(np.asarray(prim.GetTransform().GetMatrix()).ravel()) if prim.HasTransform() else ()
        sig.append((prim.GetID(), int(prim.GetType()), bb, tr))
    return tuple(sig)


###############################################################################
class PropertyEntry:
    """ The actors of one property. """

    def __init__(self, scene, prop, path):
        self.scene = scene
        self.prop = prop
        self.path = path
        self.name = prop.GetName()
        self.raw = None
        self.surf = None
        self.wire = None
        self.report = []
        self.signature = None
        self.surf_actor = None
        self.wire_actor = None
        self.depth_rank = 0
        self.edge_actor = None
        self._edges = None
        self.disc_actor = None
        self.disc_pd = None
        self._label_pts = None
        self.label = None

    # -- geometry
    def Build(self, default_cs, arc_resolution, disc_material):
        self.RemoveActors()
        self.signature = _Signature(self.prop)
        self.raw, self.report = Geometry.PropertyPolyData(self.prop, default_cs, arc_resolution)
        self.surf = self.wire = None
        self._label_pts = None
        if self.raw is not None:
            self.surf, self.wire = _SplitPolyData(self.raw)
        self.disc_pd = Geometry.DiscMaterialPolyData(self.prop) if disc_material else None
        ren = self.scene.renderer
        if self.surf is not None:
            norm = vtkPolyDataNormals()
            norm.SetInputData(self.surf)
            norm.SplittingOn()
            norm.SetFeatureAngle(30)
            norm.ConsistencyOff()
            norm.ComputeCellNormalsOff()
            self.surf_actor = self._MakeActor(norm)
        if self.wire is not None:
            self.wire_actor = self._MakeActor(self.wire)
            p = self.wire_actor.GetProperty()
            p.SetLineWidth(2)
            p.SetPointSize(8)
            p.SetRenderPointsAsSpheres(True)
        if self.disc_pd is not None:
            norm = vtkPolyDataNormals()
            norm.SetInputData(self.disc_pd)
            norm.SplittingOn()
            self.disc_actor = self._MakeActor(norm)
        for a in self.Actors():
            ren.AddActor(a)

    def Priority(self):
        """ Highest primitive priority of the property (0 without primitives). """
        prims = self.prop.GetAllPrimitives()
        return max((p.GetPriority() for p in prims), default=0)

    def SetDepthRank(self, rank):
        """ Pull the actors `rank` steps towards the camera, see DEPTH_UNITS. """
        self.depth_rank = rank
        for a in (self.surf_actor, self.wire_actor, self.disc_actor, self.edge_actor):
            if a is None:
                continue
            m = a.GetMapper()
            m.SetRelativeCoincidentTopologyPolygonOffsetParameters(rank * DEPTH_FACTOR, rank * DEPTH_UNITS)
            m.SetRelativeCoincidentTopologyLineOffsetParameters(0, rank * DEPTH_UNITS)
            m.SetRelativeCoincidentTopologyPointOffsetParameter(rank * DEPTH_UNITS)

    def _MakeActor(self, src):
        mapper = vtkPolyDataMapper()
        if isinstance(src, vtkPolyData):
            mapper.SetInputData(src)
        else:
            mapper.SetInputConnection(src.GetOutputPort())
        mapper.ScalarVisibilityOff()
        if self.scene.clip_plane is not None:
            mapper.AddClippingPlane(self.scene.clip_plane)
        actor = vtkActor()
        actor.SetMapper(mapper)
        return actor

    def EdgeActor(self):
        """ Feature-edge actor, created on first use. """
        if self.edge_actor is None and self.surf is not None:
            fe = vtkFeatureEdges()
            fe.SetInputData(self.surf)
            fe.BoundaryEdgesOn()
            fe.FeatureEdgesOn()
            fe.NonManifoldEdgesOn()
            fe.ManifoldEdgesOff()
            fe.ColoringOff()
            fe.SetFeatureAngle(self.scene.edge_angle)
            self._edges = fe
            self.edge_actor = self._MakeActor(fe)
            self.edge_actor.GetProperty().SetLineWidth(1.5)
            self.edge_actor.GetProperty().LightingOff()
            self.SetDepthRank(self.depth_rank)
            self.scene.renderer.AddActor(self.edge_actor)
        return self.edge_actor

    def SetEdgeAngle(self, angle):
        if self.edge_actor is not None:
            self._edges.SetFeatureAngle(angle)

    def Actors(self):
        return [a for a in (self.surf_actor, self.wire_actor, self.edge_actor, self.disc_actor) if a is not None]

    def RemoveActors(self):
        for a in self.Actors():
            self.scene.renderer.RemoveActor(a)
        if self.label is not None:
            self.scene.renderer.RemoveViewProp(self.label)
        self.surf_actor = self.wire_actor = self.edge_actor = self.disc_actor = self.label = None

    # -- appearance
    def GetMode(self):
        mode = self.prop.GetRenderMode()
        if mode is None:
            if self.disc_actor is not None:
                return 'edges'  # show the material model, mark its region only
            mode = self.scene.render_mode
        return mode

    def IsVisible(self):
        if not self.prop.GetVisibility():
            return False
        iso = self.scene.isolate
        return iso is None or self.name in iso

    def Apply(self):
        sc = self.scene
        vis = self.IsVisible()
        mode = self.GetMode()
        fill = self.prop.GetFillColor()
        edge = self.prop.GetEdgeColor()
        cap = sc.transparency
        if sc.dim_others and not sc.IsHighlighted(self.name):
            cap = DIM_OPACITY if cap is None else min(cap, DIM_OPACITY)
        fill_rgb = [c / 255.0 for c in fill[:3]]
        fill_op = _Opacity(fill[3], cap)
        if self.surf_actor is not None:
            self.surf_actor.SetVisibility(vis and mode in ('solid', 'solid+edges'))
            p = self.surf_actor.GetProperty()
            p.SetColor(fill_rgb)
            p.SetOpacity(fill_op)
        if self.wire_actor is not None:
            self.wire_actor.SetVisibility(vis)
            p = self.wire_actor.GetProperty()
            p.SetColor(fill_rgb)
            p.SetOpacity(fill_op)
        if self.disc_actor is not None:
            self.disc_actor.SetVisibility(vis and sc.disc_material)
            p = self.disc_actor.GetProperty()
            p.SetColor(fill_rgb)
            p.SetOpacity(fill_op)
        want_edges = vis and mode in ('edges', 'solid+edges')
        if want_edges or self.edge_actor is not None:
            ea = self.EdgeActor()
            if ea is not None:
                ea.SetVisibility(want_edges)
                p = ea.GetProperty()
                if sc.edge_color is not None:
                    rgb, a = sc.edge_color
                    p.SetColor(rgb)
                    p.SetOpacity(_Opacity(255 * (1.0 if a is None else a), cap))
                else:
                    # without faces, the edges carry the property colour
                    rgb = edge[:3] if mode == 'solid+edges' else fill[:3]
                    p.SetColor([c / 255.0 for c in rgb])
                    p.SetOpacity(_Opacity(edge[3], cap))
        if self.label is not None:
            self.label.SetVisibility(vis and sc.labels)

    def GetBounds(self):
        """ Bounds (xmin, xmax, ymin, ymax, zmin, zmax) of the geometry, or None. """
        return UnionBounds(pd.GetBounds() for pd in (self.raw, self.disc_pd)
                           if pd is not None and pd.GetNumberOfPoints())

    def LabelCandidates(self):
        """ Surface points a label may attach to: (N, 3) array, at most a few thousand. """
        if self._label_pts is None:
            parts = [vtk_to_numpy(pd.GetPoints().GetData()) for pd in (self.raw, self.disc_pd)
                     if pd is not None and pd.GetNumberOfPoints()]
            pts = np.concatenate(parts) if parts else np.zeros((0, 3))
            # a strided subset still lies on the surface and keeps placement cheap
            self._label_pts = pts[::max(1, len(pts) // 4096)].astype(float)
        return self._label_pts

    def CellsOf(self, primitive_id=None):
        """ (pd, cell ids) of the cells belonging to a primitive (all if None). """
        if self.raw is None:
            return None, None
        if primitive_id is None:
            return self.raw, np.arange(self.raw.GetNumberOfCells())
        ids = vtk_to_numpy(self.raw.GetCellData().GetArray('primitive_id'))
        return self.raw, np.nonzero(ids == primitive_id)[0]


class _GridLines:
    """ Polylines of the grid display, each with its (ratio, size) quality
    as cell arrays, see GridLayer.LineQuality. """

    def __init__(self):
        self.pts, self.cells, self.quality = [], [], []
        self.n = 0

    def Add(self, P, quality=(1.0, 0.0)):
        P = np.asarray(P, float).reshape(-1, 3)
        self.pts.append(P)
        self.cells.append(list(range(self.n, self.n + len(P))))
        self.n += len(P)
        self.quality.append(quality)

    def ToPolyData(self):
        pd = Geometry.PolyDataFromArrays(np.concatenate(self.pts), lines=self.cells)
        for name, data in zip(('ratio', 'size'), np.asarray(self.quality, float).T):
            arr = numpy_to_vtk(np.ascontiguousarray(data), deep=1)
            arr.SetName(name)
            pd.GetCellData().AddArray(arr)
        return pd


###############################################################################
class GridLayer:
    """ Mesh-grid planes, domain outline and quality overlay. """

    def __init__(self, scene):
        self.scene = scene
        self.grid = None
        self.lines = None
        self.cylindrical = False
        self.plane_actors = [None, None, None]
        self.outline_actor = None
        self.scalar_bar = None
        self.info = {}

    def SetGrid(self, grid):
        self.grid = grid
        self.lines = None
        if grid is not None:
            lines = [np.asarray(grid.GetLines(n), float) for n in range(3)]
            if all(len(l) > 0 for l in lines):
                self.lines = lines
            self.cylindrical = grid.GetMeshType() == 1

    def Clear(self):
        for a in self.plane_actors + [self.outline_actor, self.scalar_bar]:
            if a is not None:
                self.scene.renderer.RemoveActor(a)
        self.plane_actors = [None, None, None]
        self.outline_actor = None
        self.scalar_bar = None

    # -- helpers
    def ResolveIndex(self, n, plane):
        """ Line index for a plane spec, clamped; returns (index, clamped). """
        N = len(self.lines[n])
        if plane.get('position') is not None:
            pos = plane['position']
            idx = int(np.argmin(np.abs(self.lines[n] - pos)))
            return idx, bool(pos < self.lines[n][0] or pos > self.lines[n][-1])
        idx = plane.get('index')
        idx = 0 if idx is None else int(idx)
        if idx < 0:
            idx += N
        clamped = idx < 0 or idx >= N
        return int(np.clip(idx, 0, N - 1)), clamped

    def _Arc(self, r, a0, a1, z, n_min=2):
        n = Geometry._NumSteps(a1 - a0, 64, math.radians(3)) + 1
        a = np.linspace(a0, a1, max(n, n_min))
        return Geometry._Cyl2Cart(r, a, z)

    def _PlanePolyData(self, n, idx):
        """ Lines of one grid plane, with per-line arrays 'ratio' and 'size'. """
        L = self.lines
        out = _GridLines()

        def add(P, d, i):
            out.Add(P, self.LineQuality(d, i))

        val = L[n][idx]
        if not self.cylindrical:
            a, b = (n + 1) % 3, (n + 2) % 3
            for d, other in ((a, b), (b, a)):
                for i, v in enumerate(L[d]):
                    P = np.zeros((2, 3))
                    P[:, n] = val
                    P[:, d] = v
                    P[:, other] = (L[other][0], L[other][-1])
                    add(P, d, i)
        else:
            r, al, z = L
            if n == 2:   # z plane: r arcs and alpha spokes
                for i, rv in enumerate(r):
                    add(self._Arc(rv, al[0], al[-1], val), 0, i)
                for i, av in enumerate(al):
                    add(Geometry._Cyl2Cart([r[0], r[-1]], av, val), 1, i)
            elif n == 1:  # alpha half plane
                for i, rv in enumerate(r):
                    add(Geometry._Cyl2Cart(rv, val, [z[0], z[-1]]), 0, i)
                for i, zv in enumerate(z):
                    add(Geometry._Cyl2Cart([r[0], r[-1]], val, zv), 2, i)
            else:         # r cylinder
                for i, av in enumerate(al):
                    add(Geometry._Cyl2Cart(val, av, [z[0], z[-1]]), 1, i)
                for i, zv in enumerate(z):
                    add(self._Arc(val, al[0], al[-1], zv), 2, i)
        return out.ToPolyData()

    def LineQuality(self, d, i):
        """ (neighbour ratio, local cell size) at line i of direction d. """
        lines = self.lines[d]
        if len(lines) < 2:
            return 1.0, 0.0
        delta = np.diff(lines)
        left = delta[i - 1] if i > 0 else None
        right = delta[i] if i < len(delta) else None
        if left is None or right is None:
            return 1.0, float(left if right is None else right)
        lo, hi = sorted((left, right))
        return (float(hi / lo) if lo > 0 else float('inf')), float(lo)

    def Stats(self, threshold=1.5):
        """ Grid statistics per direction, see Describe(). """
        if self.lines is None:
            return None
        names = ('r', 'a', 'z') if self.cylindrical else ('x', 'y', 'z')
        out = dict(coord_system='cylindrical' if self.cylindrical else 'cartesian', directions={})
        for d in range(3):
            L = self.lines[d]
            info = dict(name=names[d], lines=len(L), range=(float(L[0]), float(L[-1])))
            if len(L) > 1:
                delta = np.diff(L)
                info['min_cell'] = float(delta.min())
                info['min_cell_at'] = float(L[int(np.argmin(delta))])
                info['max_cell'] = float(delta.max())
                info['max_cell_at'] = float(L[int(np.argmax(delta))])
                if len(delta) > 1:
                    r = np.maximum(delta[1:] / delta[:-1], delta[:-1] / delta[1:])
                    k = int(np.argmax(r))
                    info['max_ratio'] = float(r[k])
                    info['max_ratio_at'] = float(L[k + 1])
                    info['n_over_threshold'] = int(np.count_nonzero(r > threshold))
                else:
                    info['max_ratio'] = 1.0
                    info['n_over_threshold'] = 0
            out['directions'][names[d]] = info
        return out

    def _Mapper(self, pd, gs):
        m = vtkPolyDataMapper()
        m.SetInputData(pd)
        q = gs['quality']
        rgb, _ = ParseColor(gs['color'])
        if q is None:
            m.ScalarVisibilityOff()
            return m
        lut = vtkLookupTable()
        if q == 'ratio':
            thr = gs['quality_threshold']
            lut.SetNumberOfTableValues(2)
            lut.SetTableValue(0, rgb[0], rgb[1], rgb[2], 1)
            lut.SetTableValue(1, 1, 0, 0, 1)
            lut.Build()
            m.SetScalarRange(thr - 1e-9, thr + 1e-9)
            m.SelectColorArray('ratio')
        else:
            lut.SetHueRange(0.667, 0.0)
            lut.Build()
            sizes = np.concatenate([np.diff(l) for l in self.lines if len(l) > 1])
            m.SetScalarRange(float(sizes.min()), float(sizes.max()))
            m.SelectColorArray('size')
        m.SetLookupTable(lut)
        m.SetScalarModeToUseCellFieldData()
        m.ScalarVisibilityOn()
        return m

    def Rebuild(self, gs):
        """ Rebuild all grid actors from the grid state `gs`. Returns plane info. """
        self.Clear()
        self.info = {}
        if self.lines is None or not gs['visible']:
            return self.info
        ren = self.scene.renderer
        rgb, _ = ParseColor(gs['color'])
        q = gs['quality']
        for n, d in enumerate(('x', 'y', 'z')):
            plane = gs['planes'][d]
            idx, clamped = self.ResolveIndex(n, plane)
            self.info[d] = (idx, float(self.lines[n][idx]), len(self.lines[n]), clamped)
            if not plane['visible']:
                continue
            actor = vtkActor()
            actor.SetMapper(self._Mapper(self._PlanePolyData(n, idx), gs))
            p = actor.GetProperty()
            p.SetColor(rgb)
            p.SetOpacity(1.0 if q is not None else gs['opacity'])
            p.LightingOff()
            actor.PickableOff()
            ren.AddActor(actor)
            self.plane_actors[n] = actor
        if gs['outline']:
            actor = vtkActor()
            actor.SetMapper(self._Mapper(self._OutlinePolyData(), gs))
            actor.GetProperty().SetColor(rgb)
            actor.GetProperty().LightingOff()
            actor.PickableOff()
            ren.AddActor(actor)
            self.outline_actor = actor
        if q == 'size':
            sb = vtkScalarBarActor()
            any_mapper = next((a.GetMapper() for a in self.plane_actors + [self.outline_actor] if a is not None), None)
            if any_mapper is not None:
                sb.SetLookupTable(any_mapper.GetLookupTable())
                sb.SetTitle('cell size')
                sb.SetNumberOfLabels(4)
                sb.SetWidth(0.08)
                sb.SetHeight(0.4)
                sb.SetPosition(0.9, 0.05)
                tc = self.scene.TextColor()
                sb.GetTitleTextProperty().SetColor(tc)
                sb.GetLabelTextProperty().SetColor(tc)
                ren.AddViewProp(sb)
                self.scalar_bar = sb
        return self.info

    def _OutlinePolyData(self):
        """ Domain outline with a tick at every mesh line along its edges. """
        L = self.lines
        lo = np.array([l[0] for l in L])
        hi = np.array([l[-1] for l in L])
        out = _GridLines()
        add = out.Add

        if not self.cylindrical:
            diag = np.linalg.norm(hi - lo)
            tick = 0.015 * diag
            for n in range(3):
                a, b = (n + 1) % 3, (n + 2) % 3
                for va in (lo[a], hi[a]):
                    for vb in (lo[b], hi[b]):
                        P = np.zeros((2, 3))
                        P[:, n] = (lo[n], hi[n])
                        P[:, a] = va
                        P[:, b] = vb
                        add(P)
                        sa = -1 if va == lo[a] else 1
                        sb = -1 if vb == lo[b] else 1
                        for i, v in enumerate(L[n]):
                            p0 = np.zeros(3)
                            p0[n], p0[a], p0[b] = v, va, vb
                            t = np.zeros(3)
                            t[a] = sa * tick
                            t2 = np.zeros(3)
                            t2[b] = sb * tick
                            q = self.LineQuality(n, i)
                            add([p0, p0 + t], q)
                            add([p0, p0 + t2], q)
        else:
            r, al, z = L
            diag = 2 * r[-1] + (z[-1] - z[0])
            tick = 0.015 * diag
            for zv in (z[0], z[-1]):
                for rv in (r[0], r[-1]):
                    if rv > 0:
                        add(self._Arc(rv, al[0], al[-1], zv))
                for av in (al[0], al[-1]):
                    add(Geometry._Cyl2Cart([r[0], r[-1]], av, zv))
                    for i, rv in enumerate(r):
                        p0 = Geometry._Cyl2Cart(rv, av, zv)
                        add([p0, p0 + [0, 0, tick if zv == z[-1] else -tick]], self.LineQuality(0, i))
            for rv in (r[0], r[-1]):
                for av in (al[0], al[-1]):
                    add(Geometry._Cyl2Cart(rv, av, [z[0], z[-1]]))
            for i, zv in enumerate(z):
                p0 = Geometry._Cyl2Cart(r[-1], al[0], zv)
                add([p0, p0 + tick * np.array([math.cos(al[0]), math.sin(al[0]), 0])], self.LineQuality(2, i))
            for i, av in enumerate(al):
                p0 = Geometry._Cyl2Cart(r[-1], av, z[0])
                add([p0, p0 - [0, 0, tick]], self.LineQuality(1, i))
        return out.ToPolyData()

    def GetBounds(self):
        if self.lines is None:
            return None
        if self.cylindrical:
            r = self.lines[0][-1]
            return (-r, r, -r, r, self.lines[2][0], self.lines[2][-1])
        return tuple(v for l in self.lines for v in (l[0], l[-1]))

    def Nearest(self, world_pos):
        """ {dir: (index, coordinate)} of the mesh lines nearest to a point. """
        if self.lines is None:
            return None
        p = np.asarray(world_pos, float)
        if self.cylindrical:
            c = (math.hypot(p[0], p[1]), math.atan2(p[1], p[0]), p[2])
            names = ('r', 'a', 'z')
        else:
            c = p
            names = ('x', 'y', 'z')
        out = {}
        for n in range(3):
            i = int(np.argmin(np.abs(self.lines[n] - c[n])))
            out[names[n]] = (i, float(self.lines[n][i]))
        return out


###############################################################################
class TreeNode:
    def __init__(self, path, label, kind):
        self.path = path
        self.label = label
        self.kind = kind     # 'group', 'property', 'grid', 'item'
        self.children = []
        self.parent = None


class SceneTree:
    """ Hierarchical scene tree with ``/`` separated paths. """

    def __init__(self):
        self.nodes = {}
        self.roots = []

    def Add(self, path, label, kind):
        if path in self.nodes:
            return self.nodes[path]
        parts = SplitPath(path)
        parent = None
        if len(parts) > 1:
            ppath = '/'.join(parts[:-1])
            parent = self.nodes.get(ppath)
            if parent is None:
                parent = self.Add(ppath, parts[-2].replace('\\/', '/'), 'group')
        node = TreeNode(path, label, kind)
        node.parent = parent
        self.nodes[path] = node
        if parent is None:
            self.roots.append(node)
        else:
            parent.children.append(node)
        return node

    def Remove(self, path):
        node = self.nodes.get(path)
        if node is None:
            return []
        removed = []
        for child in list(node.children):
            removed.extend(self.Remove(child.path))
        if node.parent is None:
            self.roots.remove(node)
        else:
            node.parent.children.remove(node)
        del self.nodes[path]
        removed.append(node)
        return removed

    def Get(self, path):
        node = self.nodes.get(path)
        if node is None:
            raise KeyError('unknown scene tree path "{}"'.format(path))
        return node

    def Leaves(self, node):
        if node.kind != 'group' or node.path == 'grid':
            return [node]
        out = []
        for c in node.children:
            out.extend(self.Leaves(c))
        return out


###############################################################################
class ViewItem:
    """ ViewItem(name, props=(), include_in_bounds=False, exportable=True)

    A named set of VTK props shown by the viewer, the generic extension hook
    for external tools. Add one with :meth:`Viewer.AddItem`. Subclass it to
    package a feature: override the ``On...`` hooks instead of passing
    callbacks around.

    Parameters
    ----------
    name : str
        Name of the item; its tree path is ``<group>/<name>``.
    props : vtkProp or sequence of vtkProp
        Actors, 2D actors (e.g. a scalar bar) or volumes. The caller owns
        their content and updates.
    include_in_bounds : bool
        Include the item's 3D props when fitting the camera.
    exportable : bool
        Include the item's 3D actors in scene exports (glTF).

    Examples
    --------
    >>> item = viewer.AddItem('arrow', actor, group='annotations')
    >>> item.SetVisible(False)
    """

    #: group path used when :meth:`Viewer.AddItem` is given none
    DEFAULT_GROUP = 'items'
    #: tooltips of the settings by name, for those without a ``help`` key
    HELP = {}

    def __init__(self, name, props=(), include_in_bounds=False, exportable=True):
        self._name = name
        if props is None:
            props = ()
        elif not isinstance(props, (list, tuple)):
            props = (props,)
        self._props = list(props)
        self._include_in_bounds = include_in_bounds
        self._exportable = exportable
        self._visible = True
        self._viewer = None
        self._group = None
        self._opacity = 1.0
        self._base_opacity = {}

    # identity
    def GetName(self):
        """ Name of the item. """
        return self._name

    def GetGroup(self):
        """ Tree path of the group holding the item, or None while detached. """
        return self._group

    def GetPath(self):
        """ Tree path ``<group>/<name>``, or None while detached. """
        if self._group is None:
            return None
        return JoinPath(self._group, EscapeName(self._name))

    def GetViewer(self):
        """ The viewer showing this item, or None while detached. """
        return None if self._viewer is None else self._viewer()

    def _Viewer(self):
        v = self.GetViewer()
        if v is None:
            raise RuntimeError('ViewItem "{}" is not attached to a viewer'.format(self._name))
        return v

    def IsIncludedInBounds(self):
        return self._include_in_bounds

    def IsExportable(self):
        return self._exportable

    # content
    def GetProps(self):
        """ List of the VTK props of this item. """
        return list(self._props)

    def AddProp(self, prop):
        """ Add a VTK prop to this item. """
        self._props.append(prop)
        self._ApplyOpacity(prop)
        v = self.GetViewer()
        if v is not None:
            v._scene.AddItemProp(self, prop)

    def RemoveProp(self, prop):
        """ Remove a VTK prop from this item. """
        self._props.remove(prop)
        self._base_opacity.pop(id(prop), None)
        v = self.GetViewer()
        if v is not None:
            v._scene.RemoveItemProp(prop)

    def GetBounds(self):
        """ Union of the bounds of the shown 3D props (of all of them while
        the item is hidden), or None. """
        props = [p for p in self._props if isinstance(p, vtkProp3D)]
        shown = [p for p in props if p.GetVisibility()]
        bounds = [p.GetBounds() for p in (shown or props)]
        return UnionBounds(b for b in bounds if b is not None and b[0] <= b[1])

    # control
    def SetVisible(self, val):
        """ Show or hide the item. """
        v = self.GetViewer()
        if v is None:
            self._visible = bool(val)
        else:
            v.SetVisible(self.GetPath(), val)

    def GetVisible(self):
        return self._visible

    def SetOpacity(self, val):
        """ SetOpacity(val)

        Opacity of the whole item (0..1), the generic ``opacity`` setting of
        every item. It scales the opacity each 3D actor had when it was
        added to the item.
        """
        self._opacity = float(np.clip(val, 0.0, 1.0))
        for p in self._props:
            self._ApplyOpacity(p)

    def GetOpacity(self):
        return self._opacity

    def _ApplyOpacity(self, prop):
        if not isinstance(prop, vtkProp3D) or not hasattr(prop, 'GetProperty'):
            return
        base = self._base_opacity.setdefault(id(prop), prop.GetProperty().GetOpacity())
        prop.GetProperty().SetOpacity(base * self._opacity)

    def AddAnimation(self, n_steps, callback=None):
        """ AddAnimation(n_steps, callback=None)

        Register this item on the viewer's animation clock, see
        :meth:`Viewer.AddAnimation`. Without `callback`,
        :meth:`OnAnimationStep` is called.
        """
        return self._Viewer().AddAnimation(self, n_steps, callback)

    def Render(self):
        """ Request a render from the viewer (after changing the props). """
        self._Viewer().Render()

    def Remove(self):
        """ Remove the item from its viewer. """
        self._Viewer().RemoveItem(self)

    # hooks, no-ops by default
    def OnAdded(self, viewer):
        """ Called after the item was added to `viewer`. """

    def OnRemoved(self):
        """ Called after the item was removed from its viewer. """

    def OnVisibilityChanged(self, visible):
        """ Called after the visibility changed. """

    def OnAnimationStep(self, step):
        """ Show animation step `step` (cheap, must not render). """

    def OnPrepareAnimationStep(self, step):
        """ Build and cache animation step `step` before playback. """

    def GetInfo(self):
        """ Dict shown in the GUI info panel and in Describe(). """
        return {}

    def GetParameters(self):
        """ GetParameters()

        Settings of the item. The GUI shows them as a form in its Settings
        dock, the view state stores them (``items`` key), and scripts change
        them with :meth:`Viewer.SetItemParameter` (or :meth:`SetParameter`).
        Every item also has the setting ``opacity``, handled by the viewer.

        Returns
        -------
        list of dict
            One dict per setting with the keys ``name``, ``value`` and
            ``type``: ``'bool'``, ``'int'``, ``'float'``, ``'choice'``,
            ``'str'``, ``'color'`` or ``'slider'``. Optional keys:

            - ``label``: text in the form (default: the name);
            - ``min``, ``max``, ``step``: numbers;
            - ``choices``: a list, for ``'choice'``, and ``labels``: their
              texts in the form (default ``str()`` of each);
            - ``values``: the numbers a ``'slider'`` steps through (e.g.
              mesh lines); its value is snapped to the nearest one;
            - ``section``: heading the form groups the field under;
            - ``help``: a tooltip explaining the setting (or see
              :attr:`HELP`);
            - ``default``: "Copy view as Python" leaves out a setting at its
              default;
            - ``form`` (default True): False keeps a setting out of the form
              but in the view state, e.g. a list;
            - ``state`` (default True): False marks a form-only field that
              edits other settings and is not stored.

            The form is rebuilt when the set of names changes.

        Examples
        --------
        >>> def GetParameters(self):
        ...     return [dict(name='scale', type='float', value=self.scale, min=0, step=0.1, default=1.0),
        ...             dict(name='component', type='choice', value=self.comp, choices=['x', 'y', 'z'])]
        """
        return []

    def SetParameter(self, name, value):
        """ SetParameter(name, value)

        Change the setting `name` (see :meth:`GetParameters`) and update the
        props; must not render. The GUI renders afterwards, a script calls
        :meth:`Render`.
        """
        raise KeyError('item "{}" has no parameter "{}"'.format(self._name, name))

    def GetDescription(self):
        """ Short text of what the item shows (e.g. its current settings),
        shown next to its name in the GUI tree, or None. """
        return None

    def GetValueText(self, world_pos, prop, cell_id):
        """ GetValueText(world_pos, prop, cell_id)

        Text for the value under the mouse, at `world_pos` on the item's
        `prop` (cell `cell_id`), or None. Shown in the GUI status bar and in
        a corner of the plain window; called often, so it must be cheap.
        """
        return None

    def GetActions(self):
        """ List of ``(label, callable)`` pairs for the item's context menu in
        the GUI tree. The callables take no arguments. """
        return []

    def CreateWidget(self, parent):
        """ CreateWidget(parent)

        An own Qt widget for the GUI, shown below the info panel while the
        item is selected, for settings :meth:`GetParameters` cannot express.
        Only the GUI calls it, so the item stays usable without Qt.

        Returns
        -------
        QWidget or None
        """
        return None


###############################################################################
class Scene:
    """ All renderable content of one viewer. """

    def __init__(self, renderer):
        self.renderer = renderer
        self.entries = []
        self.tree = SceneTree()
        self.grid_layer = GridLayer(self)
        self.items = {}
        self.render_mode = 'solid'
        self.transparency = None
        self.edge_color = None
        self.edge_angle = 30.0
        self.arc_resolution = 32
        self.disc_material = True
        self.labels = False
        self.isolate = None
        self.highlight = []
        self.dim_others = False
        self.top_rank = 0           # depth rank above all properties, see ApplyAll
        self.clip_plane = None
        self.clip_state = None
        self.highlight_actor = None
        self.highlight_surf = None
        self._leaders = None
        self._label_obs = None
        self.freeze_2d = False      # labels keep their place while VTK renders tiles
        self.background = (1, 1, 1)
        self.default_cs = 0
        self.report = []
        self._InitTree()

    def _InitTree(self):
        t = self.tree
        t.Add('properties', 'Properties', 'group')
        for key, label in PROPERTY_GROUPS:
            t.Add('properties/' + key, label, 'group')
        t.Add('grid', 'Grid', 'grid')
        for d in ('x', 'y', 'z'):
            t.Add('grid/' + d, d, 'grid')
        t.Add('grid/outline', 'Outline', 'grid')
        t.Add('grid/quality', 'Quality', 'grid')
        t.Add('items', 'Items', 'group')

    def TextColor(self):
        return (0, 0, 0) if Luminance(self.background) > 0.5 else (1, 1, 1)

    # -- properties
    def SetCSX(self, CSX):
        for e in self.entries:
            e.RemoveActors()
            self.tree.Remove(e.path)
        self.entries = []
        self.CSX = CSX
        if CSX is None:
            self.grid_layer.SetGrid(None)
            return
        self.Update()

    def Update(self, prop=None):
        """ Re-read the structure; rebuild changed properties (or `prop`). """
        CSX = self.CSX
        self.default_cs = CSX.GetCoordInputType()
        self.grid_layer.SetGrid(CSX.GetGrid())
        props = CSX.GetAllProperties()
        old = {id(e.prop): e for e in self.entries}
        new_entries = []
        used = set()
        for p in props:
            e = old.pop(id(p), None)
            if e is None:
                e = PropertyEntry(self, p, None)
            path = JoinPath('properties', PropertyGroup(p), EscapeName(p.GetName()))
            base, n = path, 1
            while path in used:
                n += 1
                path = '{}#{}'.format(base, n)
            used.add(path)
            if e.path != path:
                if e.path is not None:
                    self.tree.Remove(e.path)
                e.path = path
                e.name = p.GetName()
                self.tree.Add(path, p.GetName(), 'property')
            rebuild = e.signature is None or (prop is None and _Signature(p) != e.signature) or prop is p or prop == p.GetName()
            if rebuild:
                e.Build(self.default_cs, self.arc_resolution, self.disc_material)
            new_entries.append(e)
        for e in old.values():
            e.RemoveActors()
            self.tree.Remove(e.path)
        self.entries = new_entries
        self.report = [r for e in self.entries for r in e.report]
        self.ApplyAll()

    def Rebuild(self):
        for e in self.entries:
            e.Build(self.default_cs, self.arc_resolution, self.disc_material)
        self.report = [r for e in self.entries for r in e.report]
        self.ApplyAll()

    def ApplyAll(self):
        prios = sorted({e.Priority() for e in self.entries})
        for e in self.entries:
            e.SetDepthRank(prios.index(e.Priority()))
        self.top_rank = len(prios)
        for e in self.entries:
            e.Apply()
        self.UpdateHighlight()
        self.UpdateLabels()

    def Entry(self, name):
        """ Entry by property name, tree path or property object. """
        for e in self.entries:
            if e.prop is name or e.name == name or e.path == name:
                return e
        raise KeyError('unknown property "{}"'.format(name))

    def Entries(self, names):
        if isinstance(names, str) or not hasattr(names, '__iter__'):
            names = [names]
        return [self.Entry(n) for n in names]

    def StructureBounds(self):
        """ Bounds of the visible properties (of all if none is visible), or None. """
        return (UnionBounds(e.GetBounds() for e in self.entries if e.IsVisible())
                or UnionBounds(e.GetBounds() for e in self.entries))

    def VisibleBounds(self, include_items=True):
        """ Bounds of the visible properties and items counted for the
        bounds; else of all properties, else of the grid; or None. """
        bounds = [e.GetBounds() for e in self.entries if e.IsVisible()]
        if include_items:
            bounds += [it.GetBounds() for it in self.items.values()
                       if it.GetVisible() and it.IsIncludedInBounds()]
        return UnionBounds(bounds) or UnionBounds(e.GetBounds() for e in self.entries) or self.grid_layer.GetBounds()

    # -- clip
    def SetClip(self, clip):
        self.clip_state = clip
        mappers = [a.GetMapper() for e in self.entries for a in e.Actors()]
        mappers += [a.GetMapper() for a in (self.highlight_actor, self.highlight_surf) if a is not None]
        if self.clip_plane is not None:
            for m in mappers:
                m.RemoveClippingPlane(self.clip_plane)
            self.clip_plane = None
        if clip is None:
            return
        normal = clip['normal']
        if isinstance(normal, str):
            normal = {'x': (1, 0, 0), 'y': (0, 1, 0), 'z': (0, 0, 1)}[normal]
        normal = np.asarray(normal, float)
        normal = normal / np.linalg.norm(normal)
        plane = vtkPlane()
        # VTK keeps the side the normal points to
        sign = 1.0 if clip.get('invert', False) else -1.0
        plane.SetNormal(*(sign * normal))
        plane.SetOrigin(*(normal * clip['position']))
        self.clip_plane = plane
        for m in mappers:
            m.AddClippingPlane(plane)

    # -- highlight
    def IsHighlighted(self, name):
        return any(t[0] == name for t in self.highlight)

    def UpdateHighlight(self):
        for a in (self.highlight_actor, self.highlight_surf):
            if a is not None:
                self.renderer.RemoveActor(a)
        self.highlight_actor = self.highlight_surf = None
        if not self.highlight:
            return
        from vtkmodules.vtkFiltersCore import vtkAppendPolyData
        from vtkmodules.vtkFiltersHybrid import vtkPolyDataSilhouette
        app = vtkAppendPolyData()
        surf = vtkAppendPolyData()
        n = 0
        for name, pid in self.highlight:
            e = self.Entry(name)
            pd, cells = e.CellsOf(pid)
            if pd is None or len(cells) == 0:
                continue
            g = ExtractCells(pd, cells)
            fe = vtkFeatureEdges()
            fe.SetInputData(g)
            fe.BoundaryEdgesOn()
            fe.FeatureEdgesOn()
            fe.NonManifoldEdgesOn()
            fe.ManifoldEdgesOff()
            fe.ColoringOff()
            fe.Update()
            app.AddInputData(fe.GetOutput())
            if g.GetNumberOfPolys():
                # curved surfaces (spheres, cylinder mantles) have no feature
                # edges; their outline depends on the view
                sil = vtkPolyDataSilhouette()
                sil.SetInputData(g)
                sil.SetCamera(self.renderer.GetActiveCamera())
                sil.SetEnableFeatureAngle(False)
                sil.BorderEdgesOff()
                app.AddInputConnection(sil.GetOutputPort())
                surf.AddInputData(g)
            if g.GetNumberOfLines() or g.GetNumberOfVerts():
                w = vtkPolyData()
                w.SetPoints(g.GetPoints())
                w.SetLines(g.GetLines())
                w.SetVerts(g.GetVerts())
                app.AddInputData(w)
            n += 1
        if n == 0:
            return
        m = vtkPolyDataMapper()
        m.SetInputConnection(app.GetOutputPort())
        m.ScalarVisibilityOff()
        # above every property (see DEPTH_UNITS); silhouette lines lie on the
        # surface: keep them in front of the tint
        top = self.top_rank + 1
        m.SetRelativeCoincidentTopologyLineOffsetParameters(0, top * DEPTH_UNITS - 8)
        if self.clip_plane is not None:
            m.AddClippingPlane(self.clip_plane)
        a = vtkActor()
        a.SetMapper(m)
        p = a.GetProperty()
        p.SetColor(HIGHLIGHT_COLOR)
        p.SetLineWidth(4)
        p.SetPointSize(12)
        p.SetRenderPointsAsSpheres(True)
        p.LightingOff()
        a.PickableOff()
        self.renderer.AddActor(a)
        self.highlight_actor = a
        if surf.GetNumberOfInputConnections(0):
            # a faint tint, so the shape shows even if the property is hidden
            m = vtkPolyDataMapper()
            m.SetInputConnection(surf.GetOutputPort())
            m.ScalarVisibilityOff()
            m.SetRelativeCoincidentTopologyPolygonOffsetParameters(top * DEPTH_FACTOR, top * DEPTH_UNITS)
            if self.clip_plane is not None:
                m.AddClippingPlane(self.clip_plane)
            a = vtkActor()
            a.SetMapper(m)
            a.GetProperty().SetColor(HIGHLIGHT_COLOR)
            a.GetProperty().SetOpacity(0.2)
            a.PickableOff()
            self.renderer.AddActor(a)
            self.highlight_surf = a

    # -- labels
    def UpdateLabels(self):
        ren = self.renderer
        for e in self.entries:
            if e.label is not None:
                ren.RemoveViewProp(e.label)
                e.label = None
        if self._leaders is not None:
            ren.RemoveViewProp(self._leaders)
            self._leaders = None
        if not self.labels:
            if self._label_obs is not None:
                ren.RemoveObserver(self._label_obs)
                self._label_obs = None
            return
        tc = self.TextColor()
        for e in self.entries:
            if e.GetBounds() is None:
                continue
            lab = vtkTextActor()
            lab.SetInput(e.name)
            tp = lab.GetTextProperty()
            tp.SetFontSize(13)
            tp.SetColor(tc)
            # a backing box in the background colour keeps the text readable over the structure
            tp.SetBackgroundColor(self.background)
            tp.SetBackgroundOpacity(0.7)
            tp.SetFrame(True)
            tp.SetFrameColor(tc)
            lab.PickableOff()
            # relative to the view (not pixels), so tiled screenshots place them right
            lab.GetPositionCoordinate().SetCoordinateSystemToNormalizedViewport()
            lab.SetVisibility(e.IsVisible())
            ren.AddViewProp(lab)
            e.label = lab
        mapper = vtkPolyDataMapper2D()
        mapper.SetInputData(vtkPolyData())
        nv = vtkCoordinate()
        nv.SetCoordinateSystemToNormalizedViewport()
        mapper.SetTransformCoordinate(nv)
        self._leaders = vtkActor2D()
        self._leaders.SetMapper(mapper)
        self._leaders.GetProperty().SetColor(tc)
        self._leaders.GetProperty().SetLineWidth(1)
        self._leaders.GetProperty().SetPointSize(4)
        self._leaders.PickableOff()
        ren.AddViewProp(self._leaders)
        if self._label_obs is None:
            # anchors follow the camera, so place them right before every render
            self._label_obs = ren.AddObserver('StartEvent', lambda o, ev: self.PlaceLabels())

    def PlaceLabels(self):
        """ Attach each label to its property's surface point nearest the camera
        (an exposed corner for boxy shapes) and push overlapping labels apart. """
        if self.freeze_2d:
            return
        ren = self.renderer
        rw = ren.GetRenderWindow()
        if self._leaders is None or rw is None:
            return
        px = rw.GetDPI() / 72.0                     # screenshots with scale > 1 raise the DPI
        cam = ren.GetActiveCamera()
        to_cam = -np.array(cam.GetDirectionOfProjection())
        up = np.array(cam.GetViewUp())
        right = np.cross(-to_cam, up)
        clip = self.clip_plane
        vw, vh = ren.GetSize()
        x0, y0 = ren.GetOrigin()
        todo = []
        for e in self.entries:
            if e.label is None:
                continue
            if not e.label.GetVisibility():
                continue
            pts = e.LabelCandidates()
            if clip is not None and len(pts):
                pts = pts[(pts - clip.GetOrigin()) @ np.array(clip.GetNormal()) >= 0]
            if not len(pts):
                e.label.SetPosition(-10, -10)
                continue
            depth = pts @ to_cam
            span = max(np.ptp(pts, axis=0).max(), 1e-12)
            # among the points nearest the camera, the one furthest up-right on screen
            near = pts[depth >= depth.max() - 1e-3 * span]
            p = near[np.argmax(near @ (up + right))]
            ren.SetWorldPoint(*p, 1.0)
            ren.WorldToDisplay()
            ax, ay, _ = ren.GetDisplayPoint()
            todo.append((e.label, ax - x0, ay - y0))
        # top labels first, each one moved up until it overlaps no earlier one
        placed, lines = [], []
        bb = [0.0] * 4
        gap = 14 * px
        for lab, ax, ay in sorted(todo, key=lambda t: -t[2]):
            lab.SetPosition(0, 0)
            lab.GetBoundingBox(ren, bb)
            w, h = bb[1] - bb[0], bb[3] - bb[2]
            x = ax + gap if ax + gap + w <= vw else ax - gap - w
            y = min(ay + gap, vh - h)
            for _ in range(50):
                hit = [b for b in placed if x < b[0] + b[2] and b[0] < x + w and y < b[1] + b[3] and b[1] < y + h]
                if not hit:
                    break
                y = max(b[1] + b[3] for b in hit) + 2 * px
            placed.append((x, y, w, h))
            lab.SetPosition(x / vw, y / vh)
            # leader from the anchor to the nearer lower corner of the box
            lines.append(((ax, ay), (x if x > ax else x + w, y)))
        pd = vtkPolyData()
        pts = vtkPoints()
        cells, verts = vtkCellArray(), vtkCellArray()
        for (a, b) in lines:
            i = pts.InsertNextPoint(a[0] / vw, a[1] / vh, 0)
            pts.InsertNextPoint(b[0] / vw, b[1] / vh, 0)
            cells.InsertNextCell(2, (i, i + 1))
            verts.InsertNextCell(1, (i,))
        pd.SetPoints(pts)
        pd.SetLines(cells)
        pd.SetVerts(verts)
        self._leaders.GetMapper().SetInputData(pd)

    # -- items
    def AddItemProp(self, item, prop):
        self.renderer.AddViewProp(prop)
        item._ApplyOpacity(prop)
        prop.SetVisibility(item.GetVisible())

    def RemoveItemProp(self, prop):
        self.renderer.RemoveViewProp(prop)
