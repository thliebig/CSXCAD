# -*- coding: utf-8 -*-
#
# Copyright (C) 2015-2025 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

"""
Module for all Properties like metal, material, propes and dumps

Notes
-----
Usually it is not meant to create properties manually, but instead
use the ContinuousStructure object to create properties using the
e.g. AddMaterial or AddMetal methods.


Examples
--------
Create a metal property:

>>> pset  = ParameterObjects.ParameterSet()
>>> metal = CSProperties.CSPropMetal(pset)
"""

import numpy as np
from CSXCAD.ParameterObjects cimport _ParameterSet, ParameterSet
cimport CSXCAD.CSProperties
cimport CSXCAD.CSPrimitives as c_CSPrimitives
from CSXCAD.CSPrimitives import CSPrimitives
from CSXCAD.Utilities import CheckNyDir
from libc.stdint cimport uintptr_t

def hex2color(color):
    if not type(color) is str:
        return color
    import matplotlib
    return tuple([int(x*255) for x in matplotlib.colors.to_rgb(color)])

cdef class CSProperties:
    _instances = {}
    """
    Virtual base class for all properties, cannot be created!

    """

    @staticmethod
    def fromType(p_type, pset, no_init=False, **kw):
        """ fromType(p_type, pset, no_init=False, **kw)

        Create a property specified by the `p_type`

        :param p_type: Property type
        :param pset: ParameterSet to assign to the new primitive
        :param no_init: do not create an actual C++ instance
        """
        if no_init and len(kw)!=0:
            raise Exception('Unable to not init a property and set kw options!')
        prop = None
        if p_type == CONDUCTINGSHEET + METAL:
            prop = CSPropConductingSheet(pset, no_init=no_init, **kw)
        elif p_type == METAL:
            prop = CSPropMetal(pset, no_init=no_init, **kw)
        elif p_type == MATERIAL:
            prop = CSPropMaterial(pset, no_init=no_init, **kw)
        elif p_type == LUMPED_ELEMENT:
            prop = CSPropLumpedElement(pset, no_init=no_init, **kw)
        elif p_type == EXCITATION:
            prop = CSPropExcitation(pset, no_init=no_init, **kw)
        elif p_type == PROBEBOX:
            prop = CSPropProbeBox(pset, no_init=no_init, **kw)
        elif p_type == DUMPBOX:
            prop = CSPropDumpBox(pset, no_init=no_init, **kw)
        elif p_type == LORENTZMATERIAL + DISPERSIVEMATERIAL + MATERIAL:
            prop = CSPropLorentzMaterial(pset, no_init=no_init, **kw)
        elif p_type == DEBYEMATERIAL + DISPERSIVEMATERIAL + MATERIAL:
            prop = CSPropDebyeMaterial(pset, no_init=no_init, **kw)

        return prop

    @staticmethod
    def fromTypeName(type_str, pset, no_init=False, **kw):
        """ fromTypeName(type_str, pset, no_init=False, **kw)

        Create a property specified by the `type_str`

        :param type_str: Property type name string
        :param pset: ParameterSet to assign to the new primitive
        :param no_init: do not create an actual C++ instance
        """
        if no_init and len(kw)!=0:
            raise Exception('Unable to not init and set property options!')
        prop = None
        if type_str=='Material':
            prop = CSPropMaterial(pset, no_init=no_init, **kw)
        elif type_str=='LumpedElement':
            prop = CSPropLumpedElement(pset, no_init=no_init, **kw)
        elif type_str=='Metal':
            prop = CSPropMetal(pset, no_init=no_init, **kw)
        elif type_str=='ConductingSheet':
            prop = CSPropConductingSheet(pset, no_init=no_init, **kw)
        elif type_str=='Excitation':
            prop = CSPropExcitation(pset, no_init=no_init, **kw)
        elif type_str=='ProbeBox':
            prop = CSPropProbeBox(pset, no_init=no_init, **kw)
        elif type_str=='DumpBox':
            prop = CSPropDumpBox(pset, no_init=no_init, **kw)
        elif type_str=='LorentzMaterial':
            prop = CSPropLorentzMaterial(pset, no_init=no_init, **kw)
        elif type_str=='DebyeMaterial':
            prop = CSPropDebyeMaterial(pset, no_init=no_init, **kw)
        return prop

    @staticmethod
    cdef fromPtr(_CSProperties  *ptr):
        if ptr == NULL:
            return None
        cdef CSProperties prop
        prop = CSProperties._instances.get(<uintptr_t>ptr, None)
        if prop is not None:
            return prop
        prop = CSProperties.fromType(ptr.GetType(), pset=None, no_init=True)
        prop._SetPtr(ptr)
        return prop

    def __init__(self, ParameterSet pset, *args, no_init=False, **kw):
        if no_init:
            self.thisptr = NULL
            return
        if not self.thisptr:
            raise Exception("Error, cannot create CSProperties (protected)")

        self._SetPtr(self.thisptr)

        alpha = 255
        if 'alpha' in kw:
            alpha = kw['alpha']
            del kw['alpha']

        if 'color' in kw:
            self.SetColor(kw['color'], alpha)
            del kw['color']

        if 'name' in kw:
            self.SetName(kw['name'])
            del kw['name']

        if len(kw)!=0:
            raise Exception('Unknown keywords: {}'.format(kw))

    cdef _SetPtr(self, _CSProperties *ptr):
        if self.thisptr != NULL and self.thisptr != ptr:
            raise Exception('Different C++ class pointer already assigned to python wrapper class!')
        self.thisptr = ptr
        CSProperties._instances[<uintptr_t>self.thisptr] = self

    def GetCopy(self, incl_prim=False):
        """
        Get a copy of this property

        :param incl_prim: Include a copy of all primitive
        :return: CSProperties -- Copy of this property
        """
        ptr = self.thisptr.GetCopy(incl_prim)
        return CSProperties.fromPtr(ptr)

    copy = GetCopy

    def GetQtyPrimitives(self):
        """
        Get the number of primitives assigned to this property

        :returns: int -- Number of primitives
        """
        return self.thisptr.GetQtyPrimitives()

    def GetPrimitive(self, index):
        if index<0:
            index = index%self.thisptr.GetQtyPrimitives()
        return self.__GetPrimitive(index)

    def GetAllPrimitives(self):
        return [self.GetPrimitive(n) for n in range(self.GetQtyPrimitives())]

    cdef __GetPrimitive(self, size_t index):
        return CSPrimitives.fromPtr(self.thisptr.GetPrimitive(index))

    def GetType(self):
        """ Get the type of the property

        :returns: int -- Type ID of this property
        """
        return self.thisptr.GetType()

    def GetTypeString(self):
        """ Get the type of the property as a string

        :returns: str -- Type name of this property type
        """
        return self.thisptr.GetTypeString().decode('UTF-8')

    def GetParameterSet(self):
        """
        Get the parameter set assigned to this class
        """
        return ParameterSet.fromPtr(self.thisptr.GetParameterSet())

    def GetMaterial(self):
        return self.thisptr.GetMaterial()

    def GetID(self):
        return self.thisptr.GetID()

    def SetName(self, name):
        """ SetName(name)

        Set the name of this property

        :params name: str -- Name for this property
        """
        self.thisptr.SetName(name.encode('UTF-8'))

    def GetName(self):
        """
        Get the name of this property

        :returns: str -- Name for this property
        """
        return self.thisptr.GetName().decode('UTF-8')

    def SetColor(self, color, alpha=255):
        """ SetColor(color, alpha=255)

        Set the fill and edge color for this property.

        :param color: color value (e.g. 'red','#1234ab', (50, 128, 200))
        :param alpha: opacity value (0 .. 255)
        """
        self.SetFillColor(color, alpha)
        self.SetEdgeColor(color, alpha)

    def SetFillColor(self, color, alpha=255):
        """ SetFillColor(color, alpha=255)

        Set the fill color for this property.

        :param color: color value (e.g. 'red','#1234ab', (50, 128, 200))
        :param alpha: opacity value (0.0 .. 255)
        """
        rgb = hex2color(color)
        self.thisptr.SetFillColor(rgb[0], rgb[1], rgb[2], alpha)

    def GetFillColor(self):
        """
        Get the fill color of this property

        :returns: (R,G,B,a) -- tuple for red, green, blue, alpha
        """
        cdef RGBa rgba
        rgba = self.thisptr.GetFillColor()
        return (rgba.R, rgba.G, rgba.B, rgba.a)

    def SetEdgeColor(self, color, alpha=255):
        """ SetColor(color, alpha=255)

        Set the edge color for this property.

        :param color: color value (e.g. 'red','#1234ab', (50, 128, 200))
        :param alpha: opacity value (0 .. 255)
        """
        rgb = hex2color(color)
        self.thisptr.SetEdgeColor(rgb[0], rgb[1], rgb[2], alpha)

    def GetEdgeColor(self):
        """
        Get the edge color of this property

        :returns: (R,G,B,a) -- tuple for red, green, blue, alpha
        """
        cdef RGBa rgba
        rgba = self.thisptr.GetEdgeColor()
        return (rgba.R, rgba.G, rgba.B, rgba.a)

    def GetVisibility(self):
        return self.thisptr.GetVisibility()

    def SetVisibility(self, val):
        self.thisptr.SetVisibility(val)

    def ExistAttribute(self, name):
        """ ExistAttribute(name)

        Check if an attribute with the given `name` exists

        :param name: str -- Attribute name
        :returns: bool
        """
        return self.thisptr.ExistAttribute(name.encode('UTF-8'))

    def GetAttributeValue(self, name):
        """ GetAttributeValue(name)

        Get the value of the attribute with the given `name`

        :param name: str -- Attribute name
        :returns: str -- Attribute value
        """
        return self.thisptr.GetAttributeValue(name.encode('UTF-8')).decode('UTF-8')

    def SetAttributeValue(self, name, val):
        """ SetAttributeValue(name, val)

        Set a new or change an existing attribute

        :param name: str -- Attribute name
        :param val: str -- Attribute value
        """
        self.thisptr.SetAttributeValue(name.encode('UTF-8'), val.encode('UTF-8'))

    AddAttribute = SetAttributeValue

    def RemoveAttribute(self, name):
        """ RemoveAttribute(name)

        Remove an existing attribute

        :param name: str -- Attribute name
        """
        self.thisptr.RemoveAttribute(name.encode('UTF-8'))

    def GetAttributeNames(self):
        names = self.thisptr.GetAttributeNames()
        attr_names = self.thisptr.GetAttributeNames()
        return tuple([name.decode('UTF-8') for name in attr_names])

    def GetAttributes(self):
        attr = dict()
        for name in self.GetAttributeNames():
            attr[name] = self.GetAttributeValue(name)
        return attr

    def AddPoint(self, coord, **kw):
        """ AddPoint(coord, **kw)

        Add a point to this property.

        See Also
        --------
        CSXCAD.CSPrimitives.CSPrimPoint : See here for details on primitive arguments
        """
        return self.__CreatePrimitive(c_CSPrimitives.POINT, coord=coord, **kw)

    def AddBox(self, start, stop, **kw):
        """ AddBox(start, stop, **kw)

        Add a box to this property.

        See Also
        --------
        CSXCAD.CSPrimitives.CSPrimBox : See here for details on primitive arguments
        """
        return self.__CreatePrimitive(c_CSPrimitives.BOX, start=start, stop=stop, **kw)

    def AddCylinder(self, start, stop, radius, **kw):
        """ AddCylinder(start, stop, radius, **kw)

        Add a cylinder to this property.

        See Also
        --------
        CSXCAD.CSPrimitives.CSPrimCylinder : See here for details on primitive arguments
        """
        return self.__CreatePrimitive(c_CSPrimitives.CYLINDER, start=start, stop=stop, radius=radius, **kw)

    def AddCylindricalShell(self, start, stop, radius, shell_width, **kw):
        """ AddCylindricalShell(start, stop, radius, shell_width, **kw)

        Add a cylindrical shell to this property.

        See Also
        --------
        CSXCAD.CSPrimitives.CSPrimCylindricalShell : See here for details on primitive arguments
        """
        return self.__CreatePrimitive(c_CSPrimitives.CYLINDRICALSHELL, start=start, stop=stop, radius=radius, shell_width=shell_width, **kw)

    def AddSphere(self, center, radius, **kw):
        """ AddSphere(center, radius, **kw)

        Add a sphere to this property.

        See Also
        --------
        CSXCAD.CSPrimitives.CSPrimSphere : See here for details on primitive arguments
        """
        return self.__CreatePrimitive(c_CSPrimitives.SPHERE, center=center, radius=radius, **kw)

    def AddSphericalShell(self, center, radius, shell_width, **kw):
        """ AddSphericalShell(center, radius, shell_width, **kw)

        Add a spherical shell to this property.

        See Also
        --------
        CSXCAD.CSPrimitives.CSPrimSphericalShell : See here for details on primitive arguments
        """
        return self.__CreatePrimitive(c_CSPrimitives.SPHERICALSHELL, center=center, radius=radius, shell_width=shell_width, **kw)

    def AddPolygon(self, points, norm_dir, elevation, **kw):
        """ AddPolygon(points, norm_dir, elevation, **kw)

        Add a polygon to this property.

        See Also
        --------
        CSXCAD.CSPrimitives.CSPrimPolygon : See here for details on primitive arguments
        """
        return self.__CreatePrimitive(c_CSPrimitives.POLYGON, points=points, norm_dir=norm_dir, elevation=elevation, **kw)

    def AddLinPoly(self, points, norm_dir, elevation, length, **kw):
        """ AddLinPoly(points, norm_dir, elevation, length, **kw)

        Add a linear extruded polygon to this property.

        See Also
        --------
        CSXCAD.CSPrimitives.CSPrimLinPoly : See here for details on primitive arguments
        """
        return self.__CreatePrimitive(c_CSPrimitives.LINPOLY, points=points, norm_dir=norm_dir, elevation=elevation, length=length, **kw)

    def AddRotPoly(self, points, norm_dir, elevation, rot_axis, angle, **kw):
        """ AddRotPoly(points, norm_dir, elevation, rot_axis, angle, **kw)

        Add a rotated polygon to this property.

        See Also
        --------
        CSXCAD.CSPrimitives.CSPrimRotPoly : See here for details on primitive arguments
        """
        return self.__CreatePrimitive(c_CSPrimitives.ROTPOLY, points=points, norm_dir=norm_dir, elevation=elevation, rot_axis=rot_axis, angle=angle, **kw)

    def AddCurve(self, points, **kw):
        """ AddCurve(points, **kw)

        Add a curve to this property.

        See Also
        --------
        CSXCAD.CSPrimitives.CSPrimCurve : See here for details on primitive arguments
        """
        return self.__CreatePrimitive(c_CSPrimitives.CURVE, points=points, **kw)

    def AddWire(self, points, radius, **kw):
        """ AddWire(points, radius, **kw)

        Add a wire to this property.

        See Also
        --------
        CSXCAD.CSPrimitives.CSPrimWire : See here for details on primitive arguments
        """
        return self.__CreatePrimitive(c_CSPrimitives.WIRE, points=points, radius=radius, **kw)

    def AddPolyhedron(self, **kw):
        """ AddPolyhedron(**kw)

        Add a polyhedron to this property.

        See Also
        --------
        CSXCAD.CSPrimitives.CSPrimPolyhedron : See here for details on primitive arguments
        """
        return self.__CreatePrimitive(c_CSPrimitives.POLYHEDRON, **kw)

    def AddPolyhedronReader(self, filename, **kw):
        """ AddPolyhedronReader(filename, **kw)

        Add a polyhedron from file to this property.

        See Also
        --------
        CSXCAD.CSPrimitives.CSPrimPolyhedronReader : See here for details on primitive arguments
        """
        return self.__CreatePrimitive(c_CSPrimitives.POLYHEDRONREADER, filename=filename, **kw)

    def __CreatePrimitive(self, prim_type, **kw):
        pset = self.GetParameterSet()
        prim = CSPrimitives.fromType(prim_type, pset, self, **kw)
        if prim is None:
            raise Exception('CreatePrimitive: Unknown primitive type requested!')
        return prim

