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

"""CSXCAD.Viewer.Geometry
Convert CSXCAD primitives and properties into ``vtkPolyData``.

Window-free: needs only ``vtk``, ``numpy`` and CSXCAD.

Every cell carries the int32 cell arrays ``primitive_id`` (``prim.GetID()``)
and ``primitive_index`` (position in ``prop.GetAllPrimitives()``), so a
whole property is one polydata (and one actor) while picking and
highlighting still find the primitive of each cell. Coordinates are in
drawing units, as in the structure.

Primitives that cannot be rendered are not an error: they are collected in a
report, a list of dicts ``{prop, primitive_id, type, reason}``.
"""

import math

import numpy as np

from vtkmodules.vtkCommonCore import vtkPoints, vtkIdTypeArray, vtkIntArray
from vtkmodules.vtkCommonDataModel import vtkPolyData, vtkCellArray
from vtkmodules.vtkCommonTransforms import vtkTransform
from vtkmodules.vtkFiltersCore import (vtkAppendPolyData, vtkCleanPolyData,
                                       vtkPolyDataNormals, vtkTriangleFilter)
from vtkmodules.vtkFiltersGeneral import vtkTransformFilter
from vtkmodules.vtkFiltersSources import vtkSphereSource
from vtkmodules.util.numpy_support import numpy_to_vtk, vtk_to_numpy

from CSXCAD import CSPrimitives
from CSXCAD.CSProperties import PropertyType

PT = CSPrimitives.PrimitiveType
CARTESIAN = 0
CYLINDRICAL = 1

__all__ = ['PrimitivePolyData', 'PropertyPolyData', 'DiscMaterialPolyData',
           'CSXPolyData', 'PolyDataFromArrays']


class _Mesh:
    """ Collects points and cells in plain numpy before building a vtkPolyData. """
    def __init__(self):
        self._points = []
        self._npts = 0
        self.polys = []
        self.lines = []
        self.verts = []

    def AddPoints(self, pts):
        pts = np.asarray(pts, dtype=np.float64).reshape(-1, 3)
        off = self._npts
        self._points.append(pts)
        self._npts += len(pts)
        return off

    def GetPoints(self):
        if not self._points:
            return np.zeros((0, 3))
        return np.concatenate(self._points)

    def AddGrid(self, P, wrap_u=False, wrap_v=False, outward=None):
        """ Add quads of a (nu, nv, 3) point grid, optionally closed in u and/or v. """
        P = np.asarray(P, dtype=np.float64)
        nu, nv = P.shape[:2]
        off = self.AddPoints(P.reshape(-1, 3))
        idx = off + np.arange(nu * nv).reshape(nu, nv)
        if wrap_u:
            idx = np.concatenate([idx, idx[:1]], axis=0)
        if wrap_v:
            idx = np.concatenate([idx, idx[:, :1]], axis=1)
        quads = np.stack([idx[:-1, :-1], idx[1:, :-1], idx[1:, 1:], idx[:-1, 1:]], axis=-1).reshape(-1, 4)
        self.polys.extend(_Orient(self.GetPoints(), list(quads), outward))

    def AddFan(self, center, ring, closed, outward=None):
        """ Triangle fan from `center` to consecutive `ring` points. """
        off = self.AddPoints(np.vstack([center, ring]))
        n = len(ring)
        tris = [(off, off + 1 + i, off + 1 + (i + 1) % n) for i in range(n if closed else n - 1)]
        self.polys.extend(_Orient(self.GetPoints(), tris, outward))

    def AddPolygon(self, P, outward=None):
        """ Add a planar (possibly concave) polygon, triangulated. """
        off = self.AddPoints(P)
        tris = _TriangulatePolygon(np.asarray(P, dtype=np.float64))
        self.polys.extend(_Orient(self.GetPoints(), [tuple(off + t) for t in tris], outward))

    def AddLine(self, P, closed=False):
        off = self.AddPoints(P)
        ids = list(range(off, off + len(P)))
        if closed:
            ids.append(off)
        self.lines.append(ids)

    def AddVertex(self, p):
        self.verts.append([self.AddPoints(p)])

    def ToPolyData(self):
        return PolyDataFromArrays(self.GetPoints(), polys=self.polys, lines=self.lines, verts=self.verts)


