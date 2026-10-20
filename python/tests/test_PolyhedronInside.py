"""Tests for CSPrimPolyhedron.IsInside: what is metal inside a surface of faces.

The surface counts as inside, as for a box. Off the surface the winding
number decides when the faces are consistently oriented (nested or
overlapping bodies are solid, a cavity whose faces point into it is a
hole), the number of crossings otherwise (every second region is a hole).
"""
import itertools
import unittest

import numpy as np

from CSXCAD import ContinuousStructure
from CSXCAD import CSPrimitives
from CSXCAD.CSTransform import CSTransform

# the corners of a unit cube and its faces as quads, counter-clockwise seen from outside
CUBE_POINTS = np.array(list(itertools.product((0, 1), repeat=3)), dtype=float)[:, ::-1]  # x fastest
CUBE_QUADS = [[0, 2, 3, 1], [4, 5, 7, 6], [0, 1, 5, 4], [2, 6, 7, 3], [0, 4, 6, 2], [1, 3, 7, 5]]


def cube_faces(triangles=True, flip=False):
    faces = []
    for q in CUBE_QUADS:
        q = q[::-1] if flip else q
        faces += [[q[0], q[1], q[2]], [q[0], q[2], q[3]]] if triangles else [q]
    return faces


def box_points(start, stop):
    start, stop = np.asarray(start, float), np.asarray(stop, float)
    return start + CUBE_POINTS * (stop - start)


