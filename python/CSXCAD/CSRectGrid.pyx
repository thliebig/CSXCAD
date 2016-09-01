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

import numpy as np
cimport CSRectGrid
from Utilities import CheckNyDir
from SmoothMeshLines import SmoothMeshLines

cdef class CSRectGrid:
    def __cinit__(self, *args, no_init=False, **kw):
        if no_init==False:
            self.thisptr = new _CSRectGrid()
        else:
            self.thisptr = NULL

    def __dealloc__(self):
        del self.thisptr

    def SetLines(self, ny, lines):
        ny = CheckNyDir(ny)

        assert len(lines)>0
        self.thisptr.ClearLines(ny)
        for n in range(len(lines)):
            self.thisptr.AddDiscLine(ny, lines[n])

    def AddLine(self, ny, line):
        ny = CheckNyDir(ny)
        if type(line) in [float, int]:
            return self.thisptr.AddDiscLine(ny, line)
        assert len(line)>0
        for n in range(len(line)):
            self.thisptr.AddDiscLine(ny, line[n])

    def GetQtyLines(self, ny):
        ny = CheckNyDir(ny)
        return self.thisptr.GetQtyLines(ny)

    def GetLine(self, ny, idx):
        ny = CheckNyDir(ny)
        return self.thisptr.GetLine(ny, idx)

    def GetLines(self, ny, do_sort=False):
        ny = CheckNyDir(ny)
        cdef unsigned int N = 0
        cdef double* array = NULL
        array = self.thisptr.GetLines(ny, array, N, do_sort)
        lines = np.zeros(N)
        for n in range(N):
            lines[n] = array[n]
        return lines

    def ClearLines(self, ny):
        ny = CheckNyDir(ny)
        self.thisptr.ClearLines(ny)

    def SmoothMeshLines(self, ny, max_res, ratio=1.5):
        if ny=='all':
            for n in range(3):
                self.SmoothMeshLines(n, max_res, ratio)
        else:
            lines = self.GetLines(ny)
            lines = SmoothMeshLines(lines, max_res, ratio)
            self.SetLines(ny, lines)

    def Clear(self):
        self.thisptr.clear()

    def SetDeltaUnit(self, unit):
        self.thisptr.SetDeltaUnit(unit)

    def GetDeltaUnit(self):
        return self.thisptr.GetDeltaUnit()

    def Sort(self, ny='all'):
        if ny=='all':
            for n in range(3):
                self.thisptr.Sort(n)
        else:
            ny = CheckNyDir(ny)
            self.thisptr.Sort(ny)

    def Snap2LineNumber(self, ny, value):
        ny = CheckNyDir(ny)
        cdef bool inside=False
        pos = self.thisptr.Snap2LineNumber(ny, value, inside)
        return pos, inside>0

    def GetSimArea(self):
        bb = np.zeros([2,3])
        cdef double *_bb = self.thisptr.GetSimArea()
        for n in range(3):
            bb[0,n] = _bb[2*n]
            bb[1,n] = _bb[2*n+1]
        return bb

    def IsValid(self):
        return self.thisptr.isValid()
