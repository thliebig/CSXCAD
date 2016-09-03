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
from ParameterObjects cimport _ParameterSet, ParameterSet
cimport CSProperties
from Utilities import CheckNyDir

cdef class CSProperties:
    """
    Virtual base class for all properties, cannot be created!

    """
    def __init__(self, ParameterSet pset, *args, **kw):
        assert self.thisptr, "Error, cannot create CSProperties (protected)"
        self.__CSX = None

        assert len(kw)==0, 'Unknown keywords: {}'.format(kw)

    def __dealloc__(self):
        # only delete if this property is not owned by a CSX object
        if self.__CSX is None:
            del self.thisptr

    def GetQtyPrimitives(self):
        """
        Get the number of primitives assigned to this property

        :returns: int -- Number of primitives
        """
        return self.thisptr.GetQtyPrimitives()

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

    def AddAttribute(self, name, val):
        """ AddAttribute(name, val)

        Add an attribure and value

        :param name: str -- Attribute name
        :param val: str -- Attribute value
        """
        self.thisptr.AddAttribute(name.encode('UTF-8'), val.encode('UTF-8'))

###############################################################################
cdef class CSPropMaterial(CSProperties):
    """ General material property

    This is a material with options to define relative electric permeability
    (`eplsilon`), relative magnetic permittivity (`mue`), electric conductivity
    (`kappa`), magnetic conductivity (`sigma`) and `density`.

    :params epsilon: scalar or vector - relative electric permeability
    :params mue:     scalar or vector - relative magnetic permittivity
    :params kappa:   scalar or vector - relectric conductivity
    :params sigma:   scalar or vector - magnetic conductivity
    :params density: float            - Density
    """
    def __init__(self, ParameterSet pset, *args, **kw):
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropMaterial(pset.thisptr)
        self.matptr  = <_CSPropMaterial*>self.thisptr
        self.SetMaterialProperty(**kw)
        for k in list(kw.keys()):
            if k in ['epsilon', 'mue', 'kappa', 'sigma', 'density']:
                del kw[k]

        super(CSPropMaterial, self).__init__(pset, *args, **kw)

    def SetIsotropy(self, val):
        """ SetIsotropy(val)

        Set isotropy status for this material.

        :param val: bool -- enable/disable isotropy
        """
        self.matptr.SetIsotropy(val)

    def GetIsotropy(self):
        """
        Get isotropy status for this material.

        :returns: bool -- isotropy status
        """
        return self.matptr.GetIsotropy()

    def SetMaterialProperty(self, **kw):
        """ SetMaterialProperty(**kw)
        Set the material properties.

        :params epsilon: scalar or vector - relative electric permeability
        :params mue:     scalar or vector - relative magnetic permittivity
        :params kappa:   scalar or vector - relectric conductivity
        :params sigma:   scalar or vector - magnetic conductivity
        :params density: float            - Density
        """
        for prop_name in kw:
            val = kw[prop_name]
            if prop_name == 'density':
                self.matptr.SetDensity(val)
                continue
            if type(val)==float or type(val)==int:
                self.__SetMaterialPropertyDir(prop_name, 0, val)
                continue
            assert len(val)==3, 'SetMaterialProperty: "{}" must be a list or array of length 3'.format(prop_name)
            for n in range(3):
                self.__SetMaterialPropertyDir(prop_name, n, val[n])

    def __SetMaterialPropertyDir(self, prop_name, ny, val):
        if prop_name=='epsilon':
            return self.matptr.SetEpsilon(val, ny)
        elif prop_name=='mue':
            return self.matptr.SetMue(val, ny)
        elif prop_name=='kappa':
            return self.matptr.SetKappa(val, ny)
        elif prop_name=='sigma':
            return self.matptr.SetSigma(val, ny)
        else:
            raise Exception('SetMaterialPropertyDir: Error, unknown material property')

    def SetMaterialWeight(self, **kw):
        """ SetMaterialWeight(**kw)

        Set the material weighting function(s)

        The functions can use the variables:
        `x`,`y`,`z`
        `rho` for the distance to z-axis
        `r`   for the distance to origin
        `a`   for alpha or phi (as in cylindircal and spherical coord systems)
        `t`   for theta (as in the spherical coord system

        all these variables are not weighted with the drawing unit defined by
        the grid

        :params epsilon: str or str-vector - relative electric permeability
        :params mue:     str or str-vector - relative magnetic permittivity
        :params kappa:   str or str-vector - relectric conductivity
        :params sigma:   str or str-vector - magnetic conductivity
        :params density: str               - Density
        """
        for prop_name in kw:
            val = kw[prop_name]
            if prop_name == 'density':
                self.matptr.SetDensityWeightFunction(val.encode('UTF-8'))
                continue
            if type(val)==str:
                self.__SetMaterialWeightDir(prop_name, 0, val)
                continue
            assert len(val)==3, 'SetMaterialWeight: "{}" must be a list or array of length 3'.format(prop_name)
            for n in range(3):
                self.__SetMaterialWeightDir(prop_name, n, val[n])

    def __SetMaterialWeightDir(self, prop_name, ny, val):
        val = val.encode('UTF-8')
        if prop_name=='epsilon':
            return self.matptr.SetEpsilonWeightFunction(val, ny)
        elif prop_name=='mue':
            return self.matptr.SetMueWeightFunction(val, ny)
        elif prop_name=='kappa':
            return self.matptr.SetKappaWeightFunction(val, ny)
        elif prop_name=='sigma':
            return self.matptr.SetSigmaWeightFunction(val, ny)
        else:
            raise Exception('SetMaterialWeightDir: Error, unknown material property')

    def GetMaterialProperty(self, prop_name):
        """ SetMaterialProperty(prop_name)
        Get the material property with of type `prop_name`.

        :params prop_name: str -- material property type
        :returns: float for isotropic material and `density` or else (3,) array
        """
        if prop_name == 'density':
            return self.matptr.GetDensity()
        if self.matptr.GetIsotropy():
            return self.__GetMaterialPropertyDir(prop_name, 0)
        val = np.zeros(3)
        for n in range(3):
            val[n] = self.__GetMaterialPropertyDir(prop_name, n)
        return val

    def __GetMaterialPropertyDir(self, prop_name, ny):
        if prop_name=='epsilon':
            return self.matptr.GetEpsilon(ny)
        elif prop_name=='mue':
            return self.matptr.GetMue(ny)
        elif prop_name=='kappa':
            return self.matptr.GetKappa(ny)
        elif prop_name=='sigma':
            return self.matptr.GetSigma(ny)
        else:
            raise Exception('GetMaterialPropertyDir: Error, unknown material property')

    def GetMaterialWeight(self, prop_name):
        """ GetMaterialWeight(prop_name)
        Get the material weighting function(s).

        :params prop_name: str -- material property type
        :returns: str for isotropic material and `density` or else str array
        """
        if prop_name == 'density':
            return self.matptr.GetDensityWeightFunction().decode('UTF-8')
        if self.matptr.GetIsotropy():
            return self.__GetMaterialWeightDir(prop_name, 0).decode('UTF-8')
        val = ['', '', '']
        for n in range(3):
            val[n] = self.__GetMaterialWeightDir(prop_name, n).decode('UTF-8')
        return val

    def __GetMaterialWeightDir(self, prop_name, ny):
        if prop_name=='epsilon':
            return self.matptr.GetEpsilonWeightFunction(ny)
        elif prop_name=='mue':
            return self.matptr.GetMueWeightFunction(ny)
        elif prop_name=='kappa':
            return self.matptr.GetKappaWeightFunction(ny)
        elif prop_name=='sigma':
            return self.matptr.GetSigmaWeightFunction(ny)
        else:
            raise Exception('GetMaterialWeightDir: Error, unknown material property')


