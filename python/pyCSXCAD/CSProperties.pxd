# -*- coding: utf-8 -*-
"""
Created on Sat Dec  5 15:06:11 2015

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

from ParameterObjects cimport _ParameterSet
from CSXCAD cimport ContinuousStructure

cdef extern from "CSProperties.h":
    cdef cppclass _CSProperties "CSProperties":
            _CSProperties(_ParameterSet*) except +
            int GetType()
            string GetTypeString()
            void SetName(string name)
            string GetName()

            int GetQtyPrimitives()

cdef class CSProperties:
    cdef  _CSProperties *thisptr
    cdef readonly ContinuousStructure CSX

##############################################################################
cdef extern from "CSPropMaterial.h":
    cdef cppclass _CSPropMaterial "CSPropMaterial" (_CSProperties):
            _CSPropMaterial(_ParameterSet*) except +
            void SetEpsilon(double val, int ny)
            double GetEpsilon(int ny)
            int SetEpsilonWeightFunction(string fct, int ny)
            string GetEpsilonWeightFunction(int ny)

            void SetMue(double val, int ny)
            double GetMue(int ny)
            int SetMueWeightFunction(string fct, int ny)
            string GetMueWeightFunction(int ny)

            void SetKappa(double val, int ny)
            double GetKappa(int ny)
            int SetKappaWeightFunction(string fct, int ny)
            string GetKappaWeightFunction(int ny)

            void SetSigma(double val, int ny)
            double GetSigma(int ny)
            int SetSigmaWeightFunction(string fct, int ny)
            string GetSigmaWeightFunction(int ny)

            void SetDensity(double val)
            double GetDensity()
            int SetDensityWeightFunction(string fct)
            string GetDensityWeightFunction()

            void SetIsotropy(bool val)
            bool GetIsotropy()

cdef class CSPropMaterial(CSProperties):
    cdef _CSPropMaterial* matptr

##############################################################################
cdef extern from "CSPropLumpedElement.h":
    cdef cppclass _CSPropLumpedElement "CSPropLumpedElement" (_CSProperties):
            _CSPropLumpedElement(_ParameterSet*) except +
            void SetResistance(double val)
            double GetResistance()

            void SetCapacity(double val)
            double GetCapacity()

            void SetInductance(double val)
            double GetInductance()

            void SetDirection(int ny)
            int GetDirection()

            void SetCaps(bool val)
            int GetCaps()

cdef class CSPropLumpedElement(CSProperties):
    cdef _CSPropLumpedElement* LEptr

##############################################################################
cdef extern from "CSPropMetal.h":
    cdef cppclass _CSPropMetal "CSPropMetal" (_CSProperties):
            _CSPropMetal(_ParameterSet*) except +

cdef class CSPropMetal(CSProperties):
    pass

##############################################################################
cdef extern from "CSPropConductingSheet.h":
    cdef cppclass _CSPropConductingSheet "CSPropConductingSheet" (_CSPropMetal):
            _CSPropConductingSheet(_ParameterSet*) except +
            void SetConductivity(double val)
            double GetConductivity()

            void SetThickness(double val)
            double GetThickness()

cdef class CSPropConductingSheet(CSPropMetal):
    cdef _CSPropConductingSheet* CSptr

##############################################################################
cdef extern from "CSPropExcitation.h":
    cdef cppclass _CSPropExcitation "CSPropExcitation" (_CSProperties):
            _CSPropExcitation(_ParameterSet*) except +
            void SetExcitType(int val)
            int  GetExcitType()

            void SetExcitation(double val, int Component)
            double GetExcitation(int Component)

            void SetPropagationDir(double val, int Component)
            double GetPropagationDir(int Component)

            void SetDelay(double val)
            double GetDelay()

cdef class CSPropExcitation(CSProperties):
    cdef _CSPropExcitation* excptr

