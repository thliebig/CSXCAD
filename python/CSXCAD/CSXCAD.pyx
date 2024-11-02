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

    def AddDump(self, name:str, dump_type:str|int='E_field_time_domain', frequency:list=None, dump_mode:str|int='node_interpolation', file_type:str|int='vtk', sub_sampling:list|int=None, opt_resolution:list|int=None):
        """Add a dump property.

        Arguments
        ---------
        name:
            A name for this dump. This name will be used as file name.
        dump_type:
            Define the type of dump, one of 'B_field_frequency_domain', 'B_field_time_domain',  'D_field_frequency_domain', 'D_field_time_domain', 'E_field_frequency_domain', 'E_field_time_domain', 'H_field_frequency_domain', 'H_field_time_domain', 'SAR_10g_averaging_frequency_domain', 'SAR_1g_averaging_frequency_domain', 'SAR_local_frequency_domain', 'SAR_raw_data', 'current_density_frequency_domain', 'current_density_time_domain', 'electric_current_frequency_domain', 'electric_current_time_domain'.
            Note: The acceptance of an `int` value is kept for backwards compatibility, but should not be used.
        frequency:
            A list of frequencies, required for frequency domain dumps.
        dump_mode:
            One of 'no_interpolation', 'node_interpolation', 'cell_interpolation'. See warning below.
            Note: The acceptance of an `int` value is kept for backwards compatibility, but should not be used.
        file_type:
            The file format, one of 'vtk', 'hdf5'.
            Note: The acceptance of an `int` value is kept for backwards compatibility, but should not be used.
        sub_sampling:
            A 3-int list defining the field domain subsampling in each direction, or a single int that will be used as the same number in all directions. E.g. `[2,2,4]` means "dump only every second line in x- and y- and only every forth line in z-direction". `4` is equivalent to `[4,4,4]`.
        opt_resolution:
            A 3-int defining the field domain dump resolution in each direction, or a single int that will be used as the same number in all directions.. E.g. `[10,20,5]` means "choose lines to dump in such a way, that they are about 10 in x-, 20 in y- and 5 in z-direction drawing units apart from another". This can mean in some area that every line is dumped (as they may be about 10 drawing units or more apart), or maybe only every second, or tenth etc. line to result in an average distance of <10 drawing units apart. The first and last line inside the dump box are always choosen. `4` is equivalent to `[4,4,4]`.
            Note: The dumped lines must not be homogeneous, they only try to be as much as possible. This option may be useful in case of a very inhomogeneous FDTD mesh to create a somewhat more homogeneous field dump.

        Warning
        -------
        FDTD Interpolation abnormalities:
          - no-interpolation: fields are located in the mesh by the Yee-scheme, the mesh only specifies E- or H-Yee-nodes. Use node- or cell-interpolation or be aware of the offset
          - E-field dump & node-interpolation: normal electric fields on boundaries will have false amplitude due to forward/backward interpolation in case of (strong) changes in material permittivity or on metal surfaces. Use no- or cell-interpolation.
          - H-field dump & cell-interpolation: normal magnetic fields on boundaries will have false amplitude due to forward/backward interpolation in case of (strong) changes in material permeability. Use no- or node-interpolation.

        See Also
        --------
        CSXCAD.CSProperties.CSPropDumpBox
        """
        DUMP_TYPE_MAPPING = dict(
            E_field_time_domain = 0,
            H_field_time_domain = 1,
            electric_current_time_domain = 2,
            current_density_time_domain = 3,
            D_field_time_domain = 4,
            B_field_time_domain = 5,
            E_field_frequency_domain = 10,
            H_field_frequency_domain = 11,
            electric_current_frequency_domain = 12,
            current_density_frequency_domain = 13,
            D_field_frequency_domain = 14,
            B_field_frequency_domain = 15,
            SAR_local_frequency_domain = 20,
            SAR_1g_averaging_frequency_domain = 21,
            SAR_10g_averaging_frequency_domain = 22,
            SAR_raw_data = 29,
        )
        DUMP_MODE_MAPPING = dict(
            no_interpolation = 0,
            node_interpolation = 1,
            cell_interpolation = 2,
        )
        FILE_TYPE_MAPPING = dict(
            vtk = 0,
            hdf5 = 1,
        )

        kw = {}

        if dump_type not in DUMP_TYPE_MAPPING | {i:i for i in (0,1,2,3,4,5,10,11,12,13,14,15,20,21,22,29)}: # The union with `{i:i for i in (0,...)}` is there for backwards compatibility.
            raise ValueError(f'`dump_type` must be one of {sorted(DUMP_TYPE_MAPPING)}, received {dump_type}. ')
        kw['dump_type'] = DUMP_TYPE_MAPPING[dump_type]

        if frequency is not None:
            if not isinstance(frequency, (list, tuple)) or any([not isinstance(f, (float,int)) for f in frequency]):
                raise TypeError(f'`frequency` must be a list of floats. ')
            kw['Frequency'] = frequency

        if dump_mode not in DUMP_MODE_MAPPING | {i:i for i in (0,1,2)}: # The union with `{i:i for i in (0,...)}` is there for backwards compatibility.
            raise ValueError(f'`dump_mode` must be one of {sorted(DUMP_MODE_MAPPING)}, received {dump_mode}. ')
        kw['dump_mode'] = DUMP_MODE_MAPPING[dump_mode]

        if file_type not in FILE_TYPE_MAPPING | {i:i for i in (0,1)}: # The union with `{i:i for i in (0,...)}` is there for backwards compatibility.
            raise ValueError(f'`file_type` must be one of {sorted(FILE_TYPE_MAPPING)}, received {file_type}. ')
        kw['file_type'] = FILE_TYPE_MAPPING[file_type]

        if sub_sampling is not None:
            if isinstance(sub_sampling, int):
                sub_sampling = [sub_sampling]*3
            if not isinstance(sub_sampling, list) or any([not isinstance(_, int) for _ in sub_sampling]) or len(sub_sampling) != 3:
                raise TypeError(f'`sub_sampling` must be a 3-int list, received {sub_sampling}. ')
            kw['sub_sampling'] = sub_sampling

        if opt_resolution is not None:
            if isinstance(opt_resolution, int):
                opt_resolution = [opt_resolution]*3
            if not isinstance(opt_resolution, list) or any([not isinstance(_, int) for _ in opt_resolution]) or len(opt_resolution) != 3:
                raise TypeError(f'`opt_resolution` must be a 3-int list, received {opt_resolution}. ')
            kw['opt_resolution'] = opt_resolution

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