###############################################################################
cdef class CSPropLumpedElement(CSProperties):
    """
    Lumped element property.

    A lumped element can consist of a resitor `R`, capacitor `C` and inductance
    `L` active in a given direction `ny`.
    If Caps are enable, a small PEC plate is added to each
    end of the lumped element to ensure contact to a connecting line

    :param ny: int or str -- direction:  0,1,2 or 'x','y','z' for the orientation of the lumped element
    :param caps: bool     -- enable/disable caps
    :param R:  float      -- lumped resitance value
    :param C:  float      -- lumped capacitance value
    :param L:  float      -- lumped inductance value
    """

    def __init__(self, ParameterSet pset, *args, **kw):
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropLumpedElement(pset.thisptr)
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
            elif k=='caps':
                self.SetCaps(kw[k])
        for k in ['R', 'L', 'C', 'ny', 'caps']:
            if k in kw:
                del kw[k]

        super(CSPropLumpedElement, self).__init__(pset, *args, **kw)

    def SetResistance(self, val):
        """ SetResistance(val)
        """
        self.LEptr.SetResistance(val)

    def GetResistance(self):
        return self.LEptr.GetResistance()

    def SetCapacity(self, val):
        """ SetCapacity(val)
        """
        self.LEptr.SetCapacity(val)

    def GetCapacity(self):
        return self.LEptr.GetCapacity()

    def SetInductance(self, val):
        """ SetInductance(val)
        """
        self.LEptr.SetInductance(val)

    def GetInductance(self):
        return self.LEptr.GetInductance()

    def SetDirection(self, ny):
        """ SetDirection(ny)
        """
        self.LEptr.SetDirection(CheckNyDir(ny))

    def GetDirection(self):
        return self.LEptr.GetDirection()

    def SetCaps(self, val):
        """ SetCaps(val)
        """
        self.LEptr.SetCaps(val)

    def GetCaps(self):
        return self.LEptr.GetCaps()==1


