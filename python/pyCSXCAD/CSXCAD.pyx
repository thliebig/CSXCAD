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
from CSProperties import CSPropLumpedElement, CSPropProbeBox, CSPropDumpBox
from CSPrimitives import CSPrimPoint, CSPrimBox, CSPrimCylinder, CSPrimCylindricalShell
from CSPrimitives import CSPrimSphere, CSPrimSphericalShell
from CSPrimitives import CSPrimPolygon, CSPrimLinPoly, CSPrimRotPoly
from CSPrimitives import CSPrimCurve, CSPrimWire
from CSPrimitives import CSPrimPolyhedron, CSPrimPolyhedronReader
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

    def AddConductingSheet(self, name, **kw):
        return self.CreateProperty('ConductingSheet', name, **kw)

    def AddExcitation(self, name, exc_type, exc_val, **kw):
        return self.CreateProperty('Excitation', name, exc_type=exc_type, exc_val=exc_val, **kw)

    def AddProbe(self, name, p_type, **kw):
        return self.CreateProperty('Probe', name, p_type=p_type, **kw)

    def AddDump(self, name, **kw):
        return self.CreateProperty('Dump', name, **kw)

    def CreateProperty(self, type_str, name, *args, **kw):
        assert len(args)==0, 'CreateProperty does not support additional arguments'
        if type_str=='Material':
            prop = CSPropMaterial(self.paraset, **kw)
        elif type_str=='LumpedElement':
            prop = CSPropLumpedElement(self.paraset, **kw)
        elif type_str=='Metal':
            assert len(kw)==0, 'CreateProperty: Metal does not support additional key words'
            prop = CSPropMetal(self.paraset)
        elif type_str=='ConductingSheet':
            prop = CSPropConductingSheet(self.paraset, **kw)
        elif type_str=='Excitation':
            prop = CSPropExcitation(self.paraset, **kw)
        elif type_str=='Probe':
            prop = CSPropProbeBox(self.paraset, **kw)
        elif type_str=='Dump':
            prop = CSPropDumpBox(self.paraset, **kw)
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

    def AddPoint(self, prop, coord, **kw):
        return self.CreatePrimitive('Point', prop, coord=coord, **kw)

    def AddBox(self, prop, start, stop, **kw):
        return self.CreatePrimitive('Box', prop, start=start, stop=stop, **kw)

    def AddClinder(self, prop, start, stop, radius, **kw):
        return self.CreatePrimitive('Cylinder', prop, start=start, stop=stop, radius=radius, **kw)

    def AddCylindricalShell(self, prop, start, stop, radius, shell_width, **kw):
        return self.CreatePrimitive('CylindricalShell', prop, start=start, stop=stop, radius=radius, shell_width=shell_width, **kw)

    def AddSphere(self, prop, center, radius, **kw):
        return self.CreatePrimitive('Sphere', prop, center=center, radius=radius, **kw)

    def AddSphericalShell(self, prop, center, radius, shell_width, **kw):
        return self.CreatePrimitive('SphericalShell', prop, center=center, radius=radius, shell_width=shell_width, **kw)

    def AddPolygon(self, prop, points, norm_dir, elevation, **kw):
        return self.CreatePrimitive('Polygon', prop, points=points, norm_dir=norm_dir, elevation=elevation, **kw)

    def AddLinPoly(self, prop, points, norm_dir, elevation, length, **kw):
        return self.CreatePrimitive('LinPoly', prop, points=points, norm_dir=norm_dir, elevation=elevation, length=length, **kw)

    def AddRotPoly(self, prop, points, norm_dir, elevation, rot_axis, angle, **kw):
        return self.CreatePrimitive('RotPoly', prop, points=points, norm_dir=norm_dir, elevation=elevation, rot_axis=rot_axis, angle=angle, **kw)

    def AddRotPoly(self, prop, points, norm_dir, elevation, rot_axis, angle, **kw):
        return self.CreatePrimitive('RotPoly', prop, points=points, norm_dir=norm_dir, elevation=elevation, rot_axis=rot_axis, angle=angle, **kw)

    def AddCurve(self, prop, points, **kw):
        return self.CreatePrimitive('Curve', prop, points=points, **kw)

    def AddWire(self, prop, points, radius, **kw):
        return self.CreatePrimitive('Wire', prop, points=points, radius=radius, **kw)

    def AddPolyhedronReader(self, prop, filename, **kw):
        return self.CreatePrimitive('PolyhedronReader', prop, filename=filename, **kw)

    def CreatePrimitive(self, type_str, prop, *args, **kw):
        pset = self.GetParameterSet()
        assert len(args)==0, 'CreatePrimitive: Box does not support additional arguments'
        if type_str=='Point':
            prim = CSPrimPoint(pset, prop, **kw)
        elif type_str=='Box':
            prim = CSPrimBox(pset, prop, **kw)
        elif type_str=='Clinder':
            prim = CSPrimCylinder(pset, prop, **kw)
        elif type_str=='CylindricalShell':
            prim = CSPrimCylindricalShell(pset, prop, **kw)
        elif type_str=='Sphere':
            prim = CSPrimSphere(pset, prop, **kw)
        elif type_str=='SphericalShell':
            prim = CSPrimSphericalShell(pset, prop, **kw)
        elif type_str=='Polygon':
            prim = CSPrimPolygon(pset, prop, **kw)
        elif type_str=='LinPoly':
            prim = CSPrimLinPoly(pset, prop, **kw)
        elif type_str=='RotPoly':
            prim = CSPrimRotPoly(pset, prop, **kw)
        elif type_str=='Curve':
            prim = CSPrimCurve(pset, prop, **kw)
        elif type_str=='Wire':
            prim = CSPrimWire(pset, prop, **kw)
        elif type_str=='Polyhedron':
            prim = CSPrimPolyhedron(pset, prop, **kw)
        elif type_str=='PolyhedronReader':
            prim = CSPrimPolyhedronReader(pset, prop, **kw)
        else:
            raise Exception('CreatePrimitive: Unknown primitive type requested: {}'.format(type_str))
        return prim

