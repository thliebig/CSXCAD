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

from libcpp.string cimport string
from libcpp cimport bool
from libc.stdint cimport uintptr_t
import weakref
from CSXCAD.CSObject cimport CSDestructionCallback, wrapper_destroyed
from CSXCAD.CSObject cimport resolve_owner, PARAMETERSET
from CSXCAD.Utilities import RegisterWrapperFactory

cimport CSXCAD.ParameterObjects

cdef class ParameterSet:
    _instances = weakref.WeakValueDictionary()
    """ Wrapper per live C++ instance, so that looking the same object up twice
    gives the same wrapper. Weak on purpose: a wrapper nobody holds any more must
    be collectable, otherwise it would keep its owner alive forever. """

    @staticmethod
    cdef fromPtr(_ParameterSet  *ptr):
        if ptr == NULL:
            return None
        cdef ParameterSet pset
        pset = ParameterSet._instances.get(<uintptr_t>ptr, None)
        # an entry whose C++ object is already gone must not be handed out: the
        # weak reference only drops it once the wrapper itself dies
        if pset is not None and pset.thisptr != NULL:
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

    @staticmethod
    def _from_address(addr):
        """ Wrapper for the C++ instance at `addr`, \sa CSXCAD.Utilities """
        return ParameterSet.fromPtr(<_ParameterSet*><uintptr_t>addr)

    cdef _SetPtr(self, _ParameterSet *ptr):
        if self.thisptr != NULL and self.thisptr != ptr:
            raise Exception('Different C++ class pointer already assigned to python wrapper class!')
        self.thisptr = ptr
        ParameterSet._instances[<uintptr_t>self.thisptr] = self
        self._owner = resolve_owner(self.thisptr)
        self.thisptr.SetDestructionCallback(<CSDestructionCallback>wrapper_destroyed,
                                           <void*>&self.thisptr)

    def __dealloc__(self):
        # drop the hook first: for an owned set we delete the C++ object right
        # below, for a borrowed one it may already have been invalidated
        if self.thisptr != NULL:
            self.thisptr.SetDestructionCallback(NULL, NULL)
            if not self.no_init:
                del self.thisptr

RegisterWrapperFactory(PARAMETERSET, ParameterSet._from_address)
