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

from libcpp.string cimport string
from libcpp cimport bool
from ParameterObjects cimport _ParameterSet

cdef extern from "CSXCAD/CSTransform.h":
        cdef cppclass _CSTransform "CSTransform":
            _CSTransform() except +
            _CSTransform(_ParameterSet*) except +

            void Reset()

            double* Transform(double *inCoord, double *outCoord)
            double* InvertTransform(double *inCoord, double *outCoord)

            double* GetMatrix()

            bool HasTransform()

            void Translate(double *translate, bool concatenate)

            void SetMatrix(double *matrix, bool concatenate)

            void RotateOrigin(double *vector, double angle, bool concatenate)
            void RotateXYZ(int ny, double angle, bool concatenate)

            void Scale(double scale, bool concatenate)
            void Scale(double *scale, bool concatenate)

            void SetPreMultiply()
            void SetPostMultiply()

            void SetAngleRadian()

cdef class CSTransform:
    cdef  _CSTransform *thisptr