# -*- coding: utf-8 -*-
"""
Created on Fri Sep  2 21:42:50 2016

@author: thorsten
"""

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

if __name__ == '__main__':
    unittest.main()
