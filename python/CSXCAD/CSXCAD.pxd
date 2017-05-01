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
from libcpp.vector cimport vector

cimport CSPrimitives
cimport CSProperties

from CSPrimitives cimport _CSPrimitives, CSPrimitives, PrimitiveType
from ParameterObjects cimport _ParameterSet, ParameterSet
from CSProperties cimport _CSProperties, CSProperties, PropertyType
from CSRectGrid cimport _CSRectGrid, CSRectGrid, CoordinateSystem

cdef extern from "CSXCAD/ContinuousStructure.h":
    cdef cppclass _ContinuousStructure "ContinuousStructure":
            _ContinuousStructure() except +
            bool Write2XML(string)
            string ReadFromXML(string)
            _ParameterSet* GetParameterSet()

            _CSRectGrid* GetGrid()

            void SetCoordInputType(CoordinateSystem cs_type)

            void AddProperty(_CSProperties* prop)
            _CSProperties* GetProperty(int index)
            int GetQtyPrimitives(PropertyType prop_type)
            int  GetQtyProperties()

            _CSProperties* GetPropertyByCoordPriority(const double* coord, PropertyType prop_type, bool markFoundAsUsed, _CSPrimitives** foundPrimitive)

            vector[_CSPrimitives*] GetAllPrimitives(bool sort, PropertyType prop_type)

            vector[_CSProperties*] GetPropertiesByName(string name)
            vector[_CSProperties*] GetPropertyByType(PropertyType prop_type)

            string Update()

cdef class ContinuousStructure:
    cdef _ContinuousStructure *thisptr      # hold a C++ instance which we're wrapping
    cdef readonly ParameterSet __paraset
    cdef readonly CSRectGrid   __grid
    cdef _AddProperty(self, CSProperties prop)
    cdef _GetProperty(self, int index)
    cdef __GetPropertyByCoordPriority(self, double* coord, PropertyType prop_type, bool markFoundAsUsed)
    cdef __GetAllPrimitives(self, bool sort, PropertyType prop_type)
    cdef __GetPropertiesByName(self, string name)
    cdef __GetPropertyByType(self, PropertyType prop_type)