###############################################################################
cdef class CSPropMaterial(CSProperties):
    """ General material property

    This is a material with options to define relative electric permittivity
    (`epsilon`), relative magnetic permeability (`mue`), electric conductivity
    (`kappa`), magnetic conductivity (`sigma`) and `density`.

    :params epsilon: scalar or vector - relative electric permittivity
    :params mue:     scalar or vector - relative magnetic permeability
    :params kappa:   scalar or vector - electric conductivity
    :params sigma:   scalar or vector - magnetic conductivity
    :params density: float            - Density
    """
    def __init__(self, ParameterSet pset, *args, no_init=False, **kw):
        if no_init:
            super(CSPropMaterial, self).__init__(pset, no_init=True)
            return
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropMaterial(pset.thisptr)

        for k in list(kw.keys()):
            mat_props = {}
            if k in ['epsilon', 'mue', 'kappa', 'sigma', 'density']:
                mat_props[k] = kw[k]
                del kw[k]
            if mat_props:
                self.SetMaterialProperty(**mat_props)

        super(CSPropMaterial, self).__init__(pset, *args, **kw)

    def SetIsotropy(self, val):
        """ SetIsotropy(val)

        Set isotropy status for this material.

        :param val: bool -- enable/disable isotropy
        """
        (<_CSPropMaterial*>self.thisptr).SetIsotropy(val)

    def GetIsotropy(self):
        """
        Get isotropy status for this material.

        :returns: bool -- isotropy status
        """
        return (<_CSPropMaterial*>self.thisptr).GetIsotropy()

    def SetMaterialProperty(self, **kw):
        """ SetMaterialProperty(**kw)
        Set the material properties.

        :params epsilon: scalar or vector - relative electric permittivity
        :params mue:     scalar or vector - relative magnetic permeability
        :params kappa:   scalar or vector - electric conductivity
        :params sigma:   scalar or vector - magnetic conductivity
        :params density: float            - Density
        """
        for prop_name in kw:
            val = kw[prop_name]
            if prop_name == 'density':
                (<_CSPropMaterial*>self.thisptr).SetDensity(val)
                continue
            if type(val)==float or type(val)==int:
                self._SetMaterialPropertyDir(prop_name, 0, val)
                continue
            assert len(val)==3, 'SetMaterialProperty: "{}" must be a list or array of length 3'.format(prop_name)
            for n in range(3):
                self._SetMaterialPropertyDir(prop_name, n, val[n])

    def _SetMaterialPropertyDir(self, prop_name, ny, val):
        if prop_name=='epsilon':
            return (<_CSPropMaterial*>self.thisptr).SetEpsilon(val, ny)
        elif prop_name=='mue':
            return (<_CSPropMaterial*>self.thisptr).SetMue(val, ny)
        elif prop_name=='kappa':
            return (<_CSPropMaterial*>self.thisptr).SetKappa(val, ny)
        elif prop_name=='sigma':
            return (<_CSPropMaterial*>self.thisptr).SetSigma(val, ny)
        else:
            raise Exception('SetMaterialPropertyDir: Error, unknown material property')

    def SetMaterialWeight(self, **kw):
        """ SetMaterialWeight(**kw)

        Set the material weighting function(s)

        The functions can use the variables:
        `x`,`y`,`z`
        `rho` for the distance to z-axis
        `r`   for the distance to origin
        `a`   for alpha or phi (as in cylindrical and spherical coord systems)
        `t`   for theta (as in the spherical coord system

        all these variables are not weighted with the drawing unit defined by
        the grid

        :params epsilon: str or str-vector - relative electric permittivity
        :params mue:     str or str-vector - relative magnetic permeability
        :params kappa:   str or str-vector - electric conductivity
        :params sigma:   str or str-vector - magnetic conductivity
        :params density: str               - Density
        """
        for prop_name in kw:
            val = kw[prop_name]
            if prop_name == 'density':
                (<_CSPropMaterial*>self.thisptr).SetDensityWeightFunction(val.encode('UTF-8'))
                continue
            if type(val)==str:
                self._SetMaterialWeightDir(prop_name, 0, val)
                continue
            assert len(val)==3, 'SetMaterialWeight: "{}" must be a list or array of length 3'.format(prop_name)
            for n in range(3):
                self._SetMaterialWeightDir(prop_name, n, val[n])

    def _SetMaterialWeightDir(self, prop_name, ny, val):
        val = val.encode('UTF-8')
        if prop_name=='epsilon':
            return (<_CSPropMaterial*>self.thisptr).SetEpsilonWeightFunction(val, ny)
        elif prop_name=='mue':
            return (<_CSPropMaterial*>self.thisptr).SetMueWeightFunction(val, ny)
        elif prop_name=='kappa':
            return (<_CSPropMaterial*>self.thisptr).SetKappaWeightFunction(val, ny)
        elif prop_name=='sigma':
            return (<_CSPropMaterial*>self.thisptr).SetSigmaWeightFunction(val, ny)
        else:
            raise Exception('SetMaterialWeightDir: Error, unknown material property')

    def GetMaterialProperty(self, prop_name):
        """ SetMaterialProperty(prop_name)
        Get the material property with of type `prop_name`.

        :params prop_name: str -- material property type
        :returns: float for isotropic material and `density` or else (3,) array
        """
        if prop_name == 'density':
            return (<_CSPropMaterial*>self.thisptr).GetDensity()
        if (<_CSPropMaterial*>self.thisptr).GetIsotropy():
            return self._GetMaterialPropertyDir(prop_name, 0)
        val = np.zeros(3)
        for n in range(3):
            val[n] = self._GetMaterialPropertyDir(prop_name, n)
        return val

    def _GetMaterialPropertyDir(self, prop_name, ny):
        if prop_name=='epsilon':
            return (<_CSPropMaterial*>self.thisptr).GetEpsilon(ny)
        elif prop_name=='mue':
            return (<_CSPropMaterial*>self.thisptr).GetMue(ny)
        elif prop_name=='kappa':
            return (<_CSPropMaterial*>self.thisptr).GetKappa(ny)
        elif prop_name=='sigma':
            return (<_CSPropMaterial*>self.thisptr).GetSigma(ny)
        else:
            raise Exception('GetMaterialPropertyDir: Error, unknown material property')

    def GetMaterialWeight(self, prop_name):
        """ GetMaterialWeight(prop_name)
        Get the material weighting function(s).

        :params prop_name: str -- material property type
        :returns: str for isotropic material and `density` or else str array
        """
        if prop_name == 'density':
            return (<_CSPropMaterial*>self.thisptr).GetDensityWeightFunction().decode('UTF-8')
        if (<_CSPropMaterial*>self.thisptr).GetIsotropy():
            return self._GetMaterialWeightDir(prop_name, 0).decode('UTF-8')
        val = ['', '', '']
        for n in range(3):
            val[n] = self._GetMaterialWeightDir(prop_name, n).decode('UTF-8')
        return val

    def _GetMaterialWeightDir(self, prop_name, ny):
        if prop_name=='epsilon':
            return (<_CSPropMaterial*>self.thisptr).GetEpsilonWeightFunction(ny)
        elif prop_name=='mue':
            return (<_CSPropMaterial*>self.thisptr).GetMueWeightFunction(ny)
        elif prop_name=='kappa':
            return (<_CSPropMaterial*>self.thisptr).GetKappaWeightFunction(ny)
        elif prop_name=='sigma':
            return (<_CSPropMaterial*>self.thisptr).GetSigmaWeightFunction(ny)
        else:
            raise Exception('GetMaterialWeightDir: Error, unknown material property')


