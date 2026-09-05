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
        grid.AddLine('y',[4, 2, 5])   # 4 is already there and is dropped
        self.assertEqual( grid.GetQtyLines('y'), 6 )
        self.assertTrue( (grid.GetLines('y')==np.array([-2.,  0.,  1.,  2.,  4.,  5.])).all() )
        grid.Sort('y')                # deprecated, must not change anything
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

    def test_get_line_index(self):
        grid = CSRectGrid.CSRectGrid()
        grid.SetLines('x', [0, 1, 2])
        grid.SetLines('y', [-2, 0, 2])
        grid.SetLines('z', [10, 11])

        self.assertEqual( grid.GetLine('x', 0), 0.0 )
        self.assertEqual( grid.GetLine('x', 2), 2.0 )

        # negative indices count from the end
        self.assertEqual( grid.GetLine('x', -1), 2.0 )
        self.assertEqual( grid.GetLine('x', -3), 0.0 )
        self.assertEqual( grid.GetLine('z', -1), 11.0 )

        # out of range must raise, not return 0.0 (a valid line position!)
        self.assertRaises( IndexError, grid.GetLine, 'x',  3 )
        self.assertRaises( IndexError, grid.GetLine, 'x',  4 )
        self.assertRaises( IndexError, grid.GetLine, 'x', -4 )
        self.assertRaises( IndexError, grid.GetLine, 'z',  2 )
        # way beyond the size_t range
        self.assertRaises( IndexError, grid.GetLine, 'x', 2**80 )

        # an empty direction has no valid index at all
        grid.ClearLines('y')
        self.assertRaises( IndexError, grid.GetLine, 'y',  0 )
        self.assertRaises( IndexError, grid.GetLine, 'y', -1 )

        # non-integer indices are still rejected
        self.assertRaises( TypeError, grid.GetLine, 'x', 1.5 )
        self.assertRaises( TypeError, grid.GetLine, 'x', None )

        # numpy integers are accepted
        self.assertEqual( grid.GetLine('x', np.int32(1)), 1.0 )

    def test_lines_always_sorted_and_unique(self):
        # lines are kept sorted and unique, no matter in which order they are
        # added, index 0 is always the smallest line
        grid = CSRectGrid.CSRectGrid()
        grid.SetLines('x', [5, 0, 2])

        self.assertTrue( (grid.GetLines('x')==np.array([0., 2., 5.])).all() )
        self.assertEqual( grid.GetLine('x',  0), 0.0 )
        self.assertEqual( grid.GetLine('x', -1), 5.0 )
        self.assertEqual( grid.GetQtyLines('x'), 3 )
        self.assertRaises( IndexError, grid.GetLine, 'x', 3 )

        # the do_sort argument is deprecated and ignored
        self.assertTrue( (grid.GetLines('x', do_sort=False)==np.array([0., 2., 5.])).all() )

        # an out of order AddLine is sorted in
        grid.AddLine('x', 1)
        self.assertTrue( (grid.GetLines('x')==np.array([0., 1., 2., 5.])).all() )
        self.assertEqual( grid.GetLine('x', 1), 1.0 )

        # an increasing AddLine is appended
        grid.AddLine('x', 9)
        self.assertEqual( grid.GetLine('x', -1), 9.0 )

        # duplicates are dropped, both on the append and on the sort path
        grid.AddLine('x', 9)          # == last line
        grid.AddLine('x', 1)          # somewhere in the middle
        self.assertEqual( grid.GetQtyLines('x'), 5 )
        self.assertTrue( (grid.GetLines('x')==np.array([0., 1., 2., 5., 9.])).all() )

        # duplicates in the initial list are dropped as well
        grid.SetLines('y', [2, 0, 2, 0])
        self.assertEqual( grid.GetQtyLines('y'), 2 )
        self.assertTrue( (grid.GetLines('y')==np.array([0., 2.])).all() )

        # GetQtyLines must see a pending sort, not the raw insert count
        grid.SetLines('z', [3, 1])
        grid.AddLine('z', 1)
        self.assertEqual( grid.GetQtyLines('z'), 2 )

    def test_snap_to_unsorted_input(self):
        # Snap2LineNumber relies on the lines being ordered, it used to return
        # garbage for lines that were added out of order
        grid = CSRectGrid.CSRectGrid()
        grid.SetLines('x', [10, 0, 5])
        self.assertEqual( grid.Snap2LineNumber('x',  -1), (0, False) )
        self.assertEqual( grid.Snap2LineNumber('x', 0.1), (0, True)  )
        self.assertEqual( grid.Snap2LineNumber('x', 4.9), (1, True)  )
        self.assertEqual( grid.Snap2LineNumber('x',  11), (2, False) )

    def test_sim_area_unsorted_input(self):
        grid = CSRectGrid.CSRectGrid()
        grid.SetLines('x', [5, -1, 2])
        grid.SetLines('y', [0, 1])
        grid.SetLines('z', [7, 3])
        self.assertTrue( (grid.GetSimArea()==np.array([[-1., 0., 3.],[5., 1., 7.]])).all() )

if __name__ == '__main__':
    unittest.main()
