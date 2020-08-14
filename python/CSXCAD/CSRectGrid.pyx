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
Rectilinear Grid Class for CSXCAD
"""

import numpy as np
cimport CSRectGrid
from Utilities import CheckNyDir
from CSXCAD.SmoothMeshLines import SmoothMeshLines

cdef class CSRectGrid:
    """
    Rectilinear Grid Class for CSXCAD. The grid can be defined e.g. as a Cartesian
    or cylindrical mesh and can hold mesh lines in the 3 fundamental directions.
    E.g. x,y and z for the Cartesian and rho, a and z for the cylindrical mesh.

    :param CoordSystem: define the coordinate system (default 0 : Cartesian)
    """
    def __cinit__(self, *args, no_init=False, **kw):
        self.no_init = no_init
        if no_init==False:
            self.thisptr = new _CSRectGrid()
            if 'CoordSystem' in kw:
                self.SetMeshType(kw['CoordSystem'])
                del kw['CoordSystem']
            elif 'cs_type' in kw:
                self.SetMeshType(kw['cs_type'])
                del kw['cs_type']
        else:
            self.thisptr = NULL

        assert len(kw)==0, 'Unknown keyword arguments: "{}"'.format(kw)

    def __dealloc__(self):
        if not self.no_init:
            del self.thisptr

    def SetMeshType(self, cs_type):
        """ SetMeshType(cs_type)

        Set the coordinate system type (Cartesian or cylindrical) for this mesh.

        :param cs_type: coordinate system (0 : Cartesian, 1 : Cylindrical)
        """
        assert cs_type in [CARTESIAN, CYLINDRICAL], 'Unknown coordinate system: {}'.format(cs_type)
        self.thisptr.SetMeshType(cs_type)

    def GetMeshType(self):
        return self.thisptr.GetMeshType()

    def SetLines(self, ny, lines):
        """ SetLines(ny, lines)

        Set an array of lines. This will clear all previous defined lines in
        the given direction.

        :param ny: int or str -- direction definition
        :param lines: array -- list of lines to be set in the given direction
        """
        ny = CheckNyDir(ny)

        assert len(lines)>0, 'SetLines: "lines" must be an array or list'
        self.thisptr.ClearLines(ny)
        for n in range(len(lines)):
            self.thisptr.AddDiscLine(ny, lines[n])

    def AddLine(self, ny, line):
        """ AddLine(ny, lines)

        Add an array of lines. This will *not* clear the previous defined lines in
        the given direction.

        :param ny: int or str -- direction definition
        :param lines: array -- list of lines to be added in the given direction
        """
        ny = CheckNyDir(ny)
        if np.isscalar(line):
            self.thisptr.AddDiscLine(ny, line)
            return
        assert len(line)>0, 'AddLine: "lines" must be a float, array or list'
        for n in range(len(line)):
            self.thisptr.AddDiscLine(ny, line[n])

    def GetQtyLines(self, ny):
        """ GetQtyLines(ny)

        :param ny: int or str -- direction definition
        """
        ny = CheckNyDir(ny)
        return self.thisptr.GetQtyLines(ny)

    def GetLine(self, ny, idx):
        """ GetLine(ny, idx)

        Get the line in a given direction `ny` and index

        :param ny: int or str -- direction definition
        :param idx: int  -- line index
        """
        ny = CheckNyDir(ny)
        return self.thisptr.GetLine(ny, idx)

    def GetLines(self, ny, do_sort=False):
        """ GetLines(ny, do_sort=False)

        Get all lines in a given direction `ny`.

        :param ny: int or str -- direction definition
        :param do_sort: bool  -- sort lines
        """
        ny = CheckNyDir(ny)
        cdef unsigned int N = 0
        cdef double* array = NULL
        array = self.thisptr.GetLines(ny, array, N, do_sort)
        lines = np.zeros(N)
        for n in range(N):
            lines[n] = array[n]
        return lines

    def ClearLines(self, ny):
        """ ClearLines(ny)

        Clear all lines in a given direction `ny`.

        :param ny: int or str -- direction definition
        """
        ny = CheckNyDir(ny)
        self.thisptr.ClearLines(ny)

    def SmoothMeshLines(self, ny, max_res, ratio=1.5):
        """ SmoothMeshLines(ny, max_res, ratio=1.5)

        Smooth all mesh lines in the given direction with a max. allowed resolution.

        :param ny: int or str -- direction definition or 'all' for all directions
        :param max_res: float -- max. allowed resolution
        :param ratio:   float -- max. allowed ration of mesh smoothing de/increase
        """
        if ny=='all':
            for n in range(3):
                self.SmoothMeshLines(n, max_res, ratio)
        else:
            lines = self.GetLines(ny)
            lines = SmoothMeshLines(lines, max_res, ratio)
            self.SetLines(ny, lines)

    def Clear(self):
        """
        Clear all lines and delta unit.
        """
        self.thisptr.clear()

    def SetDeltaUnit(self, unit):
        """ SetDeltaUnit(unit)

        Set the drawing unit for all mesh lines. Default is 1 (m)
        """
        self.thisptr.SetDeltaUnit(unit)

    def GetDeltaUnit(self):
        """
        Get the drawing unit for all mesh lines.
        """
        return self.thisptr.GetDeltaUnit()

    def Sort(self, ny='all'):
        """ Sort(ny='all')

        Sort mesh lines in the given direction or all directions.
        """
        if ny=='all':
            for n in range(3):
                self.thisptr.Sort(n)
        else:
            ny = CheckNyDir(ny)
            self.thisptr.Sort(ny)

    def Snap2LineNumber(self, ny, value):
        """ Snap2LineNumber(ny, value)

        Find a fitting mesh line index for the given direction and value.
        """
        ny = CheckNyDir(ny)
        cdef bool inside=False
        pos = self.thisptr.Snap2LineNumber(ny, value, inside)
        return pos, inside>0

    def GetSimArea(self):
        """
        Get the simulation area as defined by the mesh.

        :returns: (2,3) array -- Simulation domain box
        """
        bb = np.zeros([2,3])
        cdef double *_bb = self.thisptr.GetSimArea()
        for n in range(3):
            bb[0,n] = _bb[2*n]
            bb[1,n] = _bb[2*n+1]
        return bb

    def IsValid(self):
        """
        Check if the mesh is valid. That is at least 2 mesh lines in all directions.
        """
        return self.thisptr.isValid()
