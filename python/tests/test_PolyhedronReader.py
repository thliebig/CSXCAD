"""Tests for CSPrimPolyhedronReader: STL and PLY files, written on the fly.

Every case is a small file with a known result. Where vtk is installed,
test_same_as_vtk also reads each file with VTK's readers, which CSXCAD used
until it had its own, and compares point by point.
"""
import os
import shutil
import struct
import tempfile
import unittest

import numpy as np

from CSXCAD import ContinuousStructure
from CSXCAD import CSPrimitives

DATA_PATH = os.path.dirname(os.path.abspath(__file__))

SQUARE = [((0, 0, 0), (1, 0, 0), (0, 1, 0)), ((1, 0, 0), (1, 1, 0), (0, 1, 0))]
SQUARE_POINTS = [[0, 0, 0], [1, 0, 0], [0, 1, 0], [1, 1, 0]]
SQUARE_FACES = [[0, 1, 2], [1, 3, 2]]


def ascii_stl(tris, solids=1, name='part'):
    s = ''
    for k in range(solids):
        s += 'solid {}{}\n'.format(name, k)
        for t in tris:
            s += '  facet normal 0 0 1\n    outer loop\n'
            for v in t:
                s += '      vertex {} {} {}\n'.format(*v)
            s += '    endloop\n  endfacet\n'
        s += 'endsolid {}{}\n'.format(name, k)
    return s


def binary_stl(tris, header=b'binary'):
    b = header.ljust(80, b' ') + struct.pack('<I', len(tris))
    for t in tris:
        b += struct.pack('<3f', 0, 0, 1)
        for v in t:
            b += struct.pack('<3f', *v)
        b += b'\0\0'
    return b


def ply_header(fmt, n_vert, n_face, coord='float', count='uchar', index='int', name='vertex_indices'):
    return ('ply\nformat {} 1.0\ncomment written by test_PolyhedronReader\nelement vertex {}\n'
            'property {c} x\nproperty {c} y\nproperty {c} z\nelement face {}\n'
            'property list {} {} {}\nend_header\n').format(fmt, n_vert, n_face, count, index, name, c=coord)


def ply_ascii_body(points, faces):
    return ''.join('{} {} {}\n'.format(*p) for p in points) + \
        ''.join('{} {}\n'.format(len(f), ' '.join(map(str, f))) for f in faces)


def ply_binary_body(points, faces, endian):
    b = b''.join(struct.pack(endian + '3f', *p) for p in points)
    return b + b''.join(struct.pack(endian + 'B{}i'.format(len(f)), len(f), *f) for f in faces)


# PLY scalar types: names (old and new) and struct codes
PLY_TYPES = [('char', 'b'), ('uchar', 'B'), ('short', 'h'), ('ushort', 'H'), ('int', 'i'), ('uint', 'I'),
             ('float', 'f'), ('double', 'd'), ('int8', 'b'), ('uint8', 'B'), ('int16', 'h'), ('uint16', 'H'),
             ('int32', 'i'), ('uint32', 'I'), ('float32', 'f'), ('float64', 'd')]


def ply_all_types(endian, material=True):
    """ A binary PLY with a property of every type around x, y, z, an extra
    element with a list before the faces (VTK 9.7 crashed on that, so it can
    be left out), and an extra list after the vertex indices: the reader
    must skip all of them by their size. """
    fmt = 'binary_little_endian' if endian == '<' else 'binary_big_endian'
    h = 'ply\nformat {} 1.0\nelement vertex 4\n'.format(fmt)
    h += ''.join('property {} p{}\n'.format(t, n) for n, (t, _) in enumerate(PLY_TYPES[:8]))
    h += 'property float64 x\nproperty int16 y\nproperty float32 z\n'
    h += ''.join('property {} q{}\n'.format(t, n) for n, (t, _) in enumerate(PLY_TYPES[8:]))
    if material:
        h += 'element material 2\nproperty list uint8 float32 rgb\nproperty uint16 id\n'
    h += 'element face 2\nproperty list uint8 uint32 vertex_indices\nproperty list uchar double weights\n'
    h += 'end_header\n'
    extra = [1, 2, 3, 4, 5, 6, 7.5, 8.25]
    b = h.encode()
    for p in SQUARE_POINTS:
        b += b''.join(struct.pack(endian + c, v) for (_, c), v in zip(PLY_TYPES[:8], extra))
        b += struct.pack(endian + 'dhf', *p)
        b += b''.join(struct.pack(endian + c, v) for (_, c), v in zip(PLY_TYPES[8:], extra))
    for n in range(2 if material else 0):
        b += struct.pack(endian + 'B3fH', 3, 0.1, 0.2, 0.3, n)
    for f in SQUARE_FACES:
        b += struct.pack(endian + 'B3I', 3, *f) + struct.pack(endian + 'B2d', 2, 0.5, 0.5)
    return b