class Test_PolyhedronInside(unittest.TestCase):
    def setUp(self):
        self.csx = ContinuousStructure()
        self.metal = self.csx.AddMetal('metal')

    def poly(self, bodies):
        """ A polyhedron of bodies, each (points, faces), its own point indices. """
        ph = CSPrimitives.CSPrimPolyhedron(self.csx.GetParameterSet(), self.metal)
        off = 0
        for points, faces in bodies:
            for p in points:
                ph.AddVertex(*p)
            for f in faces:
                ph.AddFace([i + off for i in f])
            off += len(points)
        ph.Update()
        return ph

    def assertInside(self, ph, points, expected, tol=0):
        for p, e in zip(points, expected):
            with self.subTest(point=list(p)):
                self.assertEqual(ph.IsInside(p, tol), e)

    def cube_test_points(self, lo=0.0, hi=1.0):
        """ Points with what they should be for the cube [lo, hi]^3:
        inside, outside, on its faces, edges and corners. """
        m = (lo + hi) / 2
        d = (hi - lo) * 1e-3
        pts = [([m, m, m], True), ([lo + d, m, m], True), ([hi - d, hi - d, hi - d], True),
               ([lo - d, m, m], False), ([m, hi + d, m], False), ([m, m, hi + 1], False),
               ([hi + d, hi + d, hi + d], False), ([m - 10, m, m], False)]
        for axis in range(3):          # face centres
            for v in (lo, hi):
                p = [m, m, m]
                p[axis] = v
                pts.append((p, True))
        for a, b in ((0, 1), (1, 2), (0, 2)):   # edges, also through a quad's diagonal
            for va, vb in itertools.product((lo, hi), repeat=2):
                p = [lo + (hi - lo) * 0.3] * 3
                p[a], p[b] = va, vb
                pts.append((p, True))
        for c in itertools.product((lo, hi), repeat=3):   # corners
            pts.append((list(c), True))
        pts.append(([lo + (hi - lo) * 0.5, lo + (hi - lo) * 0.5, lo], True))   # on a triangle's diagonal
        return [p for p, _ in pts], [e for _, e in pts]

    # ---- one body

    def test_cube(self):
        pts, exp = self.cube_test_points()
        for name, faces in (('triangles', cube_faces()), ('quads', cube_faces(triangles=False)),
                            ('inward', cube_faces(flip=True))):
            with self.subTest(name):
                self.assertInside(self.poly([(CUBE_POINTS, faces)]), pts, exp)

    def test_cube_one_face_flipped(self):
        # inconsistent orientation: counted crossings decide, the same here
        faces = cube_faces()
        faces[3] = faces[3][::-1]
        pts, exp = self.cube_test_points()
        self.assertInside(self.poly([(CUBE_POINTS, faces)]), pts, exp)

    def test_cube_unmerged_points(self):
        # every face with points of its own, as in a polyhedron written point by point
        points, faces = [], []
        for f in cube_faces():
            faces.append([len(points), len(points) + 1, len(points) + 2])
            points += [CUBE_POINTS[i] for i in f]
        pts, exp = self.cube_test_points()
        self.assertInside(self.poly([(points, faces)]), pts, exp)

    def test_cube_open(self):
        # a face missing: the inside is still found well away from the hole
        faces = cube_faces()[2:]
        ph = self.poly([(CUBE_POINTS, faces)])
        self.assertInside(ph, [[0.5, 0.5, 0.5], [0.3, 0.6, 0.7], [0.5, 0.5, 2], [-1, 0.5, 0.5]],
                          [True, True, False, False])

    def test_ray_through_edge(self):
        # points whose first ray passes exactly through the diagonal of the top face, where two
        # triangles meet: counted once, not twice; with a face flipped, so that the crossings
        # are counted. The first direction as CSPrimPolyhedron.cpp computes it: the first
        # Fibonacci point on a half sphere at 150 degrees, turned by 82 degrees around x
        z = 1 - 0.5 / 7
        r, phi, tilt = np.sqrt(1 - z**2), np.radians(150), np.radians(82)
        y = r * np.sin(phi)
        d = np.array([r * np.cos(phi), y * np.cos(tilt) - z * np.sin(tilt), y * np.sin(tilt) + z * np.cos(tilt)])
        q = np.array([0.4, 0.4, 1.0])   # on the diagonal from (0,0,1) to (1,1,1)
        faces = cube_faces()
        faces[0] = faces[0][::-1]
        ph = self.poly([(CUBE_POINTS, faces)])
        self.assertInside(ph, [q - 0.5 * d], [True])

    def test_degenerate_faces(self):
        # faces of no area change nothing
        faces = cube_faces() + [[0, 0, 1], [2, 2, 2], [3, 3, 1]]
        pts, exp = self.cube_test_points()
        self.assertInside(self.poly([(CUBE_POINTS, faces)]), pts, exp)

    def test_tolerance(self):
        ph = self.poly([(CUBE_POINTS, cube_faces())])
        self.assertFalse(ph.IsInside([1 + 1e-6, 0.5, 0.5]))
        self.assertTrue(ph.IsInside([1 + 1e-6, 0.5, 0.5], tol=1e-5))
        self.assertTrue(ph.IsInside([1 + 1e-6, 1 + 1e-6, 1 + 1e-6], tol=1e-5))   # near a corner
        self.assertFalse(ph.IsInside([1 + 1e-4, 0.5, 0.5], tol=1e-5))
        # single precision coordinates of an STL file, against a double precision mesh line
        # (float32(0.7) is 1.2e-8 below 0.7)
        ph = self.poly([(box_points([0, 0, 0], [0.1, 0.3, 0.7]).astype(np.float32), cube_faces())])
        self.assertFalse(ph.IsInside([0.05, 0.15, 0.7]))
        self.assertTrue(ph.IsInside([0.05, 0.15, 0.7], tol=1e-7))

    def test_transform(self):
        ph = self.poly([(CUBE_POINTS, cube_faces())])
        ph.AddTransform('Translate', [10, 0, 0])
        ph.Update()
        self.assertTrue(ph.IsInside([10.5, 0.5, 0.5]))
        self.assertTrue(ph.IsInside([10, 0.5, 0.5]))
        self.assertFalse(ph.IsInside([0.5, 0.5, 0.5]))

    def test_negative_octant(self):
        # a body with all coordinates <= 0 (the old far point could end up inside it)
        pts, exp = self.cube_test_points(-2.0, -1.0)
        self.assertInside(self.poly([(box_points([-2, -2, -2], [-1, -1, -1]), cube_faces())]), pts, exp)
        pts, exp = self.cube_test_points(-1.0, 0.0)
        self.assertInside(self.poly([(box_points([-1, -1, -1], [0, 0, 0]), cube_faces())]), pts, exp)

    # ---- several bodies in one polyhedron

    def test_touching(self):
        # two cubes sharing a face, with the points they share merged
        a, b = box_points([0, 0, 0], [1, 1, 1]), box_points([1, 0, 0], [2, 1, 1])
        ph = self.poly([(np.vstack([a, b]), cube_faces() + [[i + 8 for i in f] for f in cube_faces()])])
        self.assertInside(ph, [[0.5, 0.5, 0.5], [1.5, 0.5, 0.5], [1, 0.5, 0.5], [1, 0, 0], [2.5, 0.5, 0.5],
                               [1, 1.5, 0.5]],
                          [True, True, True, True, False, False])

    def test_overlapping(self):
        # oriented outward, the overlap is solid
        ph = self.poly([(box_points([0, 0, 0], [2, 2, 2]), cube_faces()),
                        (box_points([1, 1, 1], [3, 3, 3]), cube_faces())])
        self.assertInside(ph, [[0.5] * 3, [1.5] * 3, [2.5] * 3, [3.5] * 3, [0.5, 2.5, 0.5]],
                          [True, True, True, False, False])

    def test_cavity(self):
        # a cube in a cube: a hole if its faces point into it, solid if they point out
        outer = (box_points([0, 0, 0], [3, 3, 3]), cube_faces())
        for flip, cavity in ((True, False), (False, True)):
            with self.subTest(inner_faces_inward=flip):
                ph = self.poly([outer, (box_points([1, 1, 1], [2, 2, 2]), cube_faces(flip=flip))])
                self.assertInside(ph, [[0.5] * 3, [1.5] * 3, [1, 1.5, 1.5], [2.5] * 3, [3.5] * 3],
                                  [True, cavity, True, True, False])

    def test_cavity_inconsistent(self):
        # without a consistent orientation every second region is a hole
        faces = cube_faces()
        faces[0] = faces[0][::-1]
        ph = self.poly([(box_points([0, 0, 0], [3, 3, 3]), faces),
                        (box_points([1, 1, 1], [2, 2, 2]), cube_faces())])
        self.assertInside(ph, [[0.5] * 3, [1.5] * 3, [3.5] * 3], [True, False, False])

    # ---- the same as a box

    def test_same_as_box(self):
        """ Rotated and shifted boxes as polyhedra agree with CSPrimBox: on a
        grid of points through their faces, and on random points off the
        surface. """
        rng = np.random.RandomState(7)
        for n in range(6):
            start = rng.uniform(-3, 0, 3)
            stop = start + rng.uniform(0.5, 3, 3)
            rotate = n > 1
            tr = CSTransform()
            if rotate:
                tr.RotateOrigin(rng.normal(size=3), rng.uniform(0, 360))
                tr.Translate(rng.uniform(-2, 2, 3))
            box = CSPrimitives.CSPrimBox(self.csx.GetParameterSet(), self.metal, start=start, stop=stop)
            ph = self.poly([(box_points(start, stop), cube_faces())])
            if rotate:
                for t in (box, ph):
                    t.GetTransform().SetMatrix(tr.GetMatrix())
                box.Update()
                ph.Update()
            with self.subTest(box=n):
                if not rotate:
                    # mesh lines through the faces, as openEMS has them
                    lines = [np.unique(np.concatenate([np.linspace(start[i] - 1, stop[i] + 1, 9), [start[i], stop[i]]]))
                             for i in range(3)]
                    pts = np.array(list(itertools.product(*lines)))
                else:
                    pts = rng.uniform(-6, 6, size=(400, 3))
                    # only points clearly off the surface: rotated, surface points are rounded either way
                    local = np.array([tr.Transform(p, invers=True) for p in pts])
                    dist = np.min(np.abs(np.concatenate([local - start, local - stop], axis=1)), axis=1)
                    pts = pts[dist > 1e-6]
                for p in pts:
                    self.assertEqual(ph.IsInside(p), box.IsInside(p), list(p))

    # ---- the drawing tolerance reaches the polyhedron

    def test_drawing_tolerance(self):
        ph = self.metal.AddPolyhedron()
        for p in CUBE_POINTS:
            ph.AddVertex(*p)
        for f in cube_faces():
            ph.AddFace(f)
        ph.Update()
        p = [1 + 1e-6, 0.5, 0.5]
        self.assertIsNone(self.csx.GetPropertyByCoordPriority(p))
        self.csx.SetDrawingTolerance(1e-5)
        self.assertEqual(self.csx.GetDrawingTolerance(), 1e-5)
        self.assertEqual(self.csx.GetPropertyByCoordPriority(p).GetName(), 'metal')


if __name__ == '__main__':
    unittest.main()