###############################################################################
cdef class CSPropLumpedElement(CSProperties):
    """
    Lumped element property.

    A lumped element can consist of a resistor `R`, capacitor `C` and inductance
    `L` active in a given direction `ny`.
    If Caps are enable, a small PEC plate is added to each
    end of the lumped element to ensure contact to a connecting line

    :param ny: int or str -- direction:  0,1,2 or 'x','y','z' for the orientation of the lumped element
    :param caps: bool     -- enable/disable caps
    :param R:  float      -- lumped resistance value
    :param C:  float      -- lumped capacitance value
    :param L:  float      -- lumped inductance values
    :param LEtype:  int      -- lumped element type
    """
    def __init__(self, ParameterSet pset, *args, no_init=False, **kw):
        if no_init:
            super(CSPropLumpedElement, self).__init__(pset, no_init=True)
            return
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropLumpedElement(pset.thisptr)

        b_LEtype_found = False
        for k in kw:
            if k=='R':
                self.SetResistance(kw[k])
            elif k=='L':
                self.SetInductance(kw[k])
            elif k=='C':
                self.SetCapacity(kw[k])
            elif k=='ny':
                self.SetDirection(kw[k])
            elif k=='caps':
                self.SetCaps(kw[k])
            elif k=='LEtype':
                b_LEtype_found = True
                # Consider adding some logic here, to prevent silly errors. Default parallel and all that
                self.SetLEtype(kw[k])

        # If the lumped element type is not found, assume parallel
        if (not b_LEtype_found):
            self.SetLEtype(LE_PARALLEL)

        for k in ['R', 'L', 'C', 'ny', 'caps','LEtype']:
            if k in kw:
                del kw[k]

        super(CSPropLumpedElement, self).__init__(pset, *args, **kw)

    def SetResistance(self, val):
        """ SetResistance(val)
        """
        (<_CSPropLumpedElement*>self.thisptr).SetResistance(val)

    def GetResistance(self):
        """ GetResistance()
        """
        return (<_CSPropLumpedElement*>self.thisptr).GetResistance()

    def SetCapacity(self, val):
        """ SetCapacity(val)
        """
        (<_CSPropLumpedElement*>self.thisptr).SetCapacity(val)

    def GetCapacity(self):
        """ GetCapacity()
        """
        return (<_CSPropLumpedElement*>self.thisptr).GetCapacity()

    def SetInductance(self, val):
        """ SetInductance(val)
        """
        (<_CSPropLumpedElement*>self.thisptr).SetInductance(val)

    def GetInductance(self):
        """ GetInductance()
        """
        return (<_CSPropLumpedElement*>self.thisptr).GetInductance()

    def SetDirection(self, ny):
        """ SetDirection(ny)
        """
        (<_CSPropLumpedElement*>self.thisptr).SetDirection(CheckNyDir(ny))

    def GetDirection(self):
        """ GetDirection()
        """
        return (<_CSPropLumpedElement*>self.thisptr).GetDirection()

    def SetCaps(self, val):
        """ SetCaps(val)
        """
        (<_CSPropLumpedElement*>self.thisptr).SetCaps(val)

    def GetCaps(self):
        """ GetCaps()
        """
        return (<_CSPropLumpedElement*>self.thisptr).GetCaps()==1

    def SetLEtype(self, val):
        """ SetLEtype(val)
        """
        (<_CSPropLumpedElement*>self.thisptr).SetLEtype(val)

    def GetLEtype(self):
        """ GetLEtype()
        """
        return (<_CSPropLumpedElement*>self.thisptr).GetLEtype()

