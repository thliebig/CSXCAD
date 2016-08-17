# -*- coding: utf-8 -*-
"""
Created on Mon Dec  7 23:27:39 2015

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

cimport CSPrimitives
cimport CSProperties

from CSPrimitives cimport _CSPrimitives, CSPrimitives
from ParameterObjects cimport _ParameterSet, ParameterSet
from CSProperties cimport _CSProperties, CSProperties
from CSRectGrid cimport _CSRectGrid, CSRectGrid

cdef extern from "CSXCAD/ContinuousStructure.h":
    cdef cppclass _ContinuousStructure "ContinuousStructure":
            _ContinuousStructure() except +
            bool Write2XML(string)
            _ParameterSet* GetParameterSet()
            void AddProperty(_CSProperties* prop)
            _CSRectGrid* GetGrid()

cdef class ContinuousStructure:
    cdef _ContinuousStructure *thisptr      # hold a C++ instance which we're wrapping
    cdef readonly ParameterSet paraset
    cdef readonly CSRectGrid   grid
    cdef _AddProperty(self, CSProperties prop)
