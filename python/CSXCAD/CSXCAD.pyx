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

cimport CSXCAD.CSXCAD

from CSXCAD.CSProperties import CSPropMaterial, CSPropExcitation
from CSXCAD.CSProperties import CSPropMetal, CSPropConductingSheet
from CSXCAD.CSProperties import CSPropLumpedElement, CSPropProbeBox, CSPropDumpBox
from CSXCAD.CSPrimitives import CSPrimPoint, CSPrimBox, CSPrimCylinder, CSPrimCylindricalShell
from CSXCAD.CSPrimitives import CSPrimSphere, CSPrimSphericalShell
from CSXCAD.CSPrimitives import CSPrimPolygon, CSPrimLinPoly, CSPrimRotPoly
from CSXCAD.CSPrimitives import CSPrimCurve, CSPrimWire
from CSXCAD.CSPrimitives import CSPrimPolyhedron, CSPrimPolyhedronReader
cimport CSXCAD.CSProperties as c_CSProperties
cimport CSXCAD.CSRectGrid   as c_CSRectGrid
from CSXCAD.CSProperties import CSProperties
from CSXCAD.ParameterObjects import ParameterSet

from CSXCAD.SmoothMeshLines import SmoothMeshLines

from pathlib import Path

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
    >>> box   = metal.AddBox(start, stop) # Assign a box to propety "metal"
    """
    def __cinit__(self, **kw):
        self.thisptr   = new _ContinuousStructure()

        if 'CoordSystem' in kw:
            self.SetMeshType(kw['CoordSystem'])
            del kw['CoordSystem']
        elif 'cs_type' in kw:
            self.SetMeshType(kw['cs_type'])
            del kw['cs_type']

        if len(kw)!=0:
            raise Exception('Unknown keyword arguments: "{}"'.format(kw))

    def Update(self):
        return self.thisptr.Update().decode('UTF-8')

    def Clear(self):
        return self.thisptr.clear()

    Reset=Clear

    def Write2XML(self, file:Path|str):
        """Write geometry to an xml-file

        :param file: Path to the file where to write the data.
        """
        file = Path(file) # Check that whatever we receive can be interpreted as a path.
        if not file.parent.is_dir():
            raise FileNotFoundError(f'Directory in which file is to be saved does not exist. ')
        self.thisptr.Write2XML(str(file).encode('UTF-8'))

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
        return ParameterSet.fromPtr(self.thisptr.GetParameterSet())

    def GetGrid(self):
        """
        Get the CSRectGrid assigned to this class.

        See Also
        --------
        CSXCAD.CSRectGrid, DefineGrid
        """
        return CSRectGrid.fromPtr(self.thisptr.GetGrid())

    @property
    def grid(self):
        """Get the CSRectGrid, same as calling the `GetGrid` method."""
        return self.GetGrid()

    def SetMeshType(self, cs_type):
        grid = self.GetGrid()
        grid.SetMeshType(cs_type)
        self.thisptr.SetCoordInputType(cs_type)

    def GetCoordInputType(self):
        return self.thisptr.GetCoordInputType()

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

    def AddMaterial(self, name:str, **kw):
        """Add a material property with name `name`.

        See Also
        --------
        CSXCAD.CSProperties.CSPropMaterial
        """
        if not isinstance(name, str):
            raise TypeError(f'`name` must be a str, received object of type {type(name)}. ')
        return self.__CreateProperty('Material', name, **kw)

    def AddLumpedElement(self, name, **kw):
        """ AddLumpedElement(name, **kw)

        Add a lumped element with name `name`.

        See Also
        --------
        CSXCAD.CSProperties.CSPropLumpedElement
        """
        return self.__CreateProperty('LumpedElement', name, **kw)

    def AddMetal(self, name:str, **kw):
        """Add a metal property with name `name`.

        See Also
        --------
        CSXCAD.CSProperties.CSPropMetal
        """
        return self.__CreateProperty('Metal', name, **kw)

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

    def AddDump(self, name:str, **kw):
        """ AddDump(name, **kw)

        Add a dump property with name `name`.

        See Also
        --------
        CSXCAD.CSProperties.CSPropDumpBox
        """
        return self.__CreateProperty('DumpBox', name, **kw)

    def __CreateProperty(self, type_str:str, name:str, **kw):
        pset = self.GetParameterSet()
        prop = CSProperties.fromTypeName(type_str, pset, **kw)
        if prop is None:
            raise RuntimeError(f'Unknown property type: {type_str}')
        prop.SetName(name)
        self.AddProperty(prop)
        return prop

    def AddProperty(self, CSProperties prop):
        """ AddProperty(prop)

        Add an already created property (`prop`) to this class.

        Notes
        -----
        This class will take ownership of the property.
        """
        self.thisptr.AddProperty(prop.thisptr)

    def RemoveProperty(self, CSProperties prop):
        self.thisptr.RemoveProperty(prop.thisptr)

    def DeleteProperty(self, CSProperties prop):
        self.thisptr.DeleteProperty(prop.thisptr)

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
        _prop = self.thisptr.GetProperty(index)
        return CSProperties.fromPtr(_prop)

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
            prop = CSProperties.fromPtr(_prop)
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
            prop = CSProperties.fromPtr(_prop)
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
        return CSProperties.fromPtr(_prop)

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
            prim = CSPrimitives.fromPtr(_prim)
            prims.append(prim)

        return prims