###############################################################################
cdef class CSPropMetal(CSProperties):
    """ Metal property

    A metal property is usually modeled as a perfect electric conductor (PEC).
    It does not have any further arguments.

    See Also
    --------
    CSPropConductingSheet : For all lossy conductor model with a finite conductivity
    """
    def __init__(self, ParameterSet pset, *args, no_init=False, **kw):
        if no_init:
            super(CSPropMetal, self).__init__(pset, no_init=True)
            return
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropMetal(pset.thisptr)
        super(CSPropMetal, self).__init__(pset, *args, **kw)

###############################################################################
cdef class CSPropConductingSheet(CSPropMetal):
    """ Conducting sheet model property

    A conducting sheet is a model for a thin conducting metal sheet.

    Notes
    -----
    Only 2D primitives (sheets) should be added to this property

    :param conductivity: float -- finite conductivity e.g. 56e6 (S/m)
    :param thickness:    float -- finite modeled thickness (e.g. 18e-6)
    """
    def __init__(self, ParameterSet pset, *args, no_init=False, **kw):
        if no_init:
            super(CSPropConductingSheet, self).__init__(pset, no_init=True)
            return
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropConductingSheet(pset.thisptr)

        if 'conductivity' in kw:
            self.SetConductivity(kw['conductivity'])
            del kw['conductivity']
        if 'thickness' in kw:
            self.SetThickness(kw['thickness'])
            del kw['thickness']
        super(CSPropConductingSheet, self).__init__(pset, *args, **kw)

    def SetConductivity(self, val):
        """ SetConductivity(val)
        """
        (<_CSPropConductingSheet*>self.thisptr).SetConductivity(val)

    def GetConductivity(self):
        """ GetConductivity()
        """
        return (<_CSPropConductingSheet*>self.thisptr).GetConductivity()

    def SetThickness(self, val):
        """ SetThickness(val)
        """
        (<_CSPropConductingSheet*>self.thisptr).SetThickness(val)

    def GetThickness(self):
        """ GetThickness()
        """
        return (<_CSPropConductingSheet*>self.thisptr).GetThickness()

