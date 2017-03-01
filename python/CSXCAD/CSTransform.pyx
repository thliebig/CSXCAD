# -*- coding: utf-8 -*-
#
# Copyright (C) 2015,20016 Thorsten Liebig (Thorsten.Liebig@gmx.de)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

"""
Affine Transformations for primitives
"""

import numpy as np
from ParameterObjects cimport _ParameterSet, ParameterSet
cimport CSTransform
from Utilities import CheckNyDir

cdef class CSTransform:
    """
    This class can realize affine transformations for all CSPrimtivies.
    Individual tranformations can be concatenated to the previous. As a result
    the order of transformations is important.
    """
    def __cinit__(self,  ParameterSet pset=None, no_init=False, **kw):
        if no_init==False:
            if pset is None:
                self.thisptr = new _CSTransform()
            else:
                self.thisptr = new _CSTransform(pset.thisptr)
            self.thisptr.SetAngleRadian()
        else:
            self.thisptr = NULL

    def Reset(self):
        """
        Reset all transformations.
        """
        self.thisptr.Reset()

    def HasTransform(self):
        """
        Check if any transformations are set.
        """
        return self.thisptr.HasTransform()

    def Transform(self, coord, invers=False):
        """ Transform(coord, invers)

        Apply a transformation to the given coordinate.

        :param coord: (3,) array -- coordinate to transform
        :param invers: bool -- do an invers transformation
        :returns: (3,) array -- transformed coordinates
        """
        cdef double[3] d_coord
        cdef double[3] d_out
        for n in range(3):
            d_coord[n] = coord[n]

        if not invers:
            self.thisptr.Transform(d_coord, d_out)
        else:
            self.thisptr.InvertTransform(d_coord, d_out)
        out = np.zeros((3,))
        for n in range(3):
            out[n] = d_out[n]
        return out

    def GetMatrix(self):
        """
        Get the full 4x4 transformation matrix used for transformation.

        :returns: (4,4) array -- transformation matrix
        """
        cdef double *d_mat = NULL
        d_mat = self.thisptr.GetMatrix()
        mat = np.zeros([4,4])
        for n in range(4):
            for m in range(4):
                mat[n][m] = d_mat[4*n + m]

        return mat

    def AddTransform(self, transform, *args, **kw):
        """ AddTransform(transform, *args, **kw)

        Add a transform by name and arguments.

        Examples
        --------
        Add a translation and 30° rotation around the z-axis:

        >>> tr = CSTransform()
        >>> tr.AddTransform('Translate', [10, 4,6])
        >>> tr.AddTransform('RotateAxis', 'z', 30)

        Add a rotation around the axis=[1, 1, 0] by pi/3 (30°):

        >>> tr = CSTransform()
        >>> tr.AddTransform('RotateOrigin', [1, 1, 0], np.pi/3, deg=False)

        Available Transformation keywords:

        * RotateAxis : Rotate around x,y or z-axis
        * RotateOrigin : Rotate around a given axis
        * Translate : Translation vector
        * Scale : Scale value or vector
        * Matrix : A affine transformation matrix as (4,4) array

        :param transform: str -- transformation name or keyword.
        """
        assert type(transform)==str, 'AddTransform, transform name must be a string'
        if transform == 'RotateAxis':
            self.RotateAxis(*args, **kw)
        elif transform == 'RotateOrigin':
            self.RotateOrigin(*args, **kw)
        elif transform == 'Translate':
            self.Translate(*args, **kw)
        elif transform  == 'Scale':
            self.Scale(*args, **kw)
        elif transform  in ['Matrix', 'SetMatrix']:
            self.SetMatrix(*args, **kw)
        else:
            raise Exception('AddTransform: Unkown transformation "{}"'.format(transform))

    def Translate(self, vec, concatenate=True):
        """ Translate(vec, concatenate=True)

        Add a trsanslation transformation.

        :param vec: (3,) array -- translation vector
        """
        assert len(vec)==3, 'Translate vector must be a list or array of length 3'
        cdef double[3] d_vec;
        for n in range(3):
            d_vec[n] = vec[n]
        self.thisptr.Translate(d_vec, concatenate)


    def SetMatrix(self, mat, concatenate=True):
        """ SetMatrix(mat, concatenate=True)

        Add an arbitrary (invertable) trsanslation matrix.

        :param mat: (3,) array -- translation vector
        """
        mat = np.array(mat)
        assert  mat.shape == (4,4), 'SetMatrix: matrix must be of shape (4,4)'
        cdef double[16] d_mat;
        for n in range(4):
            for m in range(4):
                d_mat[4*n + m] = mat[n][m]
        self.thisptr.SetMatrix(d_mat, concatenate)

    def RotateOrigin(self, vec, angle, deg=True, concatenate=True):
        """ RotateOrigin(vec, angle, deg=True, concatenate=True)

        Add a rotation transformation around an arbitrary axis.

        :param vec: (3,) array -- translation axis vector.
        :param angle: float -- rotation angle (default in degrees)
        :param deg: bool -- set degree or radiant for angle (default True)
        """
        assert  len(vec)==3, 'RotateOrigin: axis vector must be a list or array of length 3'
        cdef double[3] d_vec;
        for n in range(3):
            d_vec[n] = vec[n]
        if deg:
            angle = np.deg2rad(angle)
        self.thisptr.RotateOrigin(d_vec, angle, concatenate)

    def RotateAxis(self, ny, angle, deg=True, concatenate=True):
        """ RotateAxis(ny, angle, deg=True, concatenate=True)

        Add a rotation transformation around a cartesian axis (x,y or z).

        :param ny: int or str -- translation axis vector 0/1/2 or 'x'/'y'/'z.
        :param angle: float -- rotation angle (default in degrees)
        :param deg: bool -- set degree or radiant for angle (default True)
        """
        ny = CheckNyDir(ny)
        if deg:
            angle = np.deg2rad(angle)
        self.thisptr.RotateXYZ(ny, angle, concatenate)

    def Scale(self, scale, bool concatenate=True):
        """ Scale(scale, concatenate=True)

        Add a scaleing transformation.

        :param scale: float or (3,) array -- Scaling factor
        """
        cdef double[3] d_scale;
        if type(scale)==float or type(scale)==int:
            d_scale[0] = scale
            self.thisptr.Scale(d_scale[0], concatenate)
            return

        assert len(scale)==3, 'Scale: scale must be a float or array of length 3'
        for n in range(3):
            d_scale[n] = scale[n]
        self.thisptr.Scale(d_scale, concatenate)

    def SetPreMultiply(self):
        """
        Set all following transformations as pre multiply (default is post multiply)
        """
        self.thisptr.SetPreMultiply()

    def SetPostMultiply(self):
        """
        Set all following transformations as post multiply (default)
        """
        self.thisptr.SetPostMultiply()

