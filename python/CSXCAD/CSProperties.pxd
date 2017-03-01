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
from libcpp.vector cimport vector
from libcpp cimport bool

from ParameterObjects cimport _ParameterSet, ParameterSet
from CSPrimitives cimport _CSPrimitives, CSPrimitives
from CSXCAD cimport ContinuousStructure

cdef extern from "CSXCAD/CSProperties.h":
    cpdef enum PropertyType "CSProperties::PropertyType":
        ANY                "CSProperties::ANY"
        UNKNOWN            "CSProperties::UNKNOWN"
        MATERIAL           "CSProperties::MATERIAL"
        METAL              "CSProperties::METAL"
        EXCITATION         "CSProperties::EXCITATION"
        PROBEBOX           "CSProperties::PROBEBOX"
        RESBOX             "CSProperties::RESBOX"
        DUMPBOX            "CSProperties::DUMPBOX"
        DISPERSIVEMATERIAL "CSProperties::DISPERSIVEMATERIAL"
        LORENTZMATERIAL    "CSProperties::LORENTZMATERIAL"
        DEBYEMATERIAL      "CSProperties::DEBYEMATERIAL"
        DISCRETE_MATERIAL  "CSProperties::DISCRETE_MATERIAL"
        LUMPED_ELEMENT     "CSProperties::LUMPED_ELEMENT"
        CONDUCTINGSHEET    "CSProperties::CONDUCTINGSHEET"

cdef extern from "CSXCAD/CSProperties.h":
    cdef cppclass _CSProperties "CSProperties":
            _CSProperties(_ParameterSet*) except +
            int GetType()
            string GetTypeString()

            _ParameterSet* GetParameterSet()

            void SetName(string name)
            string GetName()

            int GetQtyPrimitives()
            _CSPrimitives* GetPrimitive(size_t index)

            bool ExistAttribute(string name)
            string GetAttributeValue(string name)
            void AddAttribute(string name, string value)

            void SetFillColor(unsigned char R, unsigned char G, unsigned char B, unsigned char a)
            void SetEdgeColor(unsigned char R, unsigned char G, unsigned char B, unsigned char a)

cdef class CSProperties:
    cdef  _CSProperties *thisptr
    cdef __SetPtr(self, _CSProperties *ptr)
    cdef __GetPrimitive(self, size_t index)
    cdef readonly ContinuousStructure __CSX
    cdef readonly ParameterSet __paraset
    cdef readonly list __primitives

##############################################################################
cdef extern from "CSXCAD/CSPropMaterial.h":
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
    pass

##############################################################################
cdef extern from "CSXCAD/CSPropLumpedElement.h":
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
    pass

##############################################################################
cdef extern from "CSXCAD/CSPropMetal.h":
    cdef cppclass _CSPropMetal "CSPropMetal" (_CSProperties):
            _CSPropMetal(_ParameterSet*) except +

cdef class CSPropMetal(CSProperties):
    pass

##############################################################################
cdef extern from "CSXCAD/CSPropConductingSheet.h":
    cdef cppclass _CSPropConductingSheet "CSPropConductingSheet" (_CSPropMetal):
            _CSPropConductingSheet(_ParameterSet*) except +
            void SetConductivity(double val)
            double GetConductivity()

            void SetThickness(double val)
            double GetThickness()

cdef class CSPropConductingSheet(CSPropMetal):
    pass

##############################################################################
cdef extern from "CSXCAD/CSPropExcitation.h":
    cdef cppclass _CSPropExcitation "CSPropExcitation" (_CSProperties):
            _CSPropExcitation(_ParameterSet*) except +
            void SetExcitType(int val)
            int  GetExcitType()

            void SetExcitation(double val, int Component)
            double GetExcitation(int Component)

            void SetPropagationDir(double val, int Component)
            double GetPropagationDir(int Component)

            int SetWeightFunction(string fct, int ny)
            string GetWeightFunction(int ny)

            void SetFrequency(double val)
            double GetFrequency()

            void SetDelay(double val)
            double GetDelay()

cdef class CSPropExcitation(CSProperties):
    pass

##############################################################################
cdef extern from "CSXCAD/CSPropProbeBox.h":
    cdef cppclass _CSPropProbeBox "CSPropProbeBox" (_CSProperties):
            _CSPropProbeBox(_ParameterSet*) except +
            void SetProbeType(int type)
            int GetProbeType()

            void SetWeighting(double weight)
            double GetWeighting()

            void SetNormalDir(unsigned int ndir)
            int GetNormalDir()

            size_t CountFDSamples()
            vector[double]* GetFDSamples()
            void ClearFDSamples()
            void AddFDSample(double freq)

cdef class CSPropProbeBox(CSProperties):
    pass


##############################################################################
cdef extern from "CSXCAD/CSPropDumpBox.h":
    cdef cppclass _CSPropDumpBox "CSPropDumpBox" (_CSPropProbeBox):
            _CSPropDumpBox(_ParameterSet*) except +
            void SetDumpType(int _type)
            int GetDumpType()

            void SetDumpMode(int mode)
            int GetDumpMode()

            void SetFileType(int ftype)
            int GetFileType()

            void SetOptResolution(int ny, double val)
            double GetOptResolution(int ny)

            void SetSubSampling(int ny, unsigned int val)
            unsigned int GetSubSampling(int ny)

cdef class CSPropDumpBox(CSPropProbeBox):
    pass