def _CellArray(cells):
    cells = list(cells)
    sizes = np.array([len(c) for c in cells], dtype=np.int64)
    offsets = np.zeros(len(cells) + 1, dtype=np.int64)
    np.cumsum(sizes, out=offsets[1:])
    if len(cells):
        conn = np.concatenate([np.asarray(c, dtype=np.int64) for c in cells])
    else:
        conn = np.zeros(0, dtype=np.int64)
    return _CellArrayFromOffsets(conn, offsets)


def _CellArrayFromOffsets(conn, offsets):
    ca = vtkCellArray()
    o = vtkIdTypeArray()
    o.DeepCopy(numpy_to_vtk(np.ascontiguousarray(offsets, dtype=np.int64), deep=1))
    c = vtkIdTypeArray()
    c.DeepCopy(numpy_to_vtk(np.ascontiguousarray(conn, dtype=np.int64), deep=1))
    ca.SetData(o, c)
    return ca


def PolyDataFromArrays(points, polys=None, lines=None, verts=None, offsets=None):
    """ PolyDataFromArrays(points, polys=None, lines=None, verts=None, offsets=None)

    Build a ``vtkPolyData`` from numpy data.

    Parameters
    ----------
    points : (N,3) array_like
    polys, lines, verts : list of index sequences, or (M,k) array
        Cells of the respective kind. With `offsets` given, `polys` is a flat
        connectivity array in VTK cell array layout instead.
    offsets : array_like, optional
        Offsets into a flat `polys` connectivity (number of polys + 1).
    """
    pd = vtkPolyData()
    pts = vtkPoints()
    pts.SetData(numpy_to_vtk(np.ascontiguousarray(points, dtype=np.float64).reshape(-1, 3), deep=1))
    pd.SetPoints(pts)
    if offsets is not None:
        pd.SetPolys(_CellArrayFromOffsets(polys, offsets))
    elif polys is not None and len(polys):
        pd.SetPolys(_CellArray(polys))
    if lines is not None and len(lines):
        pd.SetLines(_CellArray(lines))
    if verts is not None and len(verts):
        pd.SetVerts(_CellArray(verts))
    return pd


def _Normal(P, cell):
    """ Newell normal of a polygon given by point indices. """
    pts = P[list(cell)]
    nxt = np.roll(pts, -1, axis=0)
    return np.array([np.sum((pts[:, 1] - nxt[:, 1]) * (pts[:, 2] + nxt[:, 2])),
                     np.sum((pts[:, 2] - nxt[:, 2]) * (pts[:, 0] + nxt[:, 0])),
                     np.sum((pts[:, 0] - nxt[:, 0]) * (pts[:, 1] + nxt[:, 1]))])


def _Orient(P, cells, outward):
    """ Flip all cells if their (common) orientation is against `outward`. """
    if outward is None or not cells:
        return [tuple(c) for c in cells]
    for c in cells:
        n = _Normal(P, c)
        if np.dot(n, n) > 0:
            break
    if np.dot(n, outward) < 0:
        return [tuple(c[::-1]) for c in cells]
    return [tuple(c) for c in cells]


def _TriangulatePolygon(P):
    """ Ear-cut triangulation (VTK) of one planar polygon, as (M,3) local indices. """
    pd = PolyDataFromArrays(P, polys=[list(range(len(P)))])
    tf = vtkTriangleFilter()
    tf.SetInputData(pd)
    tf.Update()
    out = tf.GetOutput()
    if out.GetNumberOfPolys() == 0:
        return np.zeros((0, 3), dtype=np.int64)
    return vtk_to_numpy(out.GetPolys().GetConnectivityArray()).reshape(-1, 3)


def _OrientClosed(pd):
    """ Merge coincident points and orient all faces of a closed solid outward. """
    clean = vtkCleanPolyData()
    clean.SetInputData(pd)
    clean.SetTolerance(1e-9)
    clean.PointMergingOn()
    norm = vtkPolyDataNormals()
    norm.SetInputConnection(clean.GetOutputPort())
    norm.SplittingOff()
    norm.ConsistencyOn()
    norm.AutoOrientNormalsOn()
    norm.ComputePointNormalsOff()
    # without any normals to compute, the filter does nothing, the reordering neither;
    # the cell normals are dropped below
    norm.ComputeCellNormalsOn()
    norm.Update()
    out = vtkPolyData()
    out.ShallowCopy(norm.GetOutput())
    out.GetPointData().Initialize()
    out.GetCellData().Initialize()
    return out


