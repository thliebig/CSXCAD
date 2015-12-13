# -*- coding: utf-8 -*-
"""
Created on Fri Dec  4 15:21:46 2015

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

cimport CSXCAD

from CSProperties import CSPropMaterial, CSPropExcitation
from CSProperties import CSPropMetal, CSPropConductingSheet
from CSProperties import CSPropLumpedElement
from CSPrimitives import CSPrimBox
from ParameterObjects import ParameterSet

cdef class ContinuousStructure:
    def __cinit__(self):
        self.thisptr = new _ContinuousStructure()
        self.paraset = ParameterSet(no_init=True)
        self.paraset.thisptr = self.thisptr.GetParameterSet()

        self.grid         = CSRectGrid(no_init=True)
        self.grid.thisptr = self.thisptr.GetGrid()

    def __dealloc__(self):
        self.grid.thisptr = NULL
        self.paraset.thisptr = NULL
        del self.thisptr

    def Write2XML(self, fn):
        self.thisptr.Write2XML(fn.encode('UTF-8'))

    def GetParameterSet(self):
        return self.paraset

    def GetGrid(self):
        return self.grid

    def AddMaterial(self, name, **kw):
        return self.CreateProperty('Material', name, **kw)

    def AddLumpedElement(self, name, **kw):
        return self.CreateProperty('LumpedElement', name, **kw)

    def AddMetal(self, name):
        return self.CreateProperty('Metal', name)

    def AddConductingSheet(self, name, *args):
        return self.CreateProperty('ConductingSheet', name, *args)

    def AddExcitation(self, name, exc_type, exc_val, delay=None, prop_dir=None):
        prop = self.CreateProperty('Excitation', name)
        prop.SetExcitType(exc_type)
        prop.SetExcitation(exc_val)
        if delay is not None:
            prop.SetDelay(delay)
        if prop_dir is not None:
            prop.SetPropagationDir(prop_dir)
        return prop

    def CreateProperty(self, type_str, name, *args, **kw):
        if type_str=='Material':
            assert len(args)==0, 'CreateProperty: Material does not support additional arguments'
            prop = CSPropMaterial(self.paraset, **kw)
        elif type_str=='LumpedElement':
            assert len(args)==0, 'CreateProperty: LumpedElement does not support additional arguments'
            prop = CSPropLumpedElement(self.paraset, **kw)
        elif type_str=='Metal':
            assert len(kw)==0, 'CreateProperty: Metal does not support additional key words'
            prop = CSPropMetal(self.paraset)
        elif type_str=='ConductingSheet':
            assert len(kw)==0, 'CreateProperty: ConductingSheel does not support additional key words'
            prop = CSPropConductingSheet(self.paraset, *args)
        elif type_str=='Excitation':
            assert len(args)==0, 'CreateProperty: Excitation does not support additional arguments'
            assert len(kw)==0  , 'CreateProperty: Excitation does not support additional key words'
            prop = CSPropExcitation(self.paraset, **kw)
        else:
            raise Exception('CreateProperty: Unknown property type requested: {}'.format(type_str))
        prop.SetName(name)
        self.AddProperty(prop)
        return prop

    def AddProperty(self, prop):
        self._AddProperty(prop)

    cdef _AddProperty(self, CSProperties prop):
        prop.CSX = self
        self.thisptr.AddProperty(prop.thisptr)