###############################################################################
cdef class CSPropMetal(CSProperties):
    """ Metal property

    A metal property is usually modeled as a perfect electric conductor (PEC).
    It does not have any further arguments.

    See Also
    --------
    CSPropConductingSheet : For all lossy conductor model with a finite conductivity
    """
    def __init__(self, ParameterSet pset, *args, **kw):
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
    def __init__(self, ParameterSet pset, *args, **kw):
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropConductingSheet(pset.thisptr)
        self.CSptr   = <_CSPropConductingSheet*> self.thisptr

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
        self.CSptr.SetConductivity(val)

    def GetConductivity(self):
        return self.CSptr.GetConductivity()

    def SetThickness(self, val):
        """ SetThickness(val)
        """
        self.CSptr.SetThickness(val)

    def GetThickness(self):
        return self.CSptr.GetThickness()

###############################################################################
cdef class CSPropExcitation(CSProperties):
    """ Excitation property

    An excitation property is defined to define the (field) sources.

    Depending on the EM modeling tool there exist different excitation types
    with different meanings.

    openEMS excitation types:

    * 0 : E-field soft excitation
    * 1 : E-field hard excitation
    * 2 : H-field soft excitation
    * 3 : H-field hard excitation
    * 10 : plane wave excitation

    :param exc_type: int -- excitation type (see above)
    """
    def __init__(self, ParameterSet pset, *args, **kw):
        if not self.thisptr:
            self.thisptr = <_CSProperties*> new _CSPropExcitation(pset.thisptr)
        self.excptr = <_CSPropExcitation*>self.thisptr

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
        """
        self.excptr.SetExcitType(val)

    def GetExcitType(self):
        return self.excptr.GetExcitType()

    def SetExcitation(self, val):
        """ SetExcitation(val)

        :param val: (3,) array -- excitation vector
        """
        assert len(val)==3, "Error, excitation vector must be of dimension 3"
        for n in range(3):
            self.excptr.SetExcitation(val[n], n)

    def GetExcitation(self):
        val = np.zeros(3)
        for n in range(3):
            val[n] = self.excptr.GetExcitation(n)
        return val

    def SetPropagationDir(self, val):
        """ SetPropagationDir(val)

        :param val: (3,) array -- propagation vector
        """
        assert len(val)==3, "Error, excitation vector must be of dimension 3"
        for n in range(3):
            self.excptr.SetPropagationDir(val[n], n)

    def GetPropagationDir(self):
        val = np.zeros(3)
        for n in range(3):
            val[n] = self.excptr.GetPropagationDir(n)
        return val

    def SetFrequency(self, val):
        """ SetFrequency(val)

        Frequency for numerical phase velocity compensation (optional)

        :param val: float -- Frequency
        """
        self.excptr.SetFrequency(val)

    def GetFrequency(self):
        return self.excptr.GetFrequency()

    def SetDelay(self, val):
        """ SetDelay(val)

        Set signal delay for this property.

        :param val: float -- Signal delay
        """
        self.excptr.SetDelay(val)

    def GetDelay(self):
        return self.excptr.GetDelay()

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
    def __init__(self, ParameterSet pset, *args, **kw):
        if not self.thisptr:
            self.thisptr = <_CSPropProbeBox*> new _CSPropProbeBox(pset.thisptr)
        self.probeptr = <_CSPropProbeBox*>self.thisptr

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
            self.SetFrequencies(kw['frequency'])
            del kw['frequency']
        if 'mode_function' in kw:
            self.SetModeFunction(kw['mode_function'])
            del kw['mode_function']
        super(CSPropProbeBox, self).__init__(pset, *args, **kw)

    def SetProbeType(self, val):
        """ SetProbeType(val)
        """
        self.probeptr.SetProbeType(val)

    def GetProbeType(self):
        return self.probeptr.GetProbeType()

    def SetWeighting(self, val):
        """ SetWeighting(val)
        """
        self.probeptr.SetWeighting(val)

    def GetWeighting(self):
        return self.probeptr.GetWeighting()

    def SetNormalDir(self, val):
        """ SetNormalDir(val)
        """
        self.probeptr.SetNormalDir(val)

    def GetNormalDir(self):
        return self.probeptr.GetNormalDir()

    def SetFrequency(self, freq):
        """ SetFrequency(freq)
        """
        self.probeptr.ClearFDSamples()
        for f in freq:
            self.probeptr.AddFDSample(f)

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
    def __init__(self, ParameterSet pset, *args, **kw):
        if not self.thisptr:
            self.thisptr = <_CSPropDumpBox*> new _CSPropDumpBox(pset.thisptr)
        self.dbptr = <_CSPropDumpBox*>self.thisptr

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
        self.dbptr.SetDumpType(val)

    def GetDumpType(self):
        return self.dbptr.GetDumpType()

    def SetDumpMode(self, val):
        """ SetDumpMode(val)
        """
        self.dbptr.SetDumpMode(val)

    def GetDumpMode(self):
        return self.dbptr.GetDumpMode()

    def SetFileType(self, val):
        """ SetFileType(val)
        """
        self.dbptr.SetFileType(val)

    def GetFileType(self):
        return self.dbptr.GetFileType()

    def SetOptResolution(self, val):
        """ SetOptResolution(val)
        """
        assert len(val)==3, 'SetOptResolution: value must be list or array of length 3'
        for n in range(3):
            self.dbptr.SetOptResolution(n, val[n])

    def GetOptResolution(self):
        val = np.zeros(3)
        for n in range(3):
            val[n] = self.dbptr.GetOptResolution(n)
        return val

    def SetSubSampling(self, val):
        """ SetSubSampling(val)
        """
        assert len(val)==3, "SetSubSampling: 'val' must be a list or array of length 3"
        for n in range(3):
            self.dbptr.SetSubSampling(n, val[n])

    def GetSubSampling(self):
        val = np.zeros(3)
        for n in range(3):
            val[n] = self.dbptr.GetSubSampling(n)
        return val
