# -*- coding: utf-8 -*-
# cython: language_level=3
#
# Copyright (C) 2015,2016 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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
from CSXCAD.CSXCAD cimport _CSBackgroundMaterial

from libc.stdint cimport uintptr_t
import weakref
from CSXCAD.CSObject cimport CSDestructionCallback, wrapper_destroyed
from CSXCAD.CSObject cimport resolve_owner, STRUCTURE, BACKGROUNDMATERIAL
from CSXCAD.Utilities import RegisterWrapperFactory

from CSXCAD.CSProperties import CSPropMaterial, CSPropExcitation
from CSXCAD.CSProperties import CSPropMetal, CSPropConductingSheet
from CSXCAD.CSProperties import CSPropLumpedElement, CSPropProbeBox, CSPropDumpBox, CSPropAbsorbingBC
from CSXCAD.CSProperties import CSPropDiscMaterial
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


cdef class CSBackgroundMaterial:
    _instances = weakref.WeakValueDictionary()
    """ Wrapper per live C++ instance, so that looking the same object up twice
    gives the same wrapper. Weak on purpose: a wrapper nobody holds any more must
    be collectable, otherwise it would keep its owner alive forever. """
    """Background material for the continuous structure.

    Holds the EM properties (epsilon, mue, kappa, sigma) of the simulation
    background. Obtained via ``ContinuousStructure.GetBackgroundMaterial()``.

    :param epsilon: relative electric permittivity (default 1)
    :param mue:     relative magnetic permeability (default 1)
    :param kappa:   electric conductivity in S/m (default 0)
    :param sigma:   magnetic conductivity in Ohm/m (default 0)
    """

    @staticmethod
    cdef fromPtr(_CSBackgroundMaterial *ptr):
        if ptr == NULL:
            return None
        obj = CSBackgroundMaterial._instances.get(<uintptr_t>ptr, None)
        # an entry whose C++ object is already gone must not be handed out: the
        # weak reference only drops it once the wrapper itself dies
        if obj is not None and (<CSBackgroundMaterial>obj).thisptr != NULL:
            return obj
        cdef CSBackgroundMaterial new_obj = CSBackgroundMaterial.__new__(CSBackgroundMaterial)
        new_obj._SetPtr(ptr)
        return new_obj

    @staticmethod
    def _from_address(addr):
        """ Wrapper for the C++ instance at `addr`, \sa CSXCAD.Utilities """
        return CSBackgroundMaterial.fromPtr(<_CSBackgroundMaterial*><uintptr_t>addr)

    cdef _SetPtr(self, _CSBackgroundMaterial *ptr):
        self.thisptr = ptr
        CSBackgroundMaterial._instances[<uintptr_t>self.thisptr] = self
        self._owner = resolve_owner(self.thisptr)
        self.thisptr.SetDestructionCallback(<CSDestructionCallback>wrapper_destroyed,
                                           <void*>&self.thisptr)

    def __dealloc__(self):
        # the background material is a member of the ContinuousStructure
        if self.thisptr != NULL:
            self.thisptr.SetDestructionCallback(NULL, NULL)

    cdef _CSBackgroundMaterial* _ptr(self) except NULL:
        """ Access the C++ instance, raising if it has already been destroyed.

        The C++ instance is a member of the ContinuousStructure and dies with
        it, while this wrapper may still be referenced from python.
        """
        if self.thisptr == NULL:
            raise RuntimeError('wrapped C++ object of type {} has been deleted'.format(type(self).__name__))
        return self.thisptr

    def __cinit__(self):
        self.thisptr = NULL

    def __init__(self):
        raise TypeError('CSBackgroundMaterial cannot be instantiated directly; use ContinuousStructure.GetBackgroundMaterial()')

    def GetEpsilon(self):
        """Get relative electric permittivity."""
        return self._ptr().GetEpsilon()

    def SetEpsilon(self, val):
        """Set relative electric permittivity."""
        self._ptr().SetEpsilon(val)

    def GetMue(self):
        """Get relative magnetic permeability."""
        return self._ptr().GetMue()

    def SetMue(self, val):
        """Set relative magnetic permeability."""
        self._ptr().SetMue(val)

    def GetKappa(self):
        """Get electric conductivity in S/m."""
        return self._ptr().GetKappa()

    def SetKappa(self, val):
        """Set electric conductivity in S/m."""
        self._ptr().SetKappa(val)

    def GetSigma(self):
        """Get (artificial) magnetic conductivity in Ohm/m."""
        return self._ptr().GetSigma()

    def SetSigma(self, val):
        """Set (artificial) magnetic conductivity in Ohm/m."""
        self._ptr().SetSigma(val)

    def Reset(self):
        """Reset all material parameters to their defaults (vacuum)."""
        self._ptr().Reset()


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
    _instances = weakref.WeakValueDictionary()
    """ Wrapper per live C++ instance, so that an owned object and a borrowed
    lookup of it share one wrapper -- which matters because only the owning
    wrapper deletes the C++ instance, and everything owned by the structure
    keeps a reference to it. Weak, \sa CSProperties._instances """

    def __cinit__(self, *args, no_init=False, **kw):
        self.no_init = no_init
        if no_init:
            # wrapping a C++ structure owned by someone else, \sa fromPtr
            self.thisptr = NULL
            return

        self._SetPtr(new _ContinuousStructure())

        if 'CoordSystem' in kw:
            self.SetMeshType(kw['CoordSystem'])
            del kw['CoordSystem']
        elif 'cs_type' in kw:
            self.SetMeshType(kw['cs_type'])
            del kw['cs_type']

        if len(kw)!=0:
            raise Exception('Unknown keyword arguments: "{}"'.format(kw))

    @staticmethod
    cdef fromPtr(_ContinuousStructure *ptr):
        """ Wrap an existing C++ structure that is owned and deleted elsewhere. """
        if ptr == NULL:
            return None
        csx = ContinuousStructure._instances.get(<uintptr_t>ptr, None)
        # an entry whose C++ object is already gone must not be handed out: the
        # weak reference only drops it once the wrapper itself dies
        if csx is not None and (<ContinuousStructure>csx).thisptr != NULL:
            return csx
        cdef ContinuousStructure new_csx = ContinuousStructure(no_init=True)
        new_csx._SetPtr(ptr)
        return new_csx

    @staticmethod
    def _from_address(addr):
        """ Wrapper for the C++ instance at `addr`, \sa CSXCAD.Utilities """
        return ContinuousStructure.fromPtr(<_ContinuousStructure*><uintptr_t>addr)

    cdef _SetPtr(self, _ContinuousStructure *ptr):
        self.thisptr = ptr
        ContinuousStructure._instances[<uintptr_t>self.thisptr] = self
        ptr.SetDestructionCallback(<CSDestructionCallback>wrapper_destroyed,
                                   <void*>&self.thisptr)

    def __dealloc__(self):
        if self.thisptr != NULL:
            # drop the hook before deleting, it must not notify a dying wrapper
            self.thisptr.SetDestructionCallback(NULL, NULL)
            # delete only what we created and what nobody else claimed: a solver
            # taking the structure over will destroy it itself
            if not self.no_init and not self.thisptr.IsOwned():
                del self.thisptr

    cdef _ContinuousStructure* _ptr(self) except NULL:
        """ Access the C++ instance, raising if it has already been destroyed. """
        if self.thisptr == NULL:
            raise RuntimeError('wrapped C++ object of type {} has been deleted'.format(type(self).__name__))
        return self.thisptr

    def Update(self):
        return self._ptr().Update().decode('UTF-8')

    def Clear(self):
        return self._ptr().clear()

    Reset=Clear

    def Write2XML(self, file:Path|str):
        """Write geometry to an xml-file

        :param file: Path to the file where to write the data.
        """
        file = Path(file) # Check that whatever we receive can be interpreted as a path.
        if not file.parent.is_dir():
            raise FileNotFoundError(f'Directory in which file is to be saved does not exist. ')
        return self._ptr().Write2XML(str(file).encode('UTF-8'))

    def ReadFromXML(self, fn):
        """ ReadFromXML(fn)

        Read geometry from xml-file

        :param fn: str -- file name
        """
        return self._ptr().ReadFromXML(fn.encode('UTF-8')).decode('UTF-8')

    def GetParameterSet(self):
        """
        Get the parameter set assigned to this class
        """
        return ParameterSet.fromPtr(self._ptr().GetParameterSet())

    def GetGrid(self):
        """
        Get the CSRectGrid assigned to this class.

        See Also
        --------
        CSXCAD.CSRectGrid, DefineGrid
        """
        return CSRectGrid.fromPtr(self._ptr().GetGrid())

    def GetBackgroundMaterial(self):
        """Get the background material of this structure.

        :returns: CSBackgroundMaterial -- background EM material (epsilon, mue, kappa, sigma)
        """
        return CSBackgroundMaterial.fromPtr(self._ptr().GetBackgroundMaterial())

    @property
    def grid(self):
        """Get the CSRectGrid, same as calling the `GetGrid` method."""
        return self.GetGrid()

    def SetMeshType(self, cs_type):
        grid = self.GetGrid()
        grid.SetMeshType(cs_type)
        self._ptr().SetCoordInputType(cs_type)

    def GetCoordInputType(self):
        return self._ptr().GetCoordInputType()

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
        return self._ptr().GetQtyProperties()

    def GetQtyPrimitives(self, prop_type=c_CSProperties.ANY):
        return self._ptr().GetQtyPrimitives(prop_type)

    def AddMaterial(self, name:str, **kw):
        """Add a material property with name `name`.

        See Also
        --------
        CSXCAD.CSProperties.CSPropMaterial
        """
        if not isinstance(name, str):
            raise TypeError(f'`name` must be a str, received object of type {type(name)}. ')
        return self.__CreateProperty('Material', name, **kw)

    def AddDiscMaterial(self, name, **kw):
        """Add a discrete material property with name `name`.

        :param filename: str -- path to the HDF5 file containing the material data
        :param scale:    float -- spatial scale factor applied to the mesh coordinates (default 1)
        :param use_db_background: bool -- use database index 0 as background (default True)

        See Also
        --------
        CSXCAD.CSProperties.CSPropDiscMaterial
        """
        return self.__CreateProperty('DiscMaterial', name, **kw)

    def AddAbsorbingBC(self, name, **kw):
        """ AddAbsorbingBC(name, **kw)

        Add a local absorbing BC with name `name`.

        See Also
        --------
        CSXCAD.CSProperties.CSPropAbsorbingBC
        """

        return self.__CreateProperty('AbsorbingBC', name, **kw)
    
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
        self._ptr().AddProperty(prop.thisptr)
        # C++ ownership just changed, so the wrapper has to pick it up
        prop._owner = resolve_owner(prop.thisptr)

    def RemoveProperty(self, CSProperties prop):
        self._ptr().RemoveProperty(prop.thisptr)
        # C++ ownership just changed, so the wrapper has to pick it up
        prop._owner = resolve_owner(prop.thisptr)

    def DeleteProperty(self, CSProperties prop):
        self._ptr().DeleteProperty(prop.thisptr)

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
        _prop = self._ptr().GetProperty(index)
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
        vprop = self._ptr().GetPropertiesByName(name)

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
        vprop = self._ptr().GetPropertyByType(prop_type)

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
        cdef _CSProperties *_prop = self._ptr().GetPropertyByCoordPriority(coord, prop_type, markFoundAsUsed, &prim)
        return CSProperties.fromPtr(_prop)

    def GetAllPrimitives(self, sort=False, prop_type=c_CSProperties.ANY):
        """ GetAllPrimitives(sort, prop_type)

        Get a list of all primitives.
        """
        return self.__GetAllPrimitives(sort, prop_type)

    cdef __GetAllPrimitives(self, bool sort, PropertyType prop_type):
        cdef vector[_CSPrimitives*] vprim
        vprim = self._ptr().GetAllPrimitives(sort, prop_type)

        cdef _CSPrimitives* _prim
        cdef CSPrimitives prim
        prims = []
        for n in range(vprim.size()):
            _prim = vprim.at(n)
            prim = CSPrimitives.fromPtr(_prim)
            prims.append(prim)

        return prims

RegisterWrapperFactory(STRUCTURE, ContinuousStructure._from_address)
RegisterWrapperFactory(BACKGROUNDMATERIAL, CSBackgroundMaterial._from_address)
