# -*- coding: utf-8 -*-
"""
Created on Sat Dec 12 22:04:58 2015

@author: thorsten
"""

import numpy as np

from CSXCAD import CSRectGrid

import unittest

class Test_CSRectGrid(unittest.TestCase):
    def test_rect_grid(self):

        grid = CSRectGrid.CSRectGrid(CoordSystem=0)

        self.assertEqual( grid.GetMeshType(), 0 )

        grid.SetMeshType(1)
        self.assertEqual( grid.GetMeshType(), 1 )

        grid.SetLines('x', [0, 1, 2])
        grid.SetLines('y', [-2,0, 1])
        self.assertEqual( grid.GetQtyLines('y'), 3 )
        grid.AddLine('y',4)
        self.assertEqual( grid.GetQtyLines('y'), 4 )
        grid.AddLine('y',[4, 2, 5])
        self.assertEqual( grid.GetQtyLines('y'), 7 )
        self.assertTrue( (grid.GetLines('y')==np.array([-2.,  0.,  1.,  4.,  4., 2.,  5.])).all() )  # check unsorted lines
        grid.Sort('y')
        self.assertTrue( (grid.GetLines('y')==np.array([-2.,  0.,  1.,  2.,  4.,  5.])).all() )
        self.assertEqual( grid.GetQtyLines('y'), 6 )

        grid.SetLines('z', [10, 11, 12])

        self.assertEqual( grid.GetLine('y', 1), 0.0 )

        self.assertEqual( grid.GetQtyLines('x'), 3 )

        grid.SetDeltaUnit(1e-3)
        self.assertEqual( grid.GetDeltaUnit(), 1e-3 )

        self.assertTrue( grid.IsValid() )

        # check grid snapping
        self.assertEqual(  grid.Snap2LineNumber('y', 1), (2,True) )
        self.assertEqual(  grid.Snap2LineNumber('y', 1.1), (2,True) )
        self.assertEqual(  grid.Snap2LineNumber('y', 1.5), (3,True) )
        self.assertEqual(  grid.Snap2LineNumber('y', 1.6), (3,True) )
        self.assertEqual(  grid.Snap2LineNumber('y', 5.0), (5,True) )
        self.assertEqual(  grid.Snap2LineNumber('y', 5.01), (5,False) )

        self.assertEqual(  grid.Snap2LineNumber('y', -2.0), (0,True) )
        self.assertEqual(  grid.Snap2LineNumber('y', -2.01), (0,False) )

        self.assertTrue( (grid.GetSimArea() == np.array([[0, -2, 10],[2, 5, 12]])).all() )

        grid.ClearLines('x')
        self.assertEqual( grid.GetQtyLines('x'), 0 )
        self.assertEqual( grid.GetQtyLines('y'), 6 )
        self.assertEqual( grid.GetQtyLines('z'), 3 )

        self.assertFalse( grid.IsValid() )

        grid.Clear()
        self.assertEqual( grid.GetQtyLines('y'), 0 )

        self.assertFalse( grid.IsValid() )

if __name__ == '__main__':
    unittest.main()