###############################################################################
cdef class CSPropExcitation(CSProperties):
    """ Excitation property

    An excitation property is defined to define the (field) sources.

    Depending on the EM modeling tool there exist different excitation types
    with different meanings.

    :param exc_type: int -- excitation type (see SetExcitation)
    :param exc_val: (3,) array like -- set the excitation vector
    :param delay: float -- excitation delay in seconds
    """
    def __init__(self, ParameterSet pset, *args, no_init=False, **kw):
        if no_init:
            super(CSPropExcitation, self).__init__(pset, no_init=True)
            return
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropExcitation(pset.thisptr)

        if 'exc_type' in kw:
            self.SetExcitType(kw['exc_type'])
            del kw['exc_type']
        if 'exc_val' in kw:
            self.SetExcitation(kw['exc_val'])
            del kw['exc_val']
        if 'delay' in kw:
            self.SetDelay(kw['delay'])
            del kw['delay']
        super(CSPropExcitation, self).__init__(pset, *args, **kw)

    def SetExcitType(self, val):
        """ SetExcitType(val)

        :param val: int -- excitation type (see above)
        """
        (<_CSPropExcitation*>self.thisptr).SetExcitType(val)

    def GetExcitType(self):
        """ GetExcitType()
        Get the excitation type.

        :return: int -- excitation type (see above)
        """
        return (<_CSPropExcitation*>self.thisptr).GetExcitType()


    def SetEnabled(self, val):
        """ Enable/Disable the excitation"""
        (<_CSPropExcitation*>self.thisptr).SetEnabled(val)

    def GetEnabled(self):
        """ Get enable/disable state of the excitation"""
        return (<_CSPropExcitation*>self.thisptr).GetEnabled()

    def SetExcitation(self, val):
        """ SetExcitation(val)

        openEMS excitation types:

        * 0 : E-field soft excitation
        * 1 : E-field hard excitation
        * 2 : H-field soft excitation
        * 3 : H-field hard excitation
        * 10 : plane wave excitation

        :param val: (3,) array -- excitation vector
        """
        assert len(val)==3, "Error, excitation vector must be of dimension 3"
        for n in range(3):
            (<_CSPropExcitation*>self.thisptr).SetExcitation(val[n], n)

    def GetExcitation(self):
        """ GetExcitation()

        :returns: (3,) array -- excitation vector
        """
        val = np.zeros(3)
        for n in range(3):
            val[n] = (<_CSPropExcitation*>self.thisptr).GetExcitation(n)
        return val

    def SetPropagationDir(self, val):
        """ SetPropagationDir(val)

        Set the propagation direction, e.g. for a plane wave excitation

        :param val: (3,) array -- propagation vector
        """
        assert len(val)==3, "Error, excitation vector must be of dimension 3"
        for n in range(3):
            (<_CSPropExcitation*>self.thisptr).SetPropagationDir(val[n], n)

    def GetPropagationDir(self):
        """ GetPropagationDir()

        Get the propagation direction, e.g. of a plane wave excitation

        :returns: (3,) array -- propagation vector
        """
        val = np.zeros(3)
        for n in range(3):
            val[n] = (<_CSPropExcitation*>self.thisptr).GetPropagationDir(n)
        return val

    def SetWeightFunction(self, func):
        """SetWeightFunction(func)

        Set the weigthing function for the excitation.

        """

        assert len(func)==3, 'Error, excitation weighting function must be an array of length 3'
        for n in range(3):
            assert type(func[n]) is str, 'Error, excitation weighting function must be a string'
            (<_CSPropExcitation*>self.thisptr).SetWeightFunction(func[n].encode('UTF-8'), n)

    def GetWeightFunction(self):
        """GetWeightFunction()

        Get the weigthing function for the excitation.

        :returns: 3 element list of strings
        """

        func = [None]*3
        for n in range(3):
            func[n] = (<_CSPropExcitation*>self.thisptr).GetWeightFunction(n).decode('UTF-8')
        return func

    def SetFrequency(self, val):
        """ SetFrequency(val)

        Frequency for numerical phase velocity compensation (optional)

        :param val: float -- Frequency
        """
        (<_CSPropExcitation*>self.thisptr).SetFrequency(val)

    def GetFrequency(self):
        """ GetFrequency()
        """
        return (<_CSPropExcitation*>self.thisptr).GetFrequency()

    def SetDelay(self, val):
        """ SetDelay(val)

        Set signal delay for this property.

        :param val: float -- Signal delay
        """
        (<_CSPropExcitation*>self.thisptr).SetDelay(val)

    def GetDelay(self):
        """ GetDelay()
        """
        return (<_CSPropExcitation*>self.thisptr).GetDelay()