def _Cyl2Cart(r, a, z):
    r = np.asarray(r, dtype=np.float64)
    a = np.asarray(a, dtype=np.float64)
    return np.stack(np.broadcast_arrays(r * np.cos(a), r * np.sin(a), np.asarray(z, dtype=np.float64)), axis=-1)


def _ToCart(coord, cs):
    coord = np.asarray(coord, dtype=np.float64)
    if cs == CYLINDRICAL:
        return _Cyl2Cart(coord[..., 0], coord[..., 1], coord[..., 2])
    return coord


def _NumSteps(angle, arc_resolution, arc_step=None):
    """ Number of segments for an arc of `angle` rad. """
    step = 2 * math.pi / max(3, arc_resolution)
    if arc_step is not None:
        step = min(step, arc_step)
    return max(1, int(math.ceil(abs(angle) / step - 1e-9)))


def _Basis(axis):
    """ Two unit vectors orthogonal to `axis` (and each other). """
    axis = axis / np.linalg.norm(axis)
    helper = np.array([1.0, 0, 0]) if abs(axis[0]) < 0.9 else np.array([0, 1.0, 0])
    e1 = np.cross(axis, helper)
    e1 /= np.linalg.norm(e1)
    e2 = np.cross(axis, e1)
    return e1, e2, axis


###############################################################################
# primitive builders, all return (mesh or polydata, closed) or raise _Skip

class _Skip(Exception):
    pass


def _BoxCart(start, stop):
    lo = np.minimum(start, stop)
    hi = np.maximum(start, stop)
    ext = hi > lo
    k = int(np.count_nonzero(ext))
    m = _Mesh()
    if k == 0:
        m.AddVertex(lo)
        return m, False
    if k == 1:
        m.AddLine([lo, hi])
        return m, False
    if k == 2:
        n = int(np.argmin(ext))
        a, b = (n + 1) % 3, (n + 2) % 3
        P = np.zeros((2, 2, 3))
        P[:, :, n] = lo[n]
        P[:, :, a] = np.array([lo[a], hi[a]])[:, None]
        P[:, :, b] = np.array([lo[b], hi[b]])[None, :]
        e = np.zeros(3)
        e[n] = 1
        m.AddGrid(P, outward=e)
        return m, False
    for n in range(3):
        a, b = (n + 1) % 3, (n + 2) % 3
        for side, val in ((-1, lo[n]), (1, hi[n])):
            P = np.zeros((2, 2, 3))
            P[:, :, n] = val
            P[:, :, a] = np.array([lo[a], hi[a]])[:, None]
            P[:, :, b] = np.array([lo[b], hi[b]])[None, :]
            e = np.zeros(3)
            e[n] = side
            m.AddGrid(P, outward=e)
    return m, True


def _BoxCyl(start, stop, arc_resolution, arc_step):
    (r0, a0, z0), (r1, a1, z1) = np.minimum(start, stop), np.maximum(start, stop)
    r0 = max(r0, 0.0)
    r1 = max(r1, 0.0)
    da = a1 - a0
    full = da >= 2 * math.pi - 1e-12
    if full:
        a1 = a0 + 2 * math.pi
        da = 2 * math.pi
    na = _NumSteps(da, arc_resolution, arc_step)
    if full:
        na = max(na, 3)
    alphas = np.linspace(a0, a1, na + 1)
    if full:
        alphas = alphas[:-1]
    dr, dz = r1 > r0, z1 > z0
    has_a = da > 0 and r1 > 0
    m = _Mesh()
    k = int(dr) + int(has_a) + int(dz)
    radial = lambda a: np.array([math.cos(a), math.sin(a), 0.0])
    tangent = lambda a: np.array([-math.sin(a), math.cos(a), 0.0])
    am = 0.5 * (a0 + a1)

    def z_face(z, outward):
        if r0 > 0:
            P = np.stack([_Cyl2Cart(r0, alphas, z), _Cyl2Cart(r1, alphas, z)], axis=1)
            m.AddGrid(P, wrap_u=full, outward=outward)
        else:
            m.AddFan(_Cyl2Cart(0, 0, z), _Cyl2Cart(r1, alphas, z), closed=full, outward=outward)

    def r_face(r, outward):
        P = np.stack([_Cyl2Cart(r, alphas, z0), _Cyl2Cart(r, alphas, z1)], axis=1)
        m.AddGrid(P, wrap_u=full, outward=outward)

    def a_face(a, outward):
        P = np.array([[_Cyl2Cart(r0, a, z0), _Cyl2Cart(r0, a, z1)],
                      [_Cyl2Cart(r1, a, z0), _Cyl2Cart(r1, a, z1)]])
        m.AddGrid(P, outward=outward)

    if k == 3:
        r_face(r1, radial(am))
        if r0 > 0:
            r_face(r0, -radial(am))
        z_face(z0, np.array([0, 0, -1.0]))
        z_face(z1, np.array([0, 0, 1.0]))
        if not full:
            a_face(a0, -tangent(a0))
            a_face(a1, tangent(a1))
        return m, True
    if k == 2:
        if not dz:
            z_face(z0, np.array([0, 0, 1.0]))
        elif not has_a:
            a_face(a0, tangent(a0))
        else:
            r_face(r1, radial(am))
        return m, False
    if dr:
        m.AddLine([_Cyl2Cart(r0, a0, z0), _Cyl2Cart(r1, a0, z0)])
    elif dz:
        m.AddLine([_Cyl2Cart(r0, a0, z0), _Cyl2Cart(r0, a0, z1)])
    elif has_a:
        m.AddLine(_Cyl2Cart(r1, alphas, z0), closed=full)
    else:
        m.AddVertex(_Cyl2Cart(r0, a0, z0))
    return m, False


