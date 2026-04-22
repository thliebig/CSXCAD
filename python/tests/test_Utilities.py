from CSXCAD.Utilities import CheckNyDir, GetMultiDirs

import unittest

class Test_Utilities(unittest.TestCase):
    def test_check_ny_dir_int(self):
        self.assertEqual(CheckNyDir(0), 0)
        self.assertEqual(CheckNyDir(1), 1)
        self.assertEqual(CheckNyDir(2), 2)

    def test_check_ny_dir_cartesian(self):
        self.assertEqual(CheckNyDir('x'), 0)
        self.assertEqual(CheckNyDir('y'), 1)
        self.assertEqual(CheckNyDir('z'), 2)

    def test_check_ny_dir_cylindrical(self):
        self.assertEqual(CheckNyDir('r'), 0)
        self.assertEqual(CheckNyDir('a'), 1)

    def test_check_ny_dir_invalid(self):
        with self.assertRaises(ValueError):
            CheckNyDir('q')
        with self.assertRaises(ValueError):
            CheckNyDir(3)
        with self.assertRaises(ValueError):
            CheckNyDir(-1)

    def test_get_multi_dirs_single(self):
        self.assertEqual(GetMultiDirs('x'), [0])
        self.assertEqual(GetMultiDirs('y'), [1])
        self.assertEqual(GetMultiDirs('z'), [2])
        self.assertEqual(GetMultiDirs('r'), [0])
        self.assertEqual(GetMultiDirs('a'), [1])

    def test_get_multi_dirs_pairs(self):
        self.assertEqual(GetMultiDirs('xy'), [0, 1])
        self.assertEqual(GetMultiDirs('yz'), [1, 2])
        self.assertEqual(GetMultiDirs('xz'), [0, 2])
        self.assertEqual(GetMultiDirs('ra'), [0, 1])
        self.assertEqual(GetMultiDirs('yx'), [0, 1])  # order-independent

    def test_get_multi_dirs_all(self):
        self.assertEqual(GetMultiDirs('xyz'), [0, 1, 2])
        self.assertEqual(GetMultiDirs('all'), [0, 1, 2])
        self.assertEqual(GetMultiDirs('raz'), [0, 1, 2])

    def test_get_multi_dirs_type_error(self):
        with self.assertRaises(AssertionError):
            GetMultiDirs(0)
        with self.assertRaises(AssertionError):
            GetMultiDirs(['x', 'y'])


if __name__ == '__main__':
    unittest.main()
