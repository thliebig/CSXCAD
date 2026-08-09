# -*- coding: utf-8 -*-
# cython: language_level=3
#
# Copyright (C) 2015,2016 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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
cimport CSXCAD.CSRectGrid
from CSXCAD.Utilities import CheckNyDir
from CSXCAD.SmoothMeshLines import SmoothMeshLines
from libc.stdint cimport uintptr_t
import weakref
from CSXCAD.CSObject cimport CSDestructionCallback, wrapper_destroyed
from CSXCAD.CSObject cimport resolve_owner, GRID
from CSXCAD.Utilities import RegisterWrapperFactory

cdef class CSRectGrid:
    _instances = weakref.WeakValueDictionary()
    """ Wrapper per live C++ instance, so that looking the same object up twice
    gives the same wrapper. Weak on purpose: a wrapper nobody holds any more must
    be collectable, otherwise it would keep its owner alive forever. """

    @staticmethod
    cdef fromPtr(_CSRectGrid  *ptr):
        if ptr == NULL:
            return None
        cdef CSRectGrid cls
        cls = CSRectGrid._instances.get(<uintptr_t>ptr, None)
        # an entry whose C++ object is already gone must not be handed out: the
        # weak reference only drops it once the wrapper itself dies
        if cls is not None and cls.thisptr != NULL:
            return cls
        cls = CSRectGrid(no_init=True)
        cls._SetPtr(ptr)
        return cls

    """
    Rectilinear Grid Class for CSXCAD. The grid can be defined e.g. as a Cartesian
    or cylindrical mesh and can hold mesh lines in the 3 fundamental directions.
    E.g. x,y and z for the Cartesian and rho, a and z for the cylindrical mesh.

    :param CoordSystem: define the coordinate system (default 0 : Cartesian)
    """
    def __cinit__(self, *args, no_init=False, **kw):
        self.no_init = no_init
        if no_init==False:
            self._SetPtr(new _CSRectGrid())
            if 'CoordSystem' in kw:
                self.SetMeshType(kw['CoordSystem'])
                del kw['CoordSystem']
            elif 'cs_type' in kw:
                self.SetMeshType(kw['cs_type'])
                del kw['cs_type']
        else:
            self.thisptr = NULL

        assert len(kw)==0, 'Unknown keyword arguments: "{}"'.format(kw)

    @staticmethod
    def _from_address(addr):
        """ Wrapper for the C++ instance at `addr`, \sa CSXCAD.Utilities """
        return CSRectGrid.fromPtr(<_CSRectGrid*><uintptr_t>addr)

    cdef _SetPtr(self, _CSRectGrid *ptr):
        if self.thisptr != NULL and self.thisptr != ptr:
            raise Exception('Different C++ class pointer already assigned to python wrapper class!')
        self.thisptr = ptr
        CSRectGrid._instances[<uintptr_t>self.thisptr] = self
        self._owner = resolve_owner(self.thisptr)
        self.thisptr.SetDestructionCallback(<CSDestructionCallback>wrapper_destroyed,
                                           <void*>&self.thisptr)

    def __dealloc__(self):
        # drop the hook first: for an owned grid we delete the C++ object right
        # below, for a borrowed one it may already have been invalidated
        if self.thisptr != NULL:
            self.thisptr.SetDestructionCallback(NULL, NULL)
            if not self.no_init:
                del self.thisptr

    cdef _CSRectGrid* _ptr(self) except NULL:
        """ Access the C++ instance, raising if it has already been destroyed.

        The C++ instance is owned by the ContinuousStructure and can be destroyed while this
        wrapper is still referenced from python. \sa wrapper_destroyed
        """
        if self.thisptr == NULL:
            raise RuntimeError('wrapped C++ object of type {} has been deleted'.format(type(self).__name__))
        return self.thisptr

    def SetMeshType(self, cs_type):
        """ SetMeshType(cs_type)

        Set the coordinate system type (Cartesian or cylindrical) for this mesh.

        :param cs_type: coordinate system (0 : Cartesian, 1 : Cylindrical)
        """
        assert cs_type in [CARTESIAN, CYLINDRICAL], 'Unknown coordinate system: {}'.format(cs_type)
        self._ptr().SetMeshType(cs_type)

    def GetMeshType(self):
        return self._ptr().GetMeshType()

    def SetLines(self, ny, lines):
        """ SetLines(ny, lines)

        Set an array of lines. This will clear all previous defined lines in
        the given direction.

        :param ny: int or str -- direction definition
        :param lines: array -- list of lines to be set in the given direction
        """
        ny = CheckNyDir(ny)

        assert len(lines)>0, 'SetLines: "lines" must be an array or list'
        self._ptr().ClearLines(ny)
        for n in range(len(lines)):
            self._ptr().AddDiscLine(ny, lines[n])

    def AddLine(self, ny, line):
        """ AddLine(ny, lines)

        Add an array of lines. This will *not* clear the previous defined lines in
        the given direction.

        :param ny: int or str -- direction definition
        :param lines: array -- list of lines to be added in the given direction
        """
        ny = CheckNyDir(ny)
        if np.isscalar(line):
            self._ptr().AddDiscLine(ny, line)
            return
        assert len(line)>0, 'AddLine: "lines" must be a float, array or list'
        for n in range(len(line)):
            self._ptr().AddDiscLine(ny, line[n])

    def GetQtyLines(self, ny):
        """ GetQtyLines(ny)

        :param ny: int or str -- direction definition
        """
        ny = CheckNyDir(ny)
        return self._ptr().GetQtyLines(ny)

    def GetLine(self, ny, idx):
        """ GetLine(ny, idx)

        Get the line in a given direction `ny` and index

        :param ny: int or str -- direction definition
        :param idx: int  -- line index
        """
        ny = CheckNyDir(ny)
        return self._ptr().GetLine(ny, idx)

    def GetLines(self, ny, do_sort=True):
        """ GetLines(ny, do_sort=True)

        Get all lines in a given direction `ny`.

        :param ny: int or str -- direction definition
        :param do_sort: bool  -- sort lines (default True)
        """
        ny = CheckNyDir(ny)
        if do_sort:
            self._ptr().Sort(ny)
        cdef size_t N = self._ptr().GetQtyLines(ny)
        lines = np.zeros(N)
        for n in range(N):
            lines[n] = self._ptr().GetLine(ny, n)
        return lines

    def ClearLines(self, ny):
        """ ClearLines(ny)

        Clear all lines in a given direction `ny`.

        :param ny: int or str -- direction definition
        """
        ny = CheckNyDir(ny)
        self._ptr().ClearLines(ny)

    def SmoothMeshLines(self, ny, max_res, ratio=1.5, check_symmetry=True):
        """ SmoothMeshLines(ny, max_res, ratio=1.5, check_symmetry=True)

        Smooth all mesh lines in the given direction with a max. allowed resolution.

        :param ny: int or str -- direction definition or 'all' for all directions
        :param max_res: float -- max. allowed resolution
        :param ratio:   float -- max. allowed ration of mesh smoothing de/increase
        :param check_symmetry: bool -- detect a symmetric mesh and smooth only one
            half (default True); set False to disable and smooth the full line set
        """
        if ny=='all':
            for n in range(3):
                self.SmoothMeshLines(n, max_res, ratio, check_symmetry)
        else:
            lines = self.GetLines(ny)
            lines = SmoothMeshLines(lines, max_res, ratio, check_symmetry=check_symmetry)
            self.SetLines(ny, lines)

    def Clear(self):
        """
        Clear all lines and delta unit.
        """
        self._ptr().clear()

    def SetDeltaUnit(self, unit):
        """ SetDeltaUnit(unit)

        Set the drawing unit for all mesh lines. Default is 1 (m)
        """
        self._ptr().SetDeltaUnit(unit)

    def GetDeltaUnit(self):
        """
        Get the drawing unit for all mesh lines.
        """
        return self._ptr().GetDeltaUnit()

    def Sort(self, ny='all'):
        """ Sort(ny='all')

        Sort mesh lines in the given direction or all directions.
        """
        if ny=='all':
            for n in range(3):
                self._ptr().Sort(n)
        else:
            ny = CheckNyDir(ny)
            self._ptr().Sort(ny)

    def Snap2LineNumber(self, ny, value):
        """ Snap2LineNumber(ny, value)

        Find a fitting mesh line index for the given direction and value.
        """
        ny = CheckNyDir(ny)
        cdef bool inside=False
        pos = self._ptr().Snap2LineNumber(ny, value, inside)
        return pos, inside>0

    def GetSimArea(self):
        """
        Get the simulation area as defined by the mesh.

        :returns: (2,3) array -- Simulation domain box
        """
        bb = np.zeros([2,3])
        cdef double *_bb = self._ptr().GetSimArea()
        for n in range(3):
            bb[0,n] = _bb[2*n]
            bb[1,n] = _bb[2*n+1]
        return bb

    def IsValid(self):
        """
        Check if the mesh is valid. That is at least 2 mesh lines in all directions.
        """
        return self._ptr().isValid()

RegisterWrapperFactory(GRID, CSRectGrid._from_address)
