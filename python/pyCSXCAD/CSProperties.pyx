# -*- coding: utf-8 -*-
"""
Created on Sat Dec  5 13:26:56 2015

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

import numpy as np
from ParameterObjects cimport _ParameterSet, ParameterSet
cimport CSProperties
from Utilities import CheckNyDir

cdef class CSProperties:
    def __init__(self, ParameterSet pset, *args, **kw):
        assert self.thisptr, "Error, cannot create CSProperties (protected)"
        self.CSX = None

    def __dealloc__(self):
        # only delete if this property is not owned by a CSX object
        if self.CSX is None:
            print("del prop")
            del self.thisptr

    def GetQtyPrimitives(self):
        return self.thisptr.GetQtyPrimitives()
    def GetType(self):
        return self.thisptr.GetType()
    def GetTypeString(self):
        return self.thisptr.GetTypeString().decode('UTF-8')
    def SetName(self, name):
        self.thisptr.SetName(name.encode('UTF-8'))
    def GetName(self):
        return self.thisptr.GetName().decode('UTF-8')

###############################################################################
cdef class CSPropMaterial(CSProperties):
    def __init__(self, ParameterSet pset, *args, **kw):
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropMaterial(pset.thisptr)
        super(CSPropMaterial, self).__init__(pset, *args, **kw)
        self.matptr  = <_CSPropMaterial*>self.thisptr
        self.SetMaterialProperty(**kw)

    def SetMaterialProperty(self, **kw):
        for prop_name in kw:
            val = kw[prop_name]
            if prop_name == 'Density':
                self.matptr.SetDensity(val)
                continue
            if type(val)==float or type(val)==int:
                self.SetMaterialPropertyDir(prop_name, 0, val)
                continue
            assert len(val)==3
            for n in range(3):
                self.SetMaterialPropertyDir(prop_name, n, val[n])

    def SetMaterialPropertyDir(self, prop_name, ny, val):
        if prop_name=='Epsilon':
            return self.matptr.SetEpsilon(val, ny)
        elif prop_name=='Mue':
            return self.matptr.SetMue(val, ny)
        elif prop_name=='Kappa':
            return self.matptr.SetKappa(val, ny)
        elif prop_name=='Sigma':
            return self.matptr.SetSigma(val, ny)
        else:
            raise Exception('SetMaterialPropertyDir: Error, unknown material property')

    def SetMaterialWeight(self, **kw):
        for prop_name in kw:
            val = kw[prop_name]
            if prop_name == 'Density':
                self.matptr.SetDensityWeightFunction(val.encode('UTF-8'))
                continue
            if type(val)==str:
                self.SetMaterialWeightDir(prop_name, 0, val)
                continue
            assert len(val)==3
            for n in range(3):
                self.SetMaterialWeightDir(prop_name, n, val[n])

    def SetMaterialWeightDir(self, prop_name, ny, val):
        val = val.encode('UTF-8')
        if prop_name=='Epsilon':
            return self.matptr.SetEpsilonWeightFunction(val, ny)
        elif prop_name=='Mue':
            return self.matptr.SetMueWeightFunction(val, ny)
        elif prop_name=='Kappa':
            return self.matptr.SetKappaWeightFunction(val, ny)
        elif prop_name=='Sigma':
            return self.matptr.SetSigmaWeightFunction(val, ny)
        else:
            raise Exception('SetMaterialWeightDir: Error, unknown material property')

    def GetMaterialProperty(self, prop_name):
        if prop_name == 'Density':
            return self.matptr.GetDensity()
        if self.matptr.GetIsotropy():
            return self.GetMaterialPropertyDir(prop_name, 0)
        val = np.zeros(3)
        for n in range(3):
            val[n] = self.GetMaterialPropertyDir(prop_name, n)
        return val

    def GetMaterialPropertyDir(self, prop_name, ny):
        if prop_name=='Epsilon':
            return self.matptr.GetEpsilon(ny)
        elif prop_name=='Mue':
            return self.matptr.GetMue(ny)
        elif prop_name=='Kappa':
            return self.matptr.GetKappa(ny)
        elif prop_name=='Sigma':
            return self.matptr.GetSigma(ny)
        else:
            raise Exception('GetMaterialPropertyDir: Error, unknown material property')

    def GetMaterialWeight(self, prop_name):
        if prop_name == 'Density':
            return self.matptr.GetDensityWeightFunction().decode('UTF-8')
        if self.matptr.GetIsotropy():
            return self.GetMaterialWeightDir(prop_name, 0).decode('UTF-8')
        val = np.zeros(3)
        for n in range(3):
            val[n] = self.GetMaterialWeightDir(prop_name, n).decode('UTF-8')
        return val

    def GetMaterialWeightDir(self, prop_name, ny):
        if prop_name=='Epsilon':
            return self.matptr.GetEpsilonWeightFunction(ny)
        elif prop_name=='Mue':
            return self.matptr.GetMueWeightFunction(ny)
        elif prop_name=='Kappa':
            return self.matptr.GetKappaWeightFunction(ny)
        elif prop_name=='Sigma':
            return self.matptr.GetSigmaWeightFunction(ny)
        else:
            raise Exception('GetMaterialWeightDir: Error, unknown material property')


###############################################################################
cdef class CSPropLumpedElement(CSProperties):
    def __init__(self, ParameterSet pset, *args, **kw):
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropLumpedElement(pset.thisptr)
        super(CSPropLumpedElement, self).__init__(pset, *args, **kw)
        self.LEptr   = <_CSPropLumpedElement*> self.thisptr

        for k in kw:
            if k=='R':
                self.SetResistance(kw[k])
            elif k=='L':
                self.SetInductance(kw[k])
            elif k=='C':
                self.SetCapacity(kw[k])
            elif k=='ny':
                self.SetDirection(kw[k])
            elif k.lower()=='caps':
                self.SetCaps(kw[k])
            else:
                raise Exception('CSPropLumpedElement: Error, unknown key word')

    def SetResistance(self, val):
        self.LEptr.SetResistance(val)
    def GetResistance(self):
        return self.LEptr.GetResistance()

    def SetCapacity(self, val):
        self.LEptr.SetCapacity(val)
    def GetCapacity(self):
        return self.LEptr.GetCapacity()

    def SetInductance(self, val):
        self.LEptr.SetInductance(val)
    def GetInductance(self):
        return self.LEptr.GetInductance()

    def SetDirection(self, val):
        self.LEptr.SetDirection(CheckNyDir(val))
    def GetDirection(self):
        return self.LEptr.GetDirection()

    def SetCaps(self, val):
        self.LEptr.SetCaps(val)
    def GetCaps(self):
        return self.LEptr.GetCaps()==1


###############################################################################
cdef class CSPropMetal(CSProperties):
    def __init__(self, ParameterSet pset, *args, **kw):
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropMetal(pset.thisptr)
        super(CSPropMetal, self).__init__(pset, *args, **kw)

###############################################################################
cdef class CSPropConductingSheet(CSPropMetal):
    def __init__(self, ParameterSet pset, *args, **kw):
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropConductingSheet(pset.thisptr)
        super(CSPropConductingSheet, self).__init__(pset, *args, **kw)
        self.CSptr   = <_CSPropConductingSheet*> self.thisptr
        assert len(args)<=2
        if len(args)>0:
            self.SetConductivity(args[0])
        if len(args)>1:
            self.SetThickness(args[1])

    def SetConductivity(self, val):
        self.CSptr.SetConductivity(val)
    def GetConductivity(self):
        return self.CSptr.GetConductivity()

    def SetThickness(self, val):
        self.CSptr.SetThickness(val)
    def GetThickness(self):
        return self.CSptr.GetThickness()

###############################################################################
cdef class CSPropExcitation(CSProperties):
    def __init__(self, ParameterSet pset, *args, **kw):
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropExcitation(pset.thisptr)
        super(CSPropExcitation, self).__init__(pset, *args, **kw)
        self.excptr = <_CSPropExcitation*>self.thisptr

    def SetExcitType(self, val):
        self.excptr.SetExcitType(val)

    def GetExcitType(self):
        return self.excptr.GetExcitType()

    def SetExcitation(self, val):
        assert len(val)==3, "Error, excitation vector must be of dimension 3"
        for n in range(3):
            self.excptr.SetExcitation(val[n], n)

    def GetExcitation(self):
        val = np.zeros(3)
        for n in range(3):
            val[n] = self.excptr.GetExcitation(n)
        return val

    def SetPropagationDir(self, val):
        assert len(val)==3, "Error, excitation vector must be of dimension 3"
        for n in range(3):
            self.excptr.SetPropagationDir(val[n], n)

    def GetPropagationDir(self):
        val = np.zeros(3)
        for n in range(3):
            val[n] = self.excptr.GetPropagationDir(n)
        return val

    def SetDelay(self, val):
        self.excptr.SetDelay(val)

    def GetDelay(self):
        return self.excptr.GetDelay()
