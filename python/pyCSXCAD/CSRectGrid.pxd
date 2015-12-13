# -*- coding: utf-8 -*-
"""
Created on Sat Dec 12 21:56:38 2015

@author: thorsten
*
*	Copyright (C) 2015 Thorsten Liebig (Thorsten.Liebig@gmx.de)
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU Lesser General Public License as published
*	by the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
"""

from libcpp.string cimport string
from libcpp cimport bool

cdef extern from "CSXCAD_Global.h":
    ctypedef enum CoordinateSystem "CoordinateSystem":
        CARTESIAN    "CARTESIAN"
        CYLINDRICAL  "CYLINDRICAL"
        UNDEFINED_CS "UNDEFINED_CS"

cdef extern from "CSRectGrid.h":
        cdef cppclass _CSRectGrid "CSRectGrid":
            _CSRectGrid() except +
            void AddDiscLine(int direct, double val)

            void clear()
            void ClearLines(int direct)

            void SetDeltaUnit(double val)
            double GetDeltaUnit()

            void Sort(int direct)
            double* GetLines(int direct, double *array, unsigned int &qty, bool sorted)
            size_t GetQtyLines(int direct)
            double GetLine(int direct, size_t Index)

            unsigned int Snap2LineNumber(int ny, double value, bool &inside)

            int GetDimension()
            void SetMeshType(CoordinateSystem type)
            CoordinateSystem GetMeshType()

            void IncreaseResolution(int nu, int factor)
            double* GetSimArea()
            bool isValid()

cdef class CSRectGrid:
    cdef  _CSRectGrid *thisptr
