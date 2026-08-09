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

from libcpp cimport bool
from CSXCAD.CSObject cimport _CSObject

cdef extern from "CSXCAD/ParameterObjects.h":
    cdef cppclass _ParameterSet "ParameterSet"(_CSObject):
            _ParameterSet() except +
            void PrintSelf()
            bool GetModified()

cdef class ParameterSet:
    cdef _ParameterSet *thisptr
    cdef object __weakref__
    # wrapper that has to stay alive for our C++ instance to stay valid,
    # normally the ContinuousStructure the instance belongs to
    cdef object _owner
    @staticmethod
    cdef fromPtr(_ParameterSet  *ptr)
    cdef _SetPtr(self, _ParameterSet *ptr)
    cdef bool no_init
