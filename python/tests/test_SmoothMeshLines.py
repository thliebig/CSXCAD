import numpy as np

from CSXCAD.SmoothMeshLines import SmoothMeshLines
from CSXCAD import CSRectGrid

import unittest

class Test_SmoothMeshLines(unittest.TestCase):
    def test_no_smoothing_needed(self):
        lines = np.linspace(0, 10, 6)  # spacing = 2, max_res = 3
        result = SmoothMeshLines(lines, max_res=3)
        self.assertTrue((result == lines).all())

    def test_adds_lines_when_gap_too_large(self):
        lines = [0, 10]
        result = SmoothMeshLines(lines, max_res=2)
        self.assertGreater(len(result), 2)
        diffs = np.diff(result)
        self.assertTrue(np.all(diffs <= 2 + 1e-10))

    def test_respects_max_res(self):
        lines = [0, 5, 100]
        max_res = 3.0
        result = SmoothMeshLines(lines, max_res=max_res)
        diffs = np.diff(result)
        self.assertTrue(np.all(diffs <= max_res + 1e-10))

    def test_preserves_endpoints(self):
        lines = [0, 3, 100]
        result = SmoothMeshLines(lines, max_res=5)
        self.assertAlmostEqual(result[0], 0)
        self.assertAlmostEqual(result[-1], 100)

    def test_symmetric_lines_stay_symmetric(self):
        lines = np.array([-50, -10, 0, 10, 50], dtype=float)
        result = SmoothMeshLines(lines, max_res=5)
        center = 0.5 * (result[0] + result[-1])
        N = len(result)
        for n in range(N // 2):
            self.assertAlmostEqual(center - result[n], result[N - n - 1] - center, places=6)

    def test_result_is_sorted(self):
        lines = [0, 3, 1, 8]
        result = SmoothMeshLines(lines, max_res=2)
        self.assertTrue(np.all(np.diff(result) > 0))


class Test_CSRectGrid_Smooth(unittest.TestCase):
    def test_smooth_mesh_lines(self):
        grid = CSRectGrid.CSRectGrid()
        grid.SetLines('x', [0, 10])
        grid.SmoothMeshLines('x', max_res=2)
        self.assertGreater(grid.GetQtyLines('x'), 2)
        lines = grid.GetLines('x', do_sort=True)
        diffs = np.diff(lines)
        self.assertTrue(np.all(diffs <= 2 + 1e-10))

    def test_smooth_does_not_affect_other_dirs(self):
        grid = CSRectGrid.CSRectGrid()
        grid.SetLines('x', [0, 10])
        grid.SetLines('y', [0, 5])
        grid.SetLines('z', [0, 1])
        grid.SmoothMeshLines('x', max_res=2)
        self.assertEqual(grid.GetQtyLines('y'), 2)
        self.assertEqual(grid.GetQtyLines('z'), 2)


if __name__ == '__main__':
    unittest.main()
