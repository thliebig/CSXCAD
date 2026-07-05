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

    def test_symmetric_domain_preserves_fixed_lines(self):
        # Regression test for openEMS-Project issue #470.
        # An (accidentally) symmetric input used to discard the user's fixed
        # lines and rebuild them by mirroring through a floating-point center,
        # perturbing an exact 1.524 into 1.524000000000001. A zero-thickness PEC
        # primitive at 1.524 then coincides with no grid plane and is silently
        # dropped by the solver.
        lines = np.unique([-75.0, 76.524] + np.linspace(0, 1.524, 5).tolist())
        result = SmoothMeshLines(lines, 4.99654097, 1.4)
        self.assertTrue(
            np.any(result == 1.524),
            msg='fixed line 1.524 not preserved, nearest={!r}'.format(
                result[np.argmin(np.abs(result - 1.524))])
        )

    def test_check_symmetry_false_disables_detection(self):
        # check_symmetry=False must smooth the full line set directly instead of
        # mirroring one half, giving a different result for a symmetric input.
        lines = np.unique([-75.0, 76.524] + np.linspace(0, 1.524, 5).tolist())
        detected = SmoothMeshLines(lines, 4.99654097, 1.4)
        full     = SmoothMeshLines(lines, 4.99654097, 1.4, check_symmetry=False)
        self.assertFalse(
            np.array_equal(detected, full),
            msg='check_symmetry=False produced the mirrored (symmetric) result'
        )
        # disabling detection must still respect max_res and keep endpoints
        self.assertTrue(np.all(np.diff(full) <= 4.99654097 * (1 + 1e-10)))
        self.assertEqual(full[0], lines[0])
        self.assertEqual(full[-1], lines[-1])

    def test_result_is_sorted(self):
        lines = [0, 3, 1, 8]
        result = SmoothMeshLines(lines, max_res=2)
        self.assertTrue(np.all(np.diff(result) > 0))

    def test_two_lines_produce_uniform_mesh(self):
        # Two input lines with no neighbouring context must yield a uniform mesh.
        # A floating-point glitch in np.linspace can cause spacings of max_res+eps
        # which, if not tolerated, triggers ceil(1+eps)=2 and splits cells in half.
        for ratio in [1.1, 1.3, 1.5, 2.0]:
            with self.subTest(ratio=ratio):
                result = SmoothMeshLines([0.0, 10.0], max_res=0.1, ratio=ratio)
                dl = np.diff(result)
                self.assertLessEqual(np.max(dl), 0.1 * (1 + 1e-9))
                self.assertTrue(
                    np.allclose(dl, dl[0], rtol=1e-9),
                    msg='mesh not uniform: max={:.6g}, min={:.6g}'.format(np.max(dl), np.min(dl))
                )


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
