from CSXCAD import CSTransform
import numpy as np

import unittest

def compare_coords(c1, c2):
    c1 = np.array(c1)
    c2 = np.array(c2)
    return np.sum(np.abs(c1-c2))<1e-6


class Test_CSTransform(unittest.TestCase):
    def setUp(self):
        self.tr = CSTransform.CSTransform()

    def test_basics(self):
        identity = np.identity(4)

        self.assertFalse( self.tr.HasTransform() )
        self.assertTrue( (self.tr.GetMatrix()==identity).all() )

        self.tr.Translate([1,2,1])
        self.assertTrue( self.tr.HasTransform() )
        self.assertFalse((self.tr.GetMatrix()==identity).all() )

        self.tr.Reset()
        self.assertFalse( self.tr.HasTransform() )
        self.assertTrue( (self.tr.GetMatrix()==identity).all() )

    def test_translate(self):
        test_pnt = np.array([1,1,1])/np.sqrt(3)

        self.tr.Translate([1,2,1])
        translate = np.identity(4)
        translate[:3,3] = [1,2,1]
        self.assertTrue( (self.tr.GetMatrix()==translate).all() )

        self.assertTrue( compare_coords(self.tr.Transform(test_pnt), [ 1.57735027,  2.57735027,  1.57735027]) )

    def test_rotate(self):
        self.tr.RotateAxis('x', 45)
        self.assertTrue( compare_coords(self.tr.Transform([0,np.sqrt(2),0]), [0, 1, 1]) )

        self.tr.Reset()
        self.tr.RotateAxis('x', -45)
        self.assertTrue( compare_coords(self.tr.Transform([0,np.sqrt(2),0]), [0, 1, -1]) )

        self.tr.Reset()
        self.tr.RotateAxis('z', -90)
        self.assertTrue( compare_coords(self.tr.Transform([1,0,0]), [0, -1, 0]) )
        self.tr.RotateOrigin([0,0,1], 90)
        self.assertTrue( compare_coords(self.tr.Transform([1,0,0]), [1, 0, 0]) )

        self.tr.Reset()
        self.tr.RotateOrigin([0,1,1], 90)
        self.assertTrue( compare_coords(self.tr.Transform([1,0,0]), [0, 1/np.sqrt(2), -1/np.sqrt(2)]) )

    def test_scale(self):
        self.tr.Scale(3)
        self.assertTrue( compare_coords(self.tr.Transform([1,0,0]), [3, 0, 0]) )

        self.tr.Reset()
        self.tr.Scale([1,2,3])
        self.assertTrue( compare_coords(self.tr.Transform([1,1,1]), [1, 2, 3]) )

    def test_matrix(self):
        self.tr.RotateOrigin([1,1,1], 90)
        mat = self.tr.GetMatrix()
        self.tr.Reset()
        self.tr.SetMatrix(mat)
        self.assertTrue( (self.tr.GetMatrix()==mat).all() )

    def test_transform_list(self):
        self.assertEqual(self.tr.GetTransformList(), [])
        self.tr.RotateAxis('x', 30)
        self.tr.Translate([1, 2.5, 0])
        self.tr.Scale(2)
        names = [t[0] for t in self.tr.GetTransformList()]
        self.assertEqual(names, ['Rotate_X', 'Translate', 'Scale'])
        self.assertEqual([float(a) for a in self.tr.GetTransformList()[1][1]], [1, 2.5, 0])
        self.tr.Reset()
        self.assertEqual(self.tr.GetTransformList(), [])

    def test_invers_transform(self):
        self.tr.Translate([3, 0, 0])
        fwd = self.tr.Transform([1, 0, 0])
        self.assertTrue(compare_coords(fwd, [4, 0, 0]))

        inv = self.tr.Transform([4, 0, 0], invers=True)
        self.assertTrue(compare_coords(inv, [1, 0, 0]))

        self.tr.Reset()
        self.tr.RotateAxis('z', 90)
        fwd = self.tr.Transform([1, 0, 0])
        self.assertTrue(compare_coords(fwd, [0, 1, 0]))
        inv = self.tr.Transform([0, 1, 0], invers=True)
        self.assertTrue(compare_coords(inv, [1, 0, 0]))

    def test_invers_transform_general(self):
        # the inverse is the full 4x4 inverse, its first three rows applied to (x, y, z, 1)
        rng = np.random.RandomState(3)
        pnts = rng.uniform(-5, 5, size=(10, 3))
        shear = np.array([[1, 0.5, 0, 1], [0, 1, -0.3, 2], [0.2, 0, 1, 3], [0, 0, 0, 1]])
        full = rng.uniform(-1, 1, size=(4, 4)) + 3*np.identity(4)

        def composite(tr):
            tr.RotateOrigin([1, 2, 3], 37)
            tr.Scale([2, 0.5, 3])
            tr.Translate([-1, 4, 2])
            tr.RotateAxis('y', -70)

        cases = [('composite', composite, True),
                 ('shear', lambda tr: tr.SetMatrix(shear), True),
                 ('full 4x4', lambda tr: tr.SetMatrix(full), False)]
        for name, setup, affine in cases:
            with self.subTest(name):
                self.tr.Reset()
                setup(self.tr)
                inv = np.linalg.inv(self.tr.GetMatrix())
                for p in pnts:
                    np.testing.assert_allclose(self.tr.Transform(p, invers=True), (inv @ np.append(p, 1))[:3],
                                               rtol=1e-12, atol=1e-12)
                    if affine:
                        np.testing.assert_allclose(self.tr.Transform(self.tr.Transform(p), invers=True), p,
                                                   rtol=1e-12, atol=1e-12)

    def test_multiply_order(self):
        self.tr.Translate([1, 0, 0])

        self.tr.SetPreMultiply()
        self.tr.RotateAxis('z', 90)
        # pre-multiply: rotation applied before translation → rotate [1,0,0] → [0,1,0] then translate → [1,1,0]
        self.assertTrue(compare_coords(self.tr.Transform([1, 0, 0]), [1, 1, 0]))

        self.tr.Reset()
        self.tr.Translate([1, 0, 0])
        self.tr.SetPostMultiply()
        self.tr.RotateAxis('z', 90)
        # post-multiply: translation applied before rotation → translate [1,0,0] → [2,0,0] then rotate → [0,2,0]
        self.assertTrue(compare_coords(self.tr.Transform([1, 0, 0]), [0, 2, 0]))

    def test_copy_is_independent(self):
        self.tr.Translate([1, 2, 3])
        t2 = self.tr.copy()
        # same matrix values
        self.assertTrue((t2.GetMatrix() == self.tr.GetMatrix()).all())
        # distinct Python and C++ objects
        self.assertIsNot(t2, self.tr)
        # modifying the copy does not affect the original
        t2.Translate([10, 0, 0])
        self.assertFalse((t2.GetMatrix() == self.tr.GetMatrix()).all())


if __name__ == '__main__':
    unittest.main()
