"""Tests for the tolerance of IsInside: a point within the tolerance of a
primitive's surface counts as inside, as does a point on the surface itself.

Every shape is checked at points of its surface, with the outward direction
there: on the surface (where that point is exact in floating point), a
little inside, half the tolerance outside (inside only with the tolerance)
and twice the tolerance outside (never inside).
"""
import unittest

import numpy as np

from CSXCAD import ContinuousStructure

TOL = 1e-6


def unit(v):
    v = np.asarray(v, float)
    return v / np.linalg.norm(v)


class Test_InsideTolerance(unittest.TestCase):
    def setUp(self):
        self.csx = ContinuousStructure()
        self.metal = self.csx.AddMetal('metal')

    def assertSurface(self, prim, point, normal, exact=True, solid=True):
        """ point on the surface of prim, normal pointing out of it; exact if the point lies on
        the surface in floating point too, not only up to rounding; not solid for a sheet seen
        from its side, with nothing inside along the normal """
        p, n = np.asarray(point, float), unit(normal)
        with self.subTest(point=list(p), normal=list(np.round(n, 3))):
            if exact:
                self.assertTrue(prim.IsInside(p), 'on the surface')
            if solid:
                self.assertTrue(prim.IsInside(p - 1e-4 * n), 'a little inside')
            self.assertFalse(prim.IsInside(p + 0.5 * TOL * n), 'half the tolerance outside, without it')
            self.assertTrue(prim.IsInside(p + 0.5 * TOL * n, TOL), 'half the tolerance outside')
            self.assertFalse(prim.IsInside(p + 2 * TOL * n, TOL), 'twice the tolerance outside')

    def assertTransformed(self, prim, point, normal):
        """ as assertSurface, for a point and normal in the primitive's own coordinates """
        tr = prim.GetTransform()
        p = np.asarray(tr.Transform(point))
        self.assertSurface(prim, p, np.asarray(tr.Transform(np.add(point, normal))) - p, exact=False)

    # ---- boxes

    def test_box(self):
        for start, stop in (([0, 0, 0], [1, 2, 3]), ([1, 2, 3], [0, 0, 0])):
            box = self.metal.AddBox(start, stop)
            for axis in range(3):
                for side in (0, 1):
                    p, n = [0.5, 1, 1.5], [0, 0, 0]
                    p[axis] = [0, [1, 2, 3][axis]][side]
                    n[axis] = [-1, 1][side]
                    self.assertSurface(box, p, n)
            self.assertSurface(box, [1, 2, 1.5], [1, 1, 0])     # an edge
            self.assertSurface(box, [1, 2, 3], [1, 1, 1])       # a corner
            self.assertSurface(box, [0, 0, 0], [-1, -1, -1])

    def test_box_transformed(self):
        box = self.metal.AddBox([0, 0, 0], [1, 1, 1])
        box.AddTransform('RotateOrigin', [1, 2, 3], 30)
        box.AddTransform('Translate', [-1, 2, 0.5])
        box.Update()
        self.assertTransformed(box, [1, 0.5, 0.5], [1, 0, 0])
        self.assertTransformed(box, [0.5, 0.5, 0], [0, 0, -1])
        self.assertTransformed(box, [1, 1, 1], [1, 1, 1])

    def test_box_scaled(self):
        # the tolerance is a length in the global coordinates, also when the primitive is scaled
        for scale in (10, 0.1):
            with self.subTest(scale=scale):
                box = self.metal.AddBox([0, 0, 0], [1, 1, 1])
                box.AddTransform('Scale', scale)
                box.Update()
                self.assertTransformed(box, [1, 0.5, 0.5], [1, 0, 0])

    def test_box_cylindrical(self):
        # a box in cylindrical coordinates on a Cartesian mesh: the tolerance along the angle is a length too
        box = self.metal.AddBox([1, 0, 0], [2, 0.5, 1])
        box.SetCoordinateSystem(1)
        box.Update()
        a = 0.5
        self.assertSurface(box, [1.5 * np.cos(a), 1.5 * np.sin(a), 0.5], [-np.sin(a), np.cos(a), 0], exact=False)
        self.assertSurface(box, [1.5, 0, 0.5], [0, -1, 0])
        self.assertSurface(box, [2 * np.cos(0.25), 2 * np.sin(0.25), 0.5], [np.cos(0.25), np.sin(0.25), 0], exact=False)
        self.assertSurface(box, [1.5 * np.cos(0.25), 1.5 * np.sin(0.25), 1], [0, 0, 1], exact=False)

    def test_box_cylindrical_mesh(self):
        # on a cylindrical mesh the coordinates are (r, a, z); set after the box, as openEMS does
        box = self.metal.AddBox([1, 0, 0], [2, 0.5, 1])
        self.csx.SetMeshType(1)
        for r, da in ((1.5, 0), (1, 0), (2, 0)):
            with self.subTest(r=r):
                self.assertTrue(box.IsInside([r, 0.5, 0.5]))
                self.assertFalse(box.IsInside([r, 0.5 + 0.5 * TOL / r, 0.5]))
                self.assertTrue(box.IsInside([r, 0.5 + 0.5 * TOL / r, 0.5], TOL))
                self.assertFalse(box.IsInside([r, 0.5 + 2 * TOL / r, 0.5], TOL))
        self.assertTrue(box.IsInside([2 + 0.5 * TOL, 0.25, 0.5], TOL))
        self.assertFalse(box.IsInside([2 + 2 * TOL, 0.25, 0.5], TOL))
        self.assertTrue(box.IsInside([1.5, 0.25, -0.5 * TOL], TOL))

    def test_multibox(self):
        mbox = self.metal.AddMultiBox([([0, 0, 0], [1, 1, 1]), ([3, 1, 1], [2, 0, 0])])
        self.assertSurface(mbox, [1, 0.5, 0.5], [1, 0, 0])
        self.assertSurface(mbox, [2, 0.5, 0.5], [-1, 0, 0])
        self.assertSurface(mbox, [3, 1, 1], [1, 1, 1])

    # ---- round primitives

    def test_cylinder(self):
        cyl = self.metal.AddCylinder([0, 0, 0], [0, 0, 2], 1)
        self.assertSurface(cyl, [1, 0, 1], [1, 0, 0])
        self.assertSurface(cyl, [0, -1, 1], [0, -1, 0])
        self.assertSurface(cyl, [0.3, 0, 2], [0, 0, 1])
        self.assertSurface(cyl, [0.3, 0, 0], [0, 0, -1])
        self.assertSurface(cyl, [1, 0, 2], [1, 0, 1])         # the rim
        # an oblique axis
        axis = unit([1, 2, 2])
        cyl = self.metal.AddCylinder([0, 0, 0], 3 * axis, 0.5)
        side = unit(np.cross(axis, [1, 0, 0]))
        self.assertSurface(cyl, 1.5 * axis + 0.5 * side, side, exact=False)
        self.assertSurface(cyl, 3 * axis + 0.2 * side, axis, exact=False)

    def test_cylindrical_shell(self):
        shell = self.metal.AddCylindricalShell([0, 0, 0], [0, 0, 2], 1, 0.5)
        self.assertSurface(shell, [1.25, 0, 1], [1, 0, 0])
        self.assertSurface(shell, [0.75, 0, 1], [-1, 0, 0])
        self.assertSurface(shell, [1, 0, 2], [0, 0, 1])

    def test_sphere(self):
        sphere = self.metal.AddSphere([1, 2, 3], 2)
        self.assertSurface(sphere, [3, 2, 3], [1, 0, 0])
        self.assertSurface(sphere, [1, 2, 1], [0, 0, -1])
        for n in np.random.RandomState(3).normal(size=(5, 3)):
            n = unit(n)
            self.assertSurface(sphere, np.array([1, 2, 3]) + 2 * n, n, exact=False)

    def test_spherical_shell(self):
        shell = self.metal.AddSphericalShell([0, 0, 0], 2, 1)
        self.assertSurface(shell, [2.5, 0, 0], [1, 0, 0])
        self.assertSurface(shell, [0, -1.5, 0], [0, 1, 0])

    def test_sphere_scaled(self):
        sphere = self.metal.AddSphere([0, 0, 0], 1)
        sphere.AddTransform('Scale', 10)
        sphere.Update()
        self.assertTransformed(sphere, [0, 0, 1], [0, 0, 1])

    def test_wire(self):
        wire = self.metal.AddWire([[0, 1, 1], [0, 0, 1], [0, 0, 0]], 0.25)
        self.assertSurface(wire, [0.5, 0.25, 0], [0, 1, 0])
        self.assertSurface(wire, [0.5, 0, -0.25], [0, 0, -1])
        self.assertSurface(wire, [-0.25, 0, 0], [-1, 0, 0])   # the round end
        self.assertSurface(wire, [1, 1.25, 0], [0, 1, 0])

    # ---- polygons

    def test_polygon(self):
        # a triangle in the plane z=1, a sheet without thickness
        poly = self.metal.AddPolygon([[0, 1, 0], [0, 0, 1]], 'z', 1)
        self.assertSurface(poly, [0.2, 0.2, 1], [0, 0, 1], solid=False)
        self.assertSurface(poly, [0.2, 0.2, 1], [0, 0, -1], solid=False)
        self.assertSurface(poly, [0.5, 0, 1], [0, -1, 0])
        self.assertSurface(poly, [0, 0.5, 1], [-1, 0, 0])
        self.assertSurface(poly, [0.5, 0.5, 1], [1, 1, 0])    # the slanted edge
        self.assertSurface(poly, [1, 0, 1], [1, -1, 0])       # a corner
        self.assertSurface(poly, [0, 1, 1], [-1, 1, 1], solid=False)

    def test_linpoly(self):
        poly = self.metal.AddLinPoly([[0, 1, 0], [0, 0, 1]], 'z', 1, 2)
        self.assertSurface(poly, [0.2, 0.2, 3], [0, 0, 1])
        self.assertSurface(poly, [0.2, 0.2, 1], [0, 0, -1])
        self.assertSurface(poly, [0.5, 0, 2], [0, -1, 0])
        self.assertSurface(poly, [0.5, 0.5, 2], [1, 1, 0])
        self.assertSurface(poly, [1, 0, 3], [1, -1, 1])

    def test_linpoly_transformed(self):
        poly = self.metal.AddLinPoly([[0, 1, 0], [0, 0, 1]], 'z', 1, 2)
        poly.AddTransform('RotateOrigin', [1, 0, 1], 40)
        poly.AddTransform('Scale', 3)
        poly.Update()
        self.assertTransformed(poly, [0.5, 0.5, 2], [1, 1, 0])
        self.assertTransformed(poly, [0.2, 0.2, 3], [0, 0, 1])

    def test_rotpoly(self):
        # a rectangle (z 0..1, x 1..2) turned around z from 0 to 90 degrees: a quarter of a ring
        poly = self.metal.AddRotPoly([[0, 1, 1, 0], [1, 1, 2, 2]], 'y', 0, 'z', [0, np.pi / 2])
        a = 0.3
        self.assertSurface(poly, [2 * np.cos(a), 2 * np.sin(a), 0.5], [np.cos(a), np.sin(a), 0], exact=False)
        self.assertSurface(poly, [1 * np.cos(a), 1 * np.sin(a), 0.5], [-np.cos(a), -np.sin(a), 0], exact=False)
        self.assertSurface(poly, [1.5 * np.cos(a), 1.5 * np.sin(a), 1], [0, 0, 1], exact=False)
        self.assertSurface(poly, [1.5, 0, 0.5], [0, -1, 0])                # the start angle
        self.assertSurface(poly, [0, 1.5, 0.5], [-1, 0, 0])                # the stop angle
        self.assertSurface(poly, [0, 2, 1], [-1, 1, 1])

    # ---- polyhedron, and through ContinuousStructure

    def test_polyhedron_scaled(self):
        ph = self.metal.AddPolyhedron()
        for p in np.array([[x, y, z] for z in (0, 1) for y in (0, 1) for x in (0, 1)], float):
            ph.AddVertex(*p)
        for q in [[0, 2, 3, 1], [4, 5, 7, 6], [0, 1, 5, 4], [2, 6, 7, 3], [0, 4, 6, 2], [1, 3, 7, 5]]:
            ph.AddFace(q)
        ph.AddTransform('Scale', 10)
        ph.Update()
        self.assertTransformed(ph, [1, 0.5, 0.5], [1, 0, 0])

    def test_drawing_tolerance(self):
        # the tolerance ContinuousStructure passes on, as openEMS sets it from the mesh
        self.metal.AddBox([0, 0, 0], [1, 1, 1])
        p = [1 + 0.5 * TOL, 0.5, 0.5]
        self.assertIsNone(self.csx.GetPropertyByCoordPriority(p))
        self.csx.SetDrawingTolerance(TOL)
        self.assertEqual(self.csx.GetPropertyByCoordPriority(p).GetName(), 'metal')
        self.assertIsNone(self.csx.GetPropertyByCoordPriority([1 + 2 * TOL, 0.5, 0.5]))


if __name__ == '__main__':
    unittest.main()
