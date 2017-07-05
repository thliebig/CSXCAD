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


"""
This moduld contains the main class ContinuousStructure (CSX).


Examples
--------

>>> CSX   = CSXCAD.ContinuousStructure()
>>> metal = CSX.AddMetal('metal')  # create a metal property
>>> box   = CSX.AddBox(metal, [0,0,0, [10,10,10]) # assign a box to the metal property

>>> mesh = CSX.GetGrid()            # get a grid instance
>>> mesh.AddLine('x', [-20, 0, 20]) # add some lines in x-direction
>>> mesh.SmoothMeshLines('x', 2.5)  # smooth the mesh
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
cimport CSProperties as c_CSProperties
cimport CSRectGrid   as c_CSRectGrid
from CSProperties import CSProperties
from ParameterObjects import ParameterSet

from SmoothMeshLines import SmoothMeshLines

cdef class ContinuousStructure:
    """ ContinuousStructure

    With this class different properties and primitives can be created and a
    rectengular grid assigned.

    See Also
    --------
    CSXCAD.CSProperties
    CSXCAD.CSPrimitives
    CSXCAD.CSRectGrid

    Examples
    --------
    Create a metal box:

    >>> CSX   = CSXCAD.ContinuousStructure()
    >>> metal = CSX.AddMetal('metal') # create a metal property with name "metal"
    >>> start = [0,0,0]
    >>> stop  = [1,2,1]
    >>> box   = CSX.AddBox(metal, start, stop) # Assign a box to propety "metal"
    """
    def __cinit__(self, **kw):
        self.thisptr = new _ContinuousStructure()
        self.__paraset = ParameterSet(no_init=True)
        self.__paraset.thisptr = self.thisptr.GetParameterSet()

        self.__grid         = CSRectGrid(no_init=True)
        self.__grid.thisptr = self.thisptr.GetGrid()

        if 'CoordSystem' in kw:
            self.SetMeshType(kw['CoordSystem'])
            del kw['CoordSystem']
        elif 'cs_type' in kw:
            self.SetMeshType(kw['cs_type'])
            del kw['cs_type']

        assert len(kw)==0, 'Unknown keyword arguments: "{}"'.format(kw)

    def __dealloc__(self):
        del self.thisptr

    def Update(self):
        return self.thisptr.Update().decode('UTF-8')

    def Write2XML(self, fn):
        """ Write2XML(fn)

        Write geometry to an xml-file

        :param fn: str -- file name
        """
        self.thisptr.Write2XML(fn.encode('UTF-8'))

    def ReadFromXML(self, fn):
        """ ReadFromXML(fn)

        Read geometry from xml-file

        :param fn: str -- file name
        """
        return self.thisptr.ReadFromXML(fn.encode('UTF-8')).decode('UTF-8')

    def GetParameterSet(self):
        """
        Get the parameter set assigned to this class
        """
        return self.__paraset

    def GetGrid(self):
        """
        Get the CSRectGrid assigned to this class.

        See Also
        --------
        CSXCAD.CSRectGrid, DefineGrid
        """
        return self.__grid

    def SetMeshType(self, cs_type):
        self.__grid.SetMeshType(cs_type)
        self.thisptr.SetCoordInputType(cs_type)

    def DefineGrid(self, mesh, unit, smooth_mesh_res=None):
        """ DefineGrid(mesh, unit, smooth_mesh_res=None)

        Assign a mesh lines to the grid assigned to this property.

        :param mesh: (3,) list of mesh lines
        :param unit: float -- drawing unit
        :param smooth_mesh_res: an optional mesh smoothing

        See Also
        --------
        CSXCAD.CSRectGrid, GetGrid, CSXCAD.SmoothMeshLines.SmoothMeshLines

        """
        grid = self.GetGrid()
        grid.Clear()
        if smooth_mesh_res is not None:
            for k in mesh:
                mesh[k] = SmoothMeshLines(mesh[k], smooth_mesh_res)

        for k in mesh:
            grid.SetLines(k, mesh[k])

        grid.SetDeltaUnit(unit)
        return grid

    def GetQtyProperties(self):
        return self.thisptr.GetQtyProperties()

    def GetQtyPrimitives(self, prop_type=c_CSProperties.ANY):
        return self.thisptr.GetQtyPrimitives(prop_type)

    def AddMaterial(self, name, **kw):
        """ AddMaterial(name, **kw)

        Add a material property with name `name`.

        See Also
        --------
        CSXCAD.CSProperties.CSPropMaterial
        """
        return self.__CreateProperty('Material', name, **kw)

    def AddLumpedElement(self, name, **kw):
        """ AddLumpedElement(name, **kw)

        Add a lumped element with name `name`.

        See Also
        --------
        CSXCAD.CSProperties.CSPropLumpedElement
        """
        return self.__CreateProperty('LumpedElement', name, **kw)

    def AddMetal(self, name):
        """ AddMetal(name)

        Add a metal property with name `name`.

        See Also
        --------
        CSXCAD.CSProperties.CSPropMetal
        """
        return self.__CreateProperty('Metal', name)

    def AddConductingSheet(self, name, **kw):
        """ AddConductingSheet(name, **kw)

        Add a conducting sheet with name `name`.

        See Also
        --------
        CSXCAD.CSProperties.CSPropConductingSheet
        """
        return self.__CreateProperty('ConductingSheet', name, **kw)

    def AddExcitation(self, name, exc_type, exc_val, **kw):
        """ AddExcitation(name, exc_type, exc_val, **kw)

        Add an excitation property with name `name`.

        See Also
        --------
        CSXCAD.CSProperties.CSPropExcitation
        """
        return self.__CreateProperty('Excitation', name, exc_type=exc_type, exc_val=exc_val, **kw)

    def AddProbe(self, name, p_type, **kw):
        """ AddProbe(name, p_type, **kw)

        Add a probe property with name `name`.

        See Also
        --------
        CSXCAD.CSProperties.CSPropProbeBox
        """
        return self.__CreateProperty('ProbeBox', name, p_type=p_type, **kw)

    def AddDump(self, name, **kw):
        """ AddDump(name, **kw)

        Add a dump property with name `name`.

        See Also
        --------
        CSXCAD.CSProperties.CSPropDumpBox
        """
        return self.__CreateProperty('DumpBox', name, **kw)

    def __CreateProperty(self, type_str, name, *args, **kw):
        assert len(args)==0, 'CreateProperty does not support additional arguments'
        prop = CSProperties.fromTypeName(type_str, self.__paraset, **kw)
        if prop is None:
            raise Exception('CreateProperty: Unknown property type requested: {}'.format(type_str))
        prop.SetName(name)
        self.AddProperty(prop)
        return prop

    def AddProperty(self, prop):
        """ AddProperty(prop)

        Add an already created property (`prop`) to this class.

        Notes
        -----
        This class will take ownership of the property.
        """
        self._AddProperty(prop)

    cdef _AddProperty(self, CSProperties prop):
        prop.__CSX = self
        self.thisptr.AddProperty(prop.thisptr)

    def GetProperty(self, index):
        """ GetProperty(index)

        Get the property at the given index

        See Also
        --------
        CSXCAD.GetQtyProperties
        """
        if index<0 or index >=self.GetQtyProperties():
            raise IndexError('Index is out of range')
        return self._GetProperty(index)

    cdef _GetProperty(self, int index):
        cdef _CSProperties* _prop
        cdef CSProperties prop
        _prop = self.thisptr.GetProperty(index)
        prop = CSProperties.fromType(_prop.GetType(), pset=None, no_init=True)
        prop.thisptr = _prop
        return prop

    def GetAllProperties(self):
        """ GetAllProperties()

        Get a list of all properties
        """
        props = []
        for n in range(self.GetQtyProperties()):
            props.append(self._GetProperty(n))
        return props

    def GetPropertiesByName(self, name):
        """ GetPropertiesByName(name)

        Get all the property specifed by their name
        """
        return self.__GetPropertiesByName(name.encode('UTF-8'))

    cdef __GetPropertiesByName(self, string name):
        cdef vector[_CSProperties*] vprop
        vprop = self.thisptr.GetPropertiesByName(name)

        cdef _CSProperties* _prop
        cdef CSProperties prop
        props = []
        for n in range(vprop.size()):
            _prop = vprop.at(n)
            prop = CSProperties.fromType(_prop.GetType(), pset=None, no_init=True)
            prop.thisptr = _prop
            props.append(prop)

        return props

    def GetPropertyByType(self, prop_type):
        """ GetPropertyByType(prop_type)

        Get a list of properties specified by their type
        """
        return self.__GetPropertyByType(prop_type)

    cdef __GetPropertyByType(self, PropertyType prop_type):
        cdef vector[_CSProperties*] vprop
        vprop = self.thisptr.GetPropertyByType(prop_type)

        cdef _CSProperties* _prop
        cdef CSProperties prop
        props = []
        for n in range(vprop.size()):
            _prop = vprop.at(n)
            prop = CSProperties.fromType(_prop.GetType(), pset=None, no_init=True)
            prop.thisptr = _prop
            props.append(prop)

        return props

    def GetPropertyByCoordPriority(self, coord, prop_type=c_CSProperties.ANY, markFoundAsUsed=False):
        """ GetPropertyByCoordPriority(coord, prop_type=None, markFoundAsUsed=False)
        """
        cdef double _coord[3]
        for n in range(3):
            _coord[n] = coord[n]
        return self.__GetPropertyByCoordPriority(_coord, prop_type, markFoundAsUsed)

    cdef __GetPropertyByCoordPriority(self, double* coord, PropertyType prop_type, bool markFoundAsUsed):
        cdef _CSPrimitives *prim
        cdef _CSProperties *_prop = self.thisptr.GetPropertyByCoordPriority(coord, prop_type, markFoundAsUsed, &prim)

        cdef CSProperties prop
        if _prop==NULL:
            return None
        else:
            prop = CSProperties.fromType(_prop.GetType(), pset=None, no_init=True)
            prop.thisptr = _prop
            return prop

    def GetAllPrimitives(self, sort=False, prop_type=c_CSProperties.ANY):
        """ GetAllPrimitives(sort, prop_type)

        Get a list of all primitives.
        """
        return self.__GetAllPrimitives(sort, prop_type)

    cdef __GetAllPrimitives(self, bool sort, PropertyType prop_type):
        cdef vector[_CSPrimitives*] vprim
        vprim = self.thisptr.GetAllPrimitives(sort, prop_type)

        cdef _CSPrimitives* _prim
        cdef CSPrimitives prim
        prims = []
        for n in range(vprim.size()):
            _prim = vprim.at(n)
            prim = CSPrimitives.fromType(_prim.GetType(), pset=None, prop=None, no_init=True)
            prim.thisptr = _prim
            prims.append(prim)

        return prims

