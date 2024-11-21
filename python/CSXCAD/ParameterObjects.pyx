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
from libc.stdint cimport uintptr_t

cimport CSXCAD.ParameterObjects

cdef class ParameterSet:
    _instances = {}

    @staticmethod
    cdef fromPtr(_ParameterSet  *ptr):
        if ptr == NULL:
            return None
        cdef ParameterSet pset
        pset = ParameterSet._instances.get(<uintptr_t>ptr, None)
        if pset is not None:
            return pset
        pset = ParameterSet(no_init=True)
        pset._SetPtr(ptr)
        return pset

    def __cinit__(self, no_init=False):
        self.no_init = no_init
        if no_init==True:
            self.thisptr = NULL
        else:
            self._SetPtr(new _ParameterSet())

    cdef _SetPtr(self, _ParameterSet *ptr):
        if self.thisptr != NULL and self.thisptr != ptr:
            raise Exception('Different C++ class pointer already assigned to python wrapper class!')
        self.thisptr = ptr
        ParameterSet._instances[<uintptr_t>self.thisptr] = self

    def __dealloc__(self):
        if not self.no_init:
            del ParameterSet._instances[<uintptr_t>self.thisptr]
            del self.thisptr