###############################################################################
cdef class CSPropProbeBox(CSProperties):
    """
    Probe propery.

    Depending on the EM modeling tool there exist different probe types
    with different meanings.

    openEMS probe types:

    * 0 : for voltage probing
    * 1 : for current probing
    * 2 : for E-field probing
    * 3 : for H-field probing
    * 10 : for waveguide voltage mode matching
    * 11 : for waveguide current mode matching

    :param p_type:       probe type (default is 0)
    :param weight:       weighting factor (default is 1)
    :param frequency:    dump in the frequency domain at the given samples (in Hz)
    :param mode_function: A mode function (used only with type 3/4 in openEMS)
    :param norm_dir:     necessary for current probing box with dimension not 2
    """
    def __init__(self, ParameterSet pset, *args, no_init=False, **kw):
        if no_init:
            super(CSPropProbeBox, self).__init__(pset, no_init=True)
            return
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropProbeBox(pset.thisptr)

        if 'p_type' in kw:
            self.SetProbeType(kw['p_type'])
            del kw['p_type']
        if 'weight' in kw:
            self.SetWeighting(kw['weight'])
            del kw['weight']
        if 'norm_dir' in kw:
            self.SetNormalDir(kw['norm_dir'])
            del kw['norm_dir']
        if 'frequency' in kw:
            self.SetFrequency(kw['frequency'])
            del kw['frequency']
        if 'mode_function' in kw:
            self.SetModeFunction(kw['mode_function'])
            del kw['mode_function']
        super(CSPropProbeBox, self).__init__(pset, *args, **kw)

    def SetProbeType(self, val):
        """ SetProbeType(val)
        """
        (<_CSPropProbeBox*>self.thisptr).SetProbeType(val)

    def GetProbeType(self):
        """ GetProbeType()
        """
        return (<_CSPropProbeBox*>self.thisptr).GetProbeType()

    def SetWeighting(self, val):
        """ SetWeighting(val)
        """
        (<_CSPropProbeBox*>self.thisptr).SetWeighting(val)

    def GetWeighting(self):
        """ GetWeighting()
        """
        return (<_CSPropProbeBox*>self.thisptr).GetWeighting()

    def SetNormalDir(self, val):
        """ SetNormalDir(val)
        """
        (<_CSPropProbeBox*>self.thisptr).SetNormalDir(val)

    def GetNormalDir(self):
        """ GetNormalDir()
        """
        return (<_CSPropProbeBox*>self.thisptr).GetNormalDir()

    def AddFrequency(self, freq):
        """ AddFrequency(freq)
        """
        if np.isscalar(freq):
            (<_CSPropProbeBox*>self.thisptr).AddFDSample(freq)
        else:
            for f in freq:
                (<_CSPropProbeBox*>self.thisptr).AddFDSample(f)

    def ClearFrequency(self):
        """ ClearFrequency()
        """
        (<_CSPropProbeBox*>self.thisptr).ClearFDSamples()

    def SetFrequency(self, freq):
        """ SetFrequency(freq)
        """
        self.ClearFrequency()
        self.AddFrequency(freq)

    def GetFrequency(self):
        """ GetFrequency
        """
        cdef vector[double]* _freq = (<_CSPropProbeBox*>self.thisptr).GetFDSamples()
        Nf = _freq.size()
        freq = np.zeros(Nf)
        for n in range(Nf):
            freq[n] = _freq.at(n)
        return freq

    def GetFrequencyCount(self):
        """ GetFrequencyCount()
        """
        return (<_CSPropProbeBox*>self.thisptr).CountFDSamples()

    def SetModeFunction(self, mode_fun):
        """ SetModeFunction(mode_fun)
        """
        assert len(mode_fun)==3, 'SetModeFunction: mode_fun must be list of length 3'
        self.AddAttribute('ModeFunctionX', str(mode_fun[0]))
        self.AddAttribute('ModeFunctionY', str(mode_fun[1]))
        self.AddAttribute('ModeFunctionZ', str(mode_fun[2]))

###############################################################################
cdef class CSPropDumpBox(CSPropProbeBox):
    """
    Dump property to create field dumps.

    Depending on the EM modeling tool there exist different dump types, dump
    modes and file types with different meanings.

    openEMS dump types:

    * 0  : for E-field time-domain dump (default)
    * 1  : for H-field time-domain dump
    * 2  : for electric current time-domain dump
    * 3  : for total current density (rot(H)) time-domain dump

    * 10 : for E-field frequency-domain dump
    * 11 : for H-field frequency-domain dump
    * 12 : for electric current frequency-domain dump
    * 13 : for total current density (rot(H)) frequency-domain dump

    * 20 : local SAR frequency-domain dump
    * 21 :  1g averaging SAR frequency-domain dump
    * 22 : 10g averaging SAR frequency-domain dump

    * 29 : raw data needed for SAR calculations (electric field FD, cell volume, conductivity and density)

    openEMS dump modes:

    * 0 : no-interpolation
    * 1 : node-interpolation (default, see warning below)
    * 2 : cell-interpolation (see warning below)

    openEMS file types:

    * 0 : vtk-file  (default)
    * 1 : hdf5-file (easier to read by python, using h5py)

    :param dump_type: dump type (see above)
    :param dump_mode: dump mode (see above)
    :param file_type: file type (see above)
    :param frequency: specify a frequency vector (required for dump types >=10)
    :param sub_sampling:   field domain sub-sampling, e.g. '2,2,4'
    :param opt_resolution: field domain dump resolution, e.g. '10' or '10,20,5'

    Notes
    -----
    openEMS FDTD Interpolation abnormalities:

    * no-interpolation:
        fields are located in the mesh by the Yee-scheme, the mesh only
        specifies E- or H-Yee-nodes --> use node- or cell-interpolation or
        be aware of the offset
    * E-field dump & node-interpolation:
        normal electric fields on boundaries will have false amplitude due to
        forward/backward interpolation  in case of (strong) changes in material
        permittivity or on metal surfaces
        --> use no- or cell-interpolation
    * H-field dump & cell-interpolation:
        normal magnetic fields on boundaries will have false amplitude due to
        forward/backward interpolation in case of (strong) changes in material
        permeability --> use no- or node-interpolation
    """
    def __init__(self, ParameterSet pset, *args, no_init=False, **kw):
        if no_init:
            super(CSPropDumpBox, self).__init__(pset, no_init=True)
            return
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropDumpBox(pset.thisptr)

        if 'dump_type' in kw:
            self.SetDumpType(kw['dump_type'])
            del kw['dump_type']
        if 'dump_mode' in kw:
            self.SetDumpMode(kw['dump_mode'])
            del kw['dump_mode']
        if 'file_type' in kw:
            self.SetFileType(kw['file_type'])
            del kw['file_type']
        if 'opt_resolution' in kw:
            self.SetOptResolution(kw['opt_resolution'])
            del kw['opt_resolution']
        if 'sub_sampling' in kw:
            self.SetSubSampling(kw['sub_sampling'])
            del kw['sub_sampling']
        super(CSPropDumpBox, self).__init__(pset, *args, **kw)

    def SetDumpType(self, val):
        """ SetDumpType(val)
        """
        (<_CSPropDumpBox*>self.thisptr).SetDumpType(val)

    def GetDumpType(self):
        """ GetDumpType()
        """
        return (<_CSPropDumpBox*>self.thisptr).GetDumpType()

    def SetDumpMode(self, val):
        """ SetDumpMode(val)
        """
        (<_CSPropDumpBox*>self.thisptr).SetDumpMode(val)

    def GetDumpMode(self):
        """ GetDumpMode()
        """
        return (<_CSPropDumpBox*>self.thisptr).GetDumpMode()

    def SetFileType(self, val):
        """ SetFileType(val)
        """
        (<_CSPropDumpBox*>self.thisptr).SetFileType(val)

    def GetFileType(self):
        """ GetFileType()
        """
        return (<_CSPropDumpBox*>self.thisptr).GetFileType()

    def SetOptResolution(self, val):
        """ SetOptResolution(val)
        """
        assert len(val)==3, 'SetOptResolution: value must be list or array of length 3'
        for n in range(3):
            (<_CSPropDumpBox*>self.thisptr).SetOptResolution(n, val[n])

    def GetOptResolution(self):
        """ GetOptResolution()
        """
        val = np.zeros(3)
        for n in range(3):
            val[n] = (<_CSPropDumpBox*>self.thisptr).GetOptResolution(n)
        return val

    def SetSubSampling(self, val):
        """ SetSubSampling(val)
        """
        assert len(val)==3, "SetSubSampling: 'val' must be a list or array of length 3"
        for n in range(3):
            (<_CSPropDumpBox*>self.thisptr).SetSubSampling(n, val[n])

    def GetSubSampling(self):
        """ GetSubSampling()
        """
        val = np.zeros(3)
        for n in range(3):
            val[n] = (<_CSPropDumpBox*>self.thisptr).GetSubSampling(n)
        return val