def _Box(start, stop, cs, arc_resolution, arc_step):
    if cs == CYLINDRICAL:
        return _BoxCyl(np.asarray(start, float), np.asarray(stop, float), arc_resolution, arc_step)
    return _BoxCart(np.asarray(start, float), np.asarray(stop, float))


def _Sphere(center, radius, arc_resolution):
    if radius <= 0:
        raise _Skip('zero radius')
    src = vtkSphereSource()
    src.SetCenter(*center)
    src.SetRadius(radius)
    src.SetThetaResolution(max(8, arc_resolution))
    src.SetPhiResolution(max(5, arc_resolution // 2 + 1))
    src.Update()
    out = vtkPolyData()
    out.ShallowCopy(src.GetOutput())
    out.GetPointData().Initialize()
    return out


def _SphericalShell(center, radius, width, arc_resolution):
    r_o = radius + width / 2
    r_i = radius - width / 2
    if r_o <= 0:
        raise _Skip('zero radius')
    outer = _Sphere(center, r_o, arc_resolution)
    if r_i <= 0:
        return outer, False
    inner = _Sphere(center, r_i, arc_resolution)
    # flip the inner surface so that the shell is a closed solid
    P = vtk_to_numpy(inner.GetPoints().GetData())
    conn = vtk_to_numpy(inner.GetPolys().GetConnectivityArray()).reshape(-1, 3)[:, ::-1]
    inner = PolyDataFromArrays(P, polys=conn)
    app = vtkAppendPolyData()
    app.AddInputData(outer)
    app.AddInputData(inner)
    app.Update()
    out = vtkPolyData()
    out.ShallowCopy(app.GetOutput())
    return out, False


def _Cylinder(start, stop, r_in, r_out, arc_resolution):
    axis = np.asarray(stop, float) - np.asarray(start, float)
    length = np.linalg.norm(axis)
    if length == 0:
        raise _Skip('zero length')
    if r_out <= 0:
        raise _Skip('zero radius')
    e1, e2, ax = _Basis(axis)
    n = max(8, arc_resolution)
    phi = np.linspace(0, 2 * math.pi, n, endpoint=False)
    ring = np.cos(phi)[:, None] * e1 + np.sin(phi)[:, None] * e2
    p0 = np.asarray(start, float)
    p1 = p0 + axis
    m = _Mesh()
    ref = ring[0]
    m.AddGrid(np.stack([p0 + r_out * ring, p1 + r_out * ring], axis=1), wrap_u=True, outward=ref)
    if r_in > 0:
        m.AddGrid(np.stack([p0 + r_in * ring, p1 + r_in * ring], axis=1), wrap_u=True, outward=-ref)
        for p, s in ((p0, -1), (p1, 1)):
            m.AddGrid(np.stack([p + r_in * ring, p + r_out * ring], axis=1), wrap_u=True, outward=s * ax)
    else:
        for p, s in ((p0, -1), (p1, 1)):
            m.AddPolygon(p + r_out * ring, outward=s * ax)
    return m, True


def _PolygonPoints(prim, elevation=None):
    x0, x1 = prim.GetCoords()
    n = prim.GetNormDir()
    P = np.zeros((len(x0), 3))
    P[:, n] = prim.GetElevation() if elevation is None else elevation
    P[:, (n + 1) % 3] = x0
    P[:, (n + 2) % 3] = x1
    if len(P) > 1 and np.array_equal(P[0], P[-1]):
        P = P[:-1]
    if len(P) < 3:
        raise _Skip('less than 3 polygon points')
    return P, n


def _Polygon(prim):
    P, n = _PolygonPoints(prim)
    m = _Mesh()
    e = np.zeros(3)
    e[n] = 1
    m.AddPolygon(P, outward=e)
    return m, False


def _LinPoly(prim):
    P, n = _PolygonPoints(prim)
    h = np.zeros(3)
    h[n] = prim.GetLength()
    if h[n] == 0:
        return _Polygon(prim)
    m = _Mesh()
    m.AddPolygon(P, outward=-h)
    m.AddPolygon(P + h, outward=h)
    m.AddGrid(np.stack([P, P + h], axis=1), wrap_u=True)
    return m, True


def _RotPoly(prim, arc_resolution):
    # the profile lies in the plane through the rotation axis, angle 0 is
    # its half-plane with positive "radial" coordinate (cf. CSPrimRotPoly::IsInside)
    P, n = _PolygonPoints(prim, elevation=0.0)
    a = prim.GetRotAxisDir()
    if a == n:
        raise _Skip('rotation axis equals the polygon normal')
    d = 3 - a - n
    e_a = np.eye(3)[a]
    e_d = np.eye(3)[d]
    e_x = np.cross(e_a, e_d)
    a0, a1 = prim.GetAngle()
    if a1 < a0:
        a0, a1 = a1, a0
    full = (a1 - a0) >= 2 * math.pi - 1e-12
    if full:
        a1 = a0 + 2 * math.pi
    ns = _NumSteps(a1 - a0, arc_resolution)
    th = np.linspace(a0, a1, ns + 1)
    if full:
        th = th[:-1]
    u = P[:, a]
    v = P[:, d]

    def at(t):
        return u[:, None] * e_a + v[:, None] * (math.cos(t) * e_d + math.sin(t) * e_x)

    m = _Mesh()
    # (n_profile, n_angle, 3), closed along the profile and, if full, the sweep
    m.AddGrid(np.stack([at(t) for t in th], axis=1), wrap_u=True, wrap_v=full)
    if not full:
        sgn = 1.0 if np.mean(v) >= 0 else -1.0
        t0 = -math.sin(a0) * e_d + math.cos(a0) * e_x
        t1 = -math.sin(a1) * e_d + math.cos(a1) * e_x
        m.AddPolygon(at(a0), outward=-sgn * t0)
        m.AddPolygon(at(a1), outward=sgn * t1)
    return m, True


def _Polyhedron(prim):
    if prim.GetNumVertices() == 0 or prim.GetNumFaces() == 0:
        raise _Skip('empty polyhedron')
    verts = prim.GetVertices()
    conn, offsets = prim.GetFaces()
    valid = prim.GetFacesValid()
    # the flags are only set once CSXCAD has checked the polyhedron (on
    # Update()), before that none is valid and all faces are shown
    if valid.any() and not valid.all():
        sizes = np.diff(offsets)
        keep = np.repeat(valid, sizes)
        conn = conn[keep]
        offsets = np.zeros(np.count_nonzero(valid) + 1, dtype=np.int64)
        np.cumsum(sizes[valid], out=offsets[1:])
    if len(offsets) < 2:
        raise _Skip('no valid faces')
    return PolyDataFromArrays(verts, polys=conn, offsets=offsets), False


def _CurvePoints(prim, cs):
    N = prim.GetNumberOfPoints()
    if N < 2:
        raise _Skip('less than 2 curve points')
    return _ToCart(np.array([prim.GetPoint(i) for i in range(N)]), cs)


def _Wire(prim, cs, arc_resolution):
    from vtkmodules.vtkFiltersCore import vtkTubeFilter
    P = _CurvePoints(prim, cs)
    radius = prim.GetWireRadius()
    m = _Mesh()
    m.AddLine(P)
    if radius <= 0:
        return m, False
    clean = vtkCleanPolyData()
    clean.SetInputData(m.ToPolyData())
    tube = vtkTubeFilter()
    tube.SetInputConnection(clean.GetOutputPort())
    tube.SetRadius(radius)
    tube.SetNumberOfSides(max(8, arc_resolution // 4))
    tube.CappingOn()
    tris = vtkTriangleFilter()  # the tube consists of triangle strips
    tris.SetInputConnection(tube.GetOutputPort())
    tris.Update()
    out = vtkPolyData()
    out.ShallowCopy(tris.GetOutput())
    out.GetPointData().Initialize()
    return out, False


def _ApplyMatrix(pd, matrix):
    tr = vtkTransform()
    tr.SetMatrix(np.asarray(matrix, dtype=np.float64).ravel())
    flt = vtkTransformFilter()
    flt.SetInputData(pd)
    flt.SetTransform(tr)
    flt.Update()
    out = vtkPolyData()
    out.ShallowCopy(flt.GetOutput())
    out.GetPointData().Initialize()
    return out


def _PrimCS(prim, default_cs):
    cs = prim.GetCoordinateSystem()
    return default_cs if cs is None else cs


def _Report(report, prim, reason, prop=None):
    if report is None:
        return
    if prop is None:
        prop = prim.GetProperty()
    report.append(dict(prop=prop.GetName() if prop is not None else None,
                       primitive_id=prim.GetID(), type=prim.GetTypeName(),
                       reason=reason))


def PrimitivePolyData(prim, default_cs=CARTESIAN, arc_resolution=32, arc_step_deg=3.0, report=None):
    """ PrimitivePolyData(prim, default_cs=0, arc_resolution=32, arc_step_deg=3.0, report=None)

    Tessellate a single primitive.

    Parameters
    ----------
    prim : CSPrimitives
    default_cs : int
        Coordinate system used if the primitive has none of its own, usually
        ``CSX.GetCoordInputType()`` (0: Cartesian, 1: cylindrical).
    arc_resolution : int
        Number of segments of a full circle.
    arc_step_deg : float
        Maximum angular step for cylindrical boxes.
    report : list, optional
        Unrenderable primitives are appended here (dicts with ``prop``,
        ``primitive_id``, ``type`` and ``reason``).

    Returns
    -------
    vtkPolyData or None
        In drawing units, with the primitive transform applied.
    """
    pt = prim.GetType()
    cs = _PrimCS(prim, default_cs)
    arc_step = math.radians(arc_step_deg)
    try:
        if pt == PT.POINT:
            m = _Mesh()
            m.AddVertex(_ToCart(prim.GetCoord(), cs))
            res = (m, False)
        elif pt == PT.BOX:
            res = _Box(prim.GetStart(), prim.GetStop(), cs, arc_resolution, arc_step)
        elif pt == PT.MULTIBOX:
            # multi boxes are always defined in the mesh coordinate system
            app = vtkAppendPolyData()
            for i in range(prim.GetQtyBoxes()):
                start, stop = prim.GetBox(i)
                part, closed = _Box(start, stop, default_cs, arc_resolution, arc_step)
                part = part.ToPolyData()
                app.AddInputData(_OrientClosed(part) if closed else part)
            if prim.GetQtyBoxes() == 0:
                raise _Skip('no boxes')
            app.Update()
            res = (app.GetOutput(), False)
        elif pt == PT.SPHERE:
            res = (_Sphere(_ToCart(prim.GetCenter(), cs), prim.GetRadius(), arc_resolution), False)
        elif pt == PT.SPHERICALSHELL:
            res = _SphericalShell(_ToCart(prim.GetCenter(), cs), prim.GetRadius(), prim.GetShellWidth(), arc_resolution)
        elif pt == PT.CYLINDER:
            res = _Cylinder(_ToCart(prim.GetStart(), cs), _ToCart(prim.GetStop(), cs), 0, prim.GetRadius(), arc_resolution)
        elif pt == PT.CYLINDRICALSHELL:
            r, w = prim.GetRadius(), prim.GetShellWidth()
            res = _Cylinder(_ToCart(prim.GetStart(), cs), _ToCart(prim.GetStop(), cs), r - w / 2, r + w / 2, arc_resolution)
        elif pt == PT.POLYGON:
            res = _Polygon(prim)
        elif pt == PT.LINPOLY:
            res = _LinPoly(prim)
        elif pt == PT.ROTPOLY:
            res = _RotPoly(prim, arc_resolution)
        elif pt in (PT.POLYHEDRON, PT.POLYHEDRONREADER):
            res = _Polyhedron(prim)
        elif pt == PT.CURVE:
            m = _Mesh()
            m.AddLine(_CurvePoints(prim, cs))
            res = (m, False)
        elif pt == PT.WIRE:
            res = _Wire(prim, cs, arc_resolution)
        elif pt == PT.USERDEFINED:
            raise _Skip('unsupported')
        else:
            raise _Skip('unknown primitive type')
    except _Skip as e:
        _Report(report, prim, str(e))
        return None

    pd, closed = res
    if isinstance(pd, _Mesh):
        pd = pd.ToPolyData()
    if closed:
        pd = _OrientClosed(pd)
    if prim.HasTransform():
        pd = _ApplyMatrix(pd, prim.GetTransform().GetMatrix())
    if pd.GetNumberOfCells() == 0:
        _Report(report, prim, 'empty')
        return None
    return pd


def _AddCellIds(pd, prim_id, prim_index):
    n = pd.GetNumberOfCells()
    for name, val in (('primitive_id', prim_id), ('primitive_index', prim_index)):
        arr = vtkIntArray()
        arr.SetName(name)
        arr.SetNumberOfTuples(n)
        arr.Fill(val)
        pd.GetCellData().AddArray(arr)


def _Append(parts):
    if not parts:
        return None
    if len(parts) == 1:
        return parts[0]
    app = vtkAppendPolyData()
    for p in parts:
        app.AddInputData(p)
    app.Update()
    out = vtkPolyData()
    out.ShallowCopy(app.GetOutput())
    return out


def PropertyPolyData(prop, default_cs=CARTESIAN, arc_resolution=32, arc_step_deg=3.0):
    """ PropertyPolyData(prop, default_cs=0, arc_resolution=32, arc_step_deg=3.0)

    Tessellate all primitives of a property into one polydata.

    Returns
    -------
    polydata : vtkPolyData or None
        None if no primitive could be rendered. Cell arrays ``primitive_id``
        and ``primitive_index`` identify the primitive of each cell.
    report : list of dict
        Primitives that could not be rendered.
    """
    report = []
    parts = []
    for idx, prim in enumerate(prop.GetAllPrimitives()):
        pd = PrimitivePolyData(prim, default_cs, arc_resolution, arc_step_deg, report)
        if pd is None:
            continue
        pd = _Detach(pd)
        _AddCellIds(pd, prim.GetID(), idx)
        parts.append(pd)
    for r in report:
        r['prop'] = prop.GetName()
    return _Append(parts), report


def _Detach(pd):
    out = vtkPolyData()
    out.ShallowCopy(pd)
    out.GetCellData().Initialize()
    return out


def DiscMaterialPolyData(prop):
    """ DiscMaterialPolyData(prop)

    Surface model of a discrete material (``CSPropDiscMaterial``), in
    structure coordinates. Its cells have ``primitive_id`` -1.

    Returns
    -------
    vtkPolyData or None
        None if the property is no discrete material or has no valid data.
    """
    if not (prop.GetType() & PropertyType.DISCRETE_MATERIAL) or not hasattr(prop, 'GetPolyDataModel'):
        return None
    points, quads = prop.GetPolyDataModel()
    if len(quads) == 0:
        return None
    pd = PolyDataFromArrays(points, polys=quads)
    _AddCellIds(pd, -1, -1)
    return pd


def CSXPolyData(CSX, include_hidden=False, arc_resolution=32, arc_step_deg=3.0, disc_material=True):
    """ CSXPolyData(CSX, include_hidden=False, arc_resolution=32, arc_step_deg=3.0, disc_material=True)

    Tessellate all properties of a structure.

    Returns
    -------
    polydata : dict
        ``{property name: vtkPolyData}``, in property order. A repeated
        property name gets a suffix ``_2``, ``_3``, ...
    report : list of dict
        Primitives that could not be rendered.
    """
    cs = CSX.GetCoordInputType()
    result = {}
    report = []
    for prop in CSX.GetAllProperties():
        if not include_hidden and not prop.GetVisibility():
            continue
        pd, rep = PropertyPolyData(prop, cs, arc_resolution, arc_step_deg)
        report.extend(rep)
        if disc_material:
            model = DiscMaterialPolyData(prop)
            if model is not None:
                pd = model if pd is None else _Append([pd, model])
        if pd is None:
            continue
        name = prop.GetName()
        key, n = name, 1
        while key in result:
            n += 1
            key = '{}_{}'.format(name, n)
        result[key] = pd
    return result, report