class Test_PolyhedronReader(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.mkdtemp()
        self.csx = ContinuousStructure()
        self.metal = self.csx.AddMetal('metal')

    def tearDown(self):
        shutil.rmtree(self.dir)

    def write(self, name, data):
        fn = os.path.join(self.dir, name)
        if isinstance(data, bytes):
            with open(fn, 'wb') as f:
                f.write(data)
        else:
            with open(fn, 'w', newline='') as f:
                f.write(data)
        return fn

    def read(self, fn):
        """ (ok, points, faces) as read by CSPrimPolyhedronReader """
        phr = CSPrimitives.CSPrimPolyhedronReader(self.csx.GetParameterSet(), self.metal)
        phr.SetFilename(fn)
        try:
            ok = phr.ReadFile()
        except RuntimeError:   # what python makes of a failed read
            ok = False
        faces = [list(phr.GetFace(n)) for n in range(phr.GetNumFaces())]
        points = np.array([phr.GetVertex(n) for n in range(phr.GetNumVertices())]).reshape(-1, 3)
        return ok, points, faces

    def assertRead(self, fn, points, faces):
        with self.subTest(os.path.basename(fn)):
            ok, pts, fcs = self.read(fn)
            self.assertTrue(ok, fn)
            np.testing.assert_array_equal(pts, np.array(points, dtype=np.float64))
            self.assertEqual(fcs, faces)

    def assertFails(self, fn):
        with self.subTest(os.path.basename(fn)):
            ok, pts, fcs = self.read(fn)
            self.assertFalse(ok, fn)

    # ---- STL

    def test_stl_ascii(self):
        # points shared by both triangles are merged, in order of first appearance
        self.assertRead(self.write('a.stl', ascii_stl(SQUARE)), SQUARE_POINTS, SQUARE_FACES)

    def test_stl_binary(self):
        self.assertRead(self.write('b.stl', binary_stl(SQUARE)), SQUARE_POINTS, SQUARE_FACES)

    def test_stl_binary_header_solid(self):
        # CAD programs write binary files whose header starts with "solid"
        self.assertRead(self.write('b.stl', binary_stl(SQUARE, b'solid exported by CAD')),
                        SQUARE_POINTS, SQUARE_FACES)

    def test_stl_degenerate(self):
        # a triangle with two equal corners is dropped; a new point it brought stays
        tris = [SQUARE[0], ((5, 5, 5), (5, 5, 5), (1, 0, 0)), SQUARE[1]]
        points = SQUARE_POINTS[:3] + [[5, 5, 5], [1, 1, 0]]
        for name, data in (('a.stl', ascii_stl(tris)), ('b.stl', binary_stl(tris))):
            with self.subTest(name):
                self.assertRead(self.write(name, data), points, [[0, 1, 2], [1, 4, 2]])

    def test_stl_ascii_formatting(self):
        # several solids, a solid without a name, CRLF, tabs, exponents
        self.assertRead(self.write('s.stl', ascii_stl(SQUARE, solids=2)), SQUARE_POINTS, SQUARE_FACES*2)
        self.assertRead(self.write('n.stl', ascii_stl(SQUARE).replace('solid part0', 'solid')),
                        SQUARE_POINTS, SQUARE_FACES)
        self.assertRead(self.write('c.stl', ascii_stl(SQUARE).replace('\n', '\r\n')), SQUARE_POINTS, SQUARE_FACES)
        self.assertRead(self.write('t.stl', ascii_stl(SQUARE).replace(' ', '\t')), SQUARE_POINTS, SQUARE_FACES)
        self.assertRead(self.write('e.stl', ascii_stl([((1e-3, 0, 0), (1.5E+1, 0, 0), (0, -2.5e0, 0))])),
                        [[1e-3, 0, 0], [15, 0, 0], [0, -2.5, 0]], [[0, 1, 2]])

    def test_stl_precision(self):
        # ASCII coordinates are read in double precision, binary ones are single precision in the file
        ok, pts, _ = self.read(self.write('p.stl', ascii_stl([((0.1234567890123, 0, 0), (1, 0, 0), (0, 1, 0))])))
        self.assertTrue(ok)
        self.assertEqual(pts[0][0], 0.1234567890123)
        ok, pts, _ = self.read(self.write('p.stl', binary_stl([((0.1234567890123, 0, 0), (1, 0, 0), (0, 1, 0))])))
        self.assertTrue(ok)
        self.assertEqual(pts[0][0], np.float32(0.1234567890123))
        # points merge on their exact coordinates: these two differ only beyond single precision
        a, b = 0.1234567890123, 0.1234567890124
        self.assertEqual(np.float32(a), np.float32(b))
        tris = [((a, 0, 0), (1, 0, 0), (0, 1, 0)), ((b, 0, 0), (0, 1, 0), (0, 0, 1))]
        self.assertRead(self.write('m.stl', ascii_stl(tris)),
                        [[a, 0, 0], [1, 0, 0], [0, 1, 0], [b, 0, 0], [0, 0, 1]], [[0, 1, 2], [3, 2, 4]])

    def test_stl_lenient(self):
        # what VTK rejected: upper case keywords, a facet with four corners, no endsolid
        self.assertRead(self.write('u.stl', ascii_stl(SQUARE).upper()), SQUARE_POINTS, SQUARE_FACES)
        quad = 'solid q\nfacet normal 0 0 1\nouter loop\nvertex 0 0 0\nvertex 1 0 0\nvertex 1 1 0\nvertex 0 1 0\n' \
               'endloop\nendfacet\nendsolid q\n'
        self.assertRead(self.write('q.stl', quad), [[0, 0, 0], [1, 0, 0], [1, 1, 0], [0, 1, 0]], [[0, 1, 2, 3]])
        self.assertRead(self.write('t.stl', ascii_stl(SQUARE).replace('endsolid part0\n', '')),
                        SQUARE_POINTS, SQUARE_FACES)

    def test_stl_invalid(self):
        self.assertFails(os.path.join(self.dir, 'does_not_exist.stl'))
        self.assertFails(self.write('empty.stl', ''))
        self.assertFails(self.write('nofacet.stl', 'solid x\nendsolid x\n'))
        self.assertFails(self.write('garbage.stl', 'solid x\nfacet normal 0 0 1\nouter loop\nvertex 0 a 0\n'))
        self.assertFails(self.write('short.stl', ascii_stl(SQUARE).replace('      vertex 1 1 0\n', '')))
        # a binary file cut short, so its size does not match its triangle count
        self.assertFails(self.write('cut.stl', binary_stl(SQUARE)[:-10]))

    def test_stl_sphere(self):
        self._sphere('sphere.stl')

    def _sphere(self, name):
        # sphere.stl and sphere.ply hold the same sphere of radius 0.5, the points in another order
        ok, pts, faces = self.read(os.path.join(DATA_PATH, name))
        self.assertTrue(ok)
        self.assertEqual((len(pts), len(faces)), (50, 96))
        np.testing.assert_allclose(np.linalg.norm(pts, axis=1), 0.5, rtol=1e-6)
        self.assertEqual({int(i) for f in faces for i in f}, set(range(50)))   # every point in use

    # ---- PLY

    def test_ply_ascii(self):
        # polygons of any size, no merging
        points = SQUARE_POINTS + [[0, 0, 0]]
        faces = [[0, 1, 3, 2], [4, 1, 2]]
        self.assertRead(self.write('a.ply', ply_header('ascii', 5, 2) + ply_ascii_body(points, faces)), points, faces)

    def test_ply_binary(self):
        for fmt, endian in (('binary_little_endian', '<'), ('binary_big_endian', '>')):
            with self.subTest(fmt):
                data = ply_header(fmt, 4, 2).encode() + ply_binary_body(SQUARE_POINTS, SQUARE_FACES, endian)
                self.assertRead(self.write('b.ply', data), SQUARE_POINTS, SQUARE_FACES)

    def test_ply_all_types(self):
        for endian in '<>':
            with self.subTest(endian):
                self.assertRead(self.write('t.ply', ply_all_types(endian)), SQUARE_POINTS, SQUARE_FACES)

    def test_ply_variants(self):
        body = ply_ascii_body(SQUARE_POINTS, SQUARE_FACES)
        self.assertRead(self.write('i.ply', ply_header('ascii', 4, 2, name='vertex_index') + body),
                        SQUARE_POINTS, SQUARE_FACES)
        self.assertRead(self.write('u.ply', ply_header('ascii', 4, 2, count='int', index='ushort') + body),
                        SQUARE_POINTS, SQUARE_FACES)
        self.assertRead(self.write('c.ply', (ply_header('ascii', 4, 2) + body).replace('\n', '\r\n')),
                        SQUARE_POINTS, SQUARE_FACES)

    def test_ply_face_before_vertex(self):
        # allowed by the format (VTK 9.7 crashed on it)
        h = 'ply\nformat ascii 1.0\nelement face 2\nproperty list uchar int vertex_indices\nelement vertex 4\n' \
            'property float x\nproperty float y\nproperty float z\nend_header\n'
        body = ''.join('3 {} {} {}\n'.format(*f) for f in SQUARE_FACES) + \
            ''.join('{} {} {}\n'.format(*p) for p in SQUARE_POINTS)
        self.assertRead(self.write('f.ply', h + body), SQUARE_POINTS, SQUARE_FACES)

    def test_ply_precision(self):
        points = [[0, 0, 0.1234567890123], [1, 0, 0], [0, 1, 0]]
        ok, pts, _ = self.read(self.write('d.ply', ply_header('ascii', 3, 1, coord='double') +
                                          ply_ascii_body(points, [[0, 1, 2]])))
        self.assertTrue(ok)
        self.assertEqual(pts[0][2], 0.1234567890123)

    def test_ply_invalid(self):
        body = ply_ascii_body(SQUARE_POINTS, SQUARE_FACES)
        self.assertFails(os.path.join(self.dir, 'does_not_exist.ply'))
        self.assertFails(self.write('empty.ply', ''))
        self.assertFails(self.write('magic.ply', ply_header('ascii', 4, 2).replace('ply', 'plx', 1) + body))
        self.assertFails(self.write('format.ply', ply_header('ascii', 4, 2).replace('ascii', 'binary') + body))
        self.assertFails(self.write('type.ply', ply_header('ascii', 4, 2).replace('property float x', 'property real x')
                                    + body))
        self.assertFails(self.write('noface.ply', ply_header('ascii', 4, 0) + body))
        self.assertFails(self.write('noz.ply', ply_header('ascii', 4, 2).replace('property float z\n', '') + body))
        self.assertFails(self.write('short.ply', ply_header('ascii', 4, 3) + body))
        self.assertFails(self.write('range.ply', ply_header('ascii', 4, 2) + body.replace('1 3 2', '1 9 2')))
        cut = ply_header('binary_little_endian', 4, 2).encode() + ply_binary_body(SQUARE_POINTS, SQUARE_FACES, '<')
        self.assertFails(self.write('cut.ply', cut[:-3]))

    def test_ply_sphere(self):
        self._sphere('sphere.ply')

    # ---- the same as VTK

    def test_same_as_vtk(self):
        """ Every file VTK reads gives VTK's polygons and points, the
        points up to VTK's single precision. """
        try:
            from vtkmodules.vtkIOGeometry import vtkSTLReader
            from vtkmodules.vtkIOPLY import vtkPLYReader
            from vtkmodules.vtkCommonCore import vtkIdList
            from vtkmodules.util.numpy_support import vtk_to_numpy
        except ImportError:
            self.skipTest('needs vtk')

        def vtk_read(fn):
            r = vtkSTLReader() if fn.endswith('.stl') else vtkPLYReader()
            if fn.endswith('.stl'):
                r.SetMerging(1)
            r.SetFileName(fn)
            r.Update()
            pd = r.GetOutput()
            ids, faces = vtkIdList(), []
            pd.GetPolys().InitTraversal()
            while pd.GetPolys().GetNextCell(ids):
                faces.append([ids.GetId(i) for i in range(ids.GetNumberOfIds())])
            return vtk_to_numpy(pd.GetPoints().GetData()), faces

        tris = [SQUARE[0], ((5, 5, 5), (5, 5, 5), (1, 0, 0)), SQUARE[1]]
        files = [
            os.path.join(DATA_PATH, 'sphere.stl'),
            os.path.join(DATA_PATH, 'sphere.ply'),
            self.write('a.stl', ascii_stl(tris, solids=2)),
            self.write('b.stl', binary_stl(tris, b'solid exported by CAD')),
            self.write('c.stl', ascii_stl([((0.1234567890123, 1e-3, 0), (1.5E+1, 0, 0), (0, -2.5e0, 0))])
                       .replace('\n', '\r\n')),
            self.write('a.ply', ply_header('ascii', 4, 2, coord='double') + ply_ascii_body(SQUARE_POINTS, [[0, 1, 3, 2], [0, 0, 2]])),
            self.write('l.ply', ply_all_types('<', material=False)),
            self.write('g.ply', ply_all_types('>', material=False)),
        ]
        for fn in files:
            with self.subTest(os.path.basename(fn)):
                pts, faces = vtk_read(fn)
                self.assertGreater(len(faces), 0)
                ok, our_pts, our_faces = self.read(fn)
                self.assertTrue(ok)
                np.testing.assert_allclose(our_pts, pts, rtol=1e-7, atol=1e-12)
                self.assertEqual(our_faces, faces)


if __name__ == '__main__':
    unittest.main()