###############################################################################
cdef class CSPropDispersiveMaterial(CSPropMaterial):
    def __init__(self, ParameterSet pset, *args, no_init=False, **kw):
        if no_init:
            super(CSPropDispersiveMaterial, self).__init__(pset, no_init=True)
            return
        if not self.thisptr:
            raise Exception("Error, cannot create CSPropDispersiveMaterial (protected)")
        if 'order' in kw:
            self.SetDispersionOrder(kw['order'])
            del kw['order']
        super(CSPropDispersiveMaterial, self).__init__(pset, *args, **kw)

    def GetDispersionOrder(self):
        return  (<_CSPropDispersiveMaterial*>self.thisptr).GetDispersionOrder()

    def SetDispersionOrder(self, val):
        return  (<_CSPropDispersiveMaterial*>self.thisptr).SetDispersionOrder(val)

    def _CheckOrder(self, order):
        if order<0 or order>=(<_CSPropDispersiveMaterial*>self.thisptr).GetDispersionOrder():
            raise IndexError('Invalid dispersive media order requested')

    def GetDispersiveMaterialProperty(self, prop_name, order):
        """ GetDispersiveMaterialProperty(prop_name)
        Get the dispersive material property with of type `prop_name` and for order `order`.

        :params prop_name: str -- material property type
        :params order:     int -- material property order
        :returns: float for isotropic material or else (3,) array
        """
        self._CheckOrder(order)
        if (<_CSPropMaterial*>self.thisptr).GetIsotropy():
            return self._GetDispersiveMaterialPropertyDir(prop_name, order, 0)
        val = np.zeros(3)
        for n in range(3):
            val[n] = self._GetDispersiveMaterialPropertyDir(prop_name, order, n)
        return val

    def _GetDispersiveMaterialPropertyDir(self, prop_name, order, ny):
        raise Exception('GetDispersiveMaterialPropertyDir: Error, unknown material property')

    def SetDispersiveMaterialProperty(self, order, **kw):
        """ SetMaterialProperty(**kw)
        Set the material properties.

        :params epsilon: scalar or vector - relative electric permittivity
        :params mue:     scalar or vector - relative magnetic permeability
        :params kappa:   scalar or vector - electric conductivity
        :params sigma:   scalar or vector - magnetic conductivity
        :params density: float            - Density
        """
        self._CheckOrder(order)
        for prop_name in kw:
            val = kw[prop_name]
            if type(val)==float or type(val)==int:
                self._SetDispersiveMaterialPropertyDir(prop_name, order, 0, val)
                continue
            assert len(val)==3, 'SetMaterialProperty: "{}" must be a list or array of length 3'.format(prop_name)
            for n in range(3):
                self._SetDispersiveMaterialPropertyDir(prop_name, order, n, val[n])

    def _SetDispersiveMaterialPropertyDir(self, prop_name, order, ny, val):
        raise Exception('SetMaterialPropertyDir: Error, unknown material property')

    def GetDispersiveMaterialWeight(self, prop_name, order):
        """ GetMaterialWeight(prop_name)
        Get the material weighting function(s).

        :params prop_name: str -- material property type
        :returns: str for isotropic material and `density` or else str array
        """
        self._CheckOrder(order)
        if (<_CSPropMaterial*>self.thisptr).GetIsotropy():
            return self._GetDispersiveMaterialWeightDir(prop_name, order, 0).decode('UTF-8')
        val = ['', '', '']
        for n in range(3):
            val[n] = self._GetDispersiveMaterialWeightDir(prop_name, order, n).decode('UTF-8')
        return val

    def _GetDispersiveMaterialWeightDir(self, prop_name, order, ny):
        raise Exception('GetMaterialWeightDir: Error, unknown material property')

    def SetDispersiveMaterialWeight(self, order, **kw):
        self._CheckOrder(order)
        for prop_name in kw:
            val = kw[prop_name]
            if type(val)==str:
                self._SetDispersiveMaterialWeightDir(prop_name, order, 0, val)
                continue
            assert len(val)==3, 'SetDispersiveMaterialWeight: "{}" must be a list or array of length 3'.format(prop_name)
            for n in range(3):
                self._SetDispersiveMaterialWeightDir(prop_name, order, n, val[n])

    def _SetDispersiveMaterialWeightDir(self, prop_name, order, ny, val):
        raise Exception('SetMaterialWeightDir: Error, unknown material property')

###############################################################################
cdef class CSPropLorentzMaterial(CSPropDispersiveMaterial):
    def __init__(self, ParameterSet pset, *args, no_init=False, **kw):
        if no_init:
            super(CSPropLorentzMaterial, self).__init__(pset, no_init=True)
            return
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropLorentzMaterial(pset.thisptr)

        super(CSPropLorentzMaterial, self).__init__(pset, *args, **kw)

    def _GetDispersiveMaterialPropertyDir(self, prop_name, order, ny):
        if prop_name == 'eps_plasma':
            return (<_CSPropLorentzMaterial*>self.thisptr).GetEpsPlasmaFreq(order, ny)
        elif prop_name == 'eps_pole_freq':
            return (<_CSPropLorentzMaterial*>self.thisptr).GetEpsLorPoleFreq(order, ny)
        elif prop_name == 'eps_relax':
            return (<_CSPropLorentzMaterial*>self.thisptr).GetEpsRelaxTime(order, ny)
        elif prop_name == 'mue_plasma':
            return (<_CSPropLorentzMaterial*>self.thisptr).GetMuePlasmaFreq(order, ny)
        elif prop_name == 'mue_pole_freq':
            return (<_CSPropLorentzMaterial*>self.thisptr).GetMueLorPoleFreq(order, ny)
        elif prop_name == 'mue_relax':
            return (<_CSPropLorentzMaterial*>self.thisptr).GetMueRelaxTime(order, ny)
        else:
            CSPropDispersiveMaterial._GetDispersiveMaterialPropertyDir(self, prop_name, order, ny)

    def _SetDispersiveMaterialPropertyDir(self, prop_name, order, ny, val):
        if prop_name == 'eps_plasma':
            return (<_CSPropLorentzMaterial*>self.thisptr).SetEpsPlasmaFreq(order, val, ny)
        elif prop_name == 'eps_pole_freq':
            return (<_CSPropLorentzMaterial*>self.thisptr).SetEpsLorPoleFreq(order, val, ny)
        elif prop_name == 'eps_relax':
            return (<_CSPropLorentzMaterial*>self.thisptr).SetEpsRelaxTime(order, val, ny)
        elif prop_name == 'mue_plasma':
            return (<_CSPropLorentzMaterial*>self.thisptr).SetMuePlasmaFreq(order, val, ny)
        elif prop_name == 'mue_pole_freq':
            return (<_CSPropLorentzMaterial*>self.thisptr).SetMueLorPoleFreq(order, val, ny)
        elif prop_name == 'mue_relax':
            return (<_CSPropLorentzMaterial*>self.thisptr).SetMueRelaxTime(order, val, ny)
        else:
            CSPropDispersiveMaterial._SetDispersiveMaterialPropertyDir(self, prop_name, order, ny, val)

    def _GetDispersiveMaterialWeightDir(self, prop_name, order, ny):
        if prop_name == 'eps_plasma':
            return (<_CSPropLorentzMaterial*>self.thisptr).GetEpsPlasmaFreqWeightFunction(order, ny)
        elif prop_name == 'eps_pole_freq':
            return (<_CSPropLorentzMaterial*>self.thisptr).GetEpsLorPoleFreqWeightFunction(order, ny)
        elif prop_name == 'eps_relax':
            return (<_CSPropLorentzMaterial*>self.thisptr).GetEpsRelaxTimeWeightFunction(order, ny)
        elif prop_name == 'mue_plasma':
            return (<_CSPropLorentzMaterial*>self.thisptr).GetMuePlasmaFreqWeightFunction(order, ny)
        elif prop_name == 'mue_pole_freq':
            return (<_CSPropLorentzMaterial*>self.thisptr).GetMueLorPoleFreqWeightFunction(order, ny)
        elif prop_name == 'mue_relax':
            return (<_CSPropLorentzMaterial*>self.thisptr).GetMueRelaxTimeWeightFunction(order, ny)
        else:
            CSPropDispersiveMaterial._GetDispersiveMaterialWeightDir(self, prop_name, order, ny)

    def _SetDispersiveMaterialWeightDir(self, prop_name, order, ny, val):
        if prop_name == 'eps_plasma':
            return (<_CSPropLorentzMaterial*>self.thisptr).SetEpsPlasmaFreqWeightFunction(order, val, ny)
        elif prop_name == 'eps_pole_freq':
            return (<_CSPropLorentzMaterial*>self.thisptr).SetEpsLorPoleFreqWeightFunction(order, val, ny)
        elif prop_name == 'eps_relax':
            return (<_CSPropLorentzMaterial*>self.thisptr).SetEpsRelaxTimeWeightFunction(order, val, ny)
        elif prop_name == 'mue_plasma':
            return (<_CSPropLorentzMaterial*>self.thisptr).SetMuePlasmaFreqWeightFunction(order, val, ny)
        elif prop_name == 'mue_pole_freq':
            return (<_CSPropLorentzMaterial*>self.thisptr).SetMueLorPoleFreqWeightFunction(order, val, ny)
        elif prop_name == 'mue_relax':
            return (<_CSPropLorentzMaterial*>self.thisptr).SetMueRelaxTimeWeightFunction(order, val, ny)
        else:
            CSPropDispersiveMaterial._SetDispersiveMaterialWeightDir(self, prop_name, order, ny, val)

###############################################################################
cdef class CSPropDebyeMaterial(CSPropDispersiveMaterial):
    def __init__(self, ParameterSet pset, *args, no_init=False, **kw):
        if no_init:
            super(CSPropDebyeMaterial, self).__init__(pset, no_init=True)
            return
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropDebyeMaterial(pset.thisptr)

        super(CSPropDebyeMaterial, self).__init__(pset, *args, **kw)

    def _GetDispersiveMaterialPropertyDir(self, prop_name, order, ny):
        if prop_name == 'eps_delta':
            return (<_CSPropDebyeMaterial*>self.thisptr).GetEpsDelta(order, ny)
        elif prop_name == 'eps_relax':
            return (<_CSPropDebyeMaterial*>self.thisptr).GetEpsRelaxTime(order, ny)
        else:
            CSPropDispersiveMaterial._GetDispersiveMaterialPropertyDir(self, prop_name, order, ny)

    def _SetDispersiveMaterialPropertyDir(self, prop_name, order, ny, val):
        if prop_name=='eps_delta':
            (<_CSPropDebyeMaterial*>self.thisptr).SetEpsDelta(order, val, ny)
        elif prop_name=='eps_relax':
            (<_CSPropDebyeMaterial*>self.thisptr).SetEpsRelaxTime(order, val, ny)
        else:
            CSPropDispersiveMaterial._SetDispersiveMaterialPropertyDir(self, prop_name, order, ny, val)

    def _GetDispersiveMaterialWeightDir(self, prop_name, order, ny):
        if prop_name=='eps_delta':
            return (<_CSPropDebyeMaterial*>self.thisptr).GetEpsDeltaWeightFunction(order, ny)
        elif prop_name=='eps_relax':
            return (<_CSPropDebyeMaterial*>self.thisptr).GetEpsRelaxTimeWeightFunction(order, ny)
        else:
            CSPropDispersiveMaterial._GetDispersiveMaterialWeightDir(self, prop_name, order, ny)

    def _SetDispersiveMaterialWeightDir(self, prop_name, order, ny, val):
        val = val.encode('UTF-8')
        if prop_name=='eps_delta':
            (<_CSPropDebyeMaterial*>self.thisptr).SetEpsDeltaWeightFunction(order, val, ny)
        elif prop_name=='eps_relax':
            (<_CSPropDebyeMaterial*>self.thisptr).SetEpsRelaxTimeWeightFunction(order, val, ny)
        else:
            CSPropDispersiveMaterial._SetDispersiveMaterialWeightDir(self, prop_name, order, ny, val)

