# -*- coding: utf-8 -*-
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

"""CSXCAD.CSPrimitives
Module for all Primitives

Notes
-----
Usually it is not meant to create primitives manually, but instead
use the ContinuousStructure object to create primives using the
e.g. AddBox or AddCylinder methods.

Examples
--------
Create a metal box:

>>> pset  = ParameterObjects.ParameterSet()
>>> metal = CSProperties.CSPropMetal(pset)
>>> box   = CSPrimitives.CSPrimBox(pset, metal)
"""

import numpy as np
import sys
from libcpp.string cimport string
from libcpp cimport bool

cimport CSPrimitives
from Utilities import CheckNyDir, GetMultiDirs
cimport CSRectGrid

cdef class CSPrimitives:
    """
    Virtual base class for all primives, cannot be created!

    """
    @staticmethod
    def fromType(prim_type, pset, prop, no_init=False, **kw):
        """ fromType(prim_type, pset, prop, no_init=False, **kw)

        Create a primtive specified by the `prim_type`

        :param prim_type: Primitive type
        :param pset: ParameterSet to assign to the new primitive
        :param prop: CSProperty to assign to the new primitive
        :param no_init: do not create an actual C++ instance
        """
        prim = None
        if prim_type == POINT:
            prim = CSPrimPoint(pset, prop, no_init=no_init, **kw)
        elif prim_type == POINT:
            prim = CSPrimPoint(pset, prop, no_init=no_init, **kw)
        elif prim_type == BOX:
            prim = CSPrimBox(pset, prop, no_init=no_init, **kw)
        elif prim_type == MULTIBOX:
            raise Exception('Primitive type "MULTIBOX" not yet implemented!')
        elif prim_type == SPHERE:
            prim = CSPrimSphere(pset, prop, no_init=no_init, **kw)
        elif prim_type == SPHERICALSHELL:
            prim = CSPrimSphericalShell(pset, prop, no_init=no_init, **kw)
        elif prim_type == CYLINDER:
            prim = CSPrimCylinder(pset, prop, no_init=no_init, **kw)
        elif prim_type == CYLINDRICALSHELL:
            prim = CSPrimCylindricalShell(pset, prop, no_init=no_init, **kw)
        elif prim_type == POLYGON:
            prim = CSPrimPolygon(pset, prop, no_init=no_init, **kw)
        elif prim_type == LINPOLY:
            prim = CSPrimLinPoly(pset, prop, no_init=no_init, **kw)
        elif prim_type == ROTPOLY:
            prim = CSPrimRotPoly(pset, prop, no_init=no_init, **kw)
        elif prim_type == POLYHEDRON:
            prim = CSPrimPolyhedron(pset, prop, no_init=no_init, **kw)
        elif prim_type == CURVE:
            prim = CSPrimCurve(pset, prop, no_init=no_init, **kw)
        elif prim_type == WIRE:
            prim = CSPrimWire(pset, prop, no_init=no_init, **kw)
        elif prim_type == USERDEFINED:
            raise Exception('Primitive type "USERDEFINED" not yet implemented!')
        elif prim_type == POLYHEDRONREADER:
            prim = CSPrimPolyhedronReader(pset, prop, no_init=no_init, **kw)
        return prim

    def __init__(self, ParameterSet pset, CSProperties prop, *args, no_init=False, **kw):
        self.__transform = None
        self.__prop      = prop
        if no_init:
            self.thisptr = NULL
            return

        assert self.thisptr, 'Error, object cannot be created (protected)'
        self.__prop = prop

        if 'priority' in kw:
            self.SetPriority(kw['priority'])
            del kw['priority']
        if 'no_init' in kw:
            del kw['no_init']

        assert len(kw)==0, 'Unknown keyword arguments: "{}"'.format(kw)

    def GetID(self):
        """
        Get the ID for this primitive

        :returns: int -- ID for this primitive
        """
        return self.thisptr.GetID()

    def GetProperty(self):
        """
        Get the property for this primitive

        :returns: CSProperties.CSProperties
        """
        return self.__GetProperty()

    cdef __GetProperty(self):
        cdef _CSProperties* _prop
        cdef CSProperties prop
        if self.__prop is None:
            _prop = self.thisptr.GetProperty()
            self.__prop  = CSProperties.fromType(_prop.GetType(), pset=None, no_init=True)
            self.__prop.thisptr = _prop

        return self.__prop

    def GetType(self):
        """
        Get the type as int for this primitive

        :returns: int -- Type for this primitive (e.g. 0 --> Point, 1 --> Box, ...)
        """
        return self.thisptr.GetType()

    def GetTypeName(self):
        """
        Get the type as string (UTF-8) for this primitive

        :returns: str -- Type name for this primitive ("Point", "Box", ...)
        """
        return self.thisptr.GetTypeName().decode('UTF-8')

    def SetPriority(self, val):
        """ SetPriority(val)

        Set the priority for this primitive

        :param val: int -- Higher priority values will override primitives with a lower priority
        """
        self.thisptr.SetPriority(val)

    def GetPriority(self):
        """
        Get the priority for this primitive

        :returns: int -- Priority for this primitive
        """
        return self.thisptr.GetPriority()

    def GetBoundBox(self):
        """
        Get the bounding box for this primitive

        :returns: (2,3) ndarray -- bounding box for this primitive
        """
        bb = np.zeros([2,3])
        cdef double _bb[6]
        self.thisptr.GetBoundBox(_bb)
        for n in range(3):
            bb[0,n] = _bb[2*n]
            bb[1,n] = _bb[2*n+1]
        return bb

    def GetDimension(self):
        """
        Get the dimension of this primitive

        :returns: int -- dimension 0..3
        """
        return self.thisptr.GetDimension()

    def IsInside(self, coord, tol=0):
        """ IsInside(coord, tol=0)

        Check if a given coordinate is inside this primitive.

        :param coord: (3,) array -- coordinate
        :returns: bool
        """
        cdef double c_coord[3]
        for n in range(3):
            c_coord[n] = coord[n]

        return self.thisptr.IsInside(c_coord, tol)

    def GetPrimitiveUsed(self):
        """
        Get if this primitive has been used (used flag set)
        """
        return self.thisptr.GetPrimitiveUsed()

    def SetPrimitiveUsed(self, val):
        """ SetPrimitiveUsed(val)

        Set used flag.
        """
        self.thisptr.SetPrimitiveUsed(val)

    def __GetCSX(self):
        if self.__prop is None:
            return None
        return self.__prop.__CSX

    def GetTransform(self):
        """ GetTransform()

        Get the transformation class assigned to this primitive.
        If this primitve does not have any, it will be created.

        :return: CSTransform class

        See Also
        --------
        CSXCAD.CSTransform.CSTransform
        """
        if self.__transform is None:
            self.__transform         = CSTransform(no_init=True)
            self.__transform.thisptr = self.thisptr.GetTransform()
        return self.__transform

    def AddTransform(self, transform, *args, no_init=False, **kw):
        """ AddTransform(transform, *args, **kw)

        Add a transformation to this primitive.

        See Also
        --------
        CSXCAD.CSTransform.CSTransform.AddTransform
        """
        tr = self.GetTransform()
        tr.AddTransform(transform, *args, **kw)

    def HasTransform(self):
        """
        Check if this primitive has a transformation attached.
        It will not create one if it does not.

        :returns: bool
        """
        if self.__transform is None:
            return False
        return self.__transform.HasTransform()

    def SetCoordinateSystem(self, cs_type):
        """ SetCoordinateSystem(cs_type)

        Set the coordinate system type (Cartesian or cylindrical) for this primitive.
        If set to None, the mesh type of the assigned rect grid will be used.

        :param cs_type: coordinate system (0 : Cartesian, 1 : Cylindrical) or None

        See Also
        --------
        CSXCAD.CSRectGrid.CSRectGrid.SetMeshType

        """
        assert cs_type in [CSRectGrid.CARTESIAN, CSRectGrid.CYLINDRICAL, None], 'Unknown coordinate system: {}'.format(cs_type)
        if cs_type is None:
            cs_type = CSRectGrid.UNDEFINED_CS
        self.thisptr.SetCoordinateSystem(cs_type)

    def GetCoordinateSystem(self):
        """ GetCoordinateSystem

        :returns: coordinate system (0 : Cartesian, 1 : Cylindrical) or None
        """
        cs_type = self.thisptr.GetCoordinateSystem()
        if cs_type == CSRectGrid.UNDEFINED_CS:
            return None
        return cs_type

    def Update(self):
        """ Trigger an internal update and report success and error message

        :returns: bool, err_msg -- success and error message (empty on success)
        """
        cdef string s
        succ = self.thisptr.Update(&s)
        return succ, str(s)


###############################################################################
cdef class CSPrimPoint(CSPrimitives):
    """ Point Primitive

    A point is defined by a single 3D coordinate.

    Parameters
    ----------
    coord : (vector)
        Coordinate vector (3,) array

    """
    def __init__(self, ParameterSet pset, CSProperties prop, *args, no_init=False, **kw):
        if no_init:
            self.thisptr = NULL
            return
        if not self.thisptr:
            self.thisptr   = new _CSPrimPoint(pset.thisptr, prop.thisptr)

        if 'coord' in kw:
            self.SetCoord(kw['coord'])
            del kw['coord']
        super(CSPrimPoint, self).__init__(pset, prop, *args, **kw)

    def SetCoord(self, coord):
        """ SetCoord(coord)

        Set the coordinate for this point primitive

        :param coord: list/array of float -- Set the point coordinate
        """
        ptr = <_CSPrimPoint*>self.thisptr
        assert len(coord)==3, "CSPrimPoint:SetCoord: length of array needs to be 3"
        for n in range(3):
            ptr.SetCoord(n, coord[n])

    def GetCoord(self):
        """
        Get the point coordinate for this primitive

        :returns: (3,) ndarray -- point coordinate for this primitive
        """
        ptr = <_CSPrimPoint*>self.thisptr
        coord = np.zeros(3)
        for n in range(3):
            coord[n] = ptr.GetCoord(n)
        return coord

###############################################################################
cdef class CSPrimBox(CSPrimitives):
    """ Box Primitive

    A box is defined by two 3D coordinates. E.g. the lower-left (start)
    and upper right (stop) point.
    A box is a cube in Cartesian coordinates.
    A box is a cylinder in cylindrical coordinates.

    Parameters
    ----------
    start : (3,) array
        Start point vector (3,) array
    stop : (3,) array
        Stop point vector (3,) array

    """
    def __init__(self, ParameterSet pset, CSProperties prop, *args, no_init=False, **kw):
        if no_init:
            self.thisptr = NULL
            return
        if not self.thisptr:
            self.thisptr   = new _CSPrimBox(pset.thisptr, prop.thisptr)
        if 'start' in kw:
            self.SetStart(kw['start'])
            del kw['start']
        if 'stop' in kw:
            self.SetStop(kw['stop'])
            del kw['stop']
        super(CSPrimBox, self).__init__(pset, prop, *args, **kw)

    def SetStart(self, coord):
        """ SetStart(coord)

        Set the start coordinate for this box primitive.

        :param coord: list/array of float -- Set the start point coordinate
        """
        ptr = <_CSPrimBox*>self.thisptr
        assert len(coord)==3, "CSPrimBox:SetStart: length of array needs to be 3"
        for n in range(3):
            ptr.SetCoord(2*n, coord[n])

    def SetStop(self, coord):
        """ SetStop(coord)

        Set the stop coordinate for this box primitive.

        :param coord: list/array of float -- Set the stop point coordinate
        """
        ptr = <_CSPrimBox*>self.thisptr
        assert len(coord)==3, "CSPrimBox:SetStop: length of array needs to be 3"
        for n in range(3):
            ptr.SetCoord(2*n+1, coord[n])

    def GetStart(self):
        """
        Get the start coordinate for this primitive.

        :returns: (3,) ndarray -- Start coordinate for this primitive
        """
        ptr = <_CSPrimBox*>self.thisptr
        coord = np.zeros(3)
        for n in range(3):
            coord[n] = ptr.GetCoord(2*n)
        return coord

    def GetStop(self):
        """
        Get the stop coordinate for this primitive.

        :returns: (3,) ndarray -- Stop coordinate for this primitive
        """
        ptr = <_CSPrimBox*>self.thisptr
        coord = np.zeros(3)
        for n in range(3):
            coord[n] = ptr.GetCoord(2*n+1)
        return coord

###############################################################################
cdef class CSPrimCylinder(CSPrimitives):
    """ Cylinder Primitive

    A cylinder is defined by its axis and a radius.
    The axis is defined by two 3D coordinates (start/stop).

    Parameters
    ----------
    start : (3,) array
        Axis start point vector (3,) array
    stop : (3,) array
        Axis stop point vector (3,) array
    radius : float
        The cylinder radius

    """
    def __init__(self, ParameterSet pset, CSProperties prop, *args, no_init=False, **kw):
        if no_init:
            self.thisptr = NULL
            return
        if not self.thisptr:
            self.thisptr   = new _CSPrimCylinder(pset.thisptr, prop.thisptr)
        if 'start' in kw:
            self.SetStart(kw['start'])
            del kw['start']
        if 'stop' in kw:
            self.SetStop(kw['stop'])
            del kw['stop']
        if 'radius' in kw:
            self.SetRadius(kw['radius'])
            del kw['radius']
        super(CSPrimCylinder, self).__init__(pset, prop, *args, **kw)

    def SetStart(self, coord):
        """ SetStart(coord)

        Set the start coordinate for the cylinder axis.

        :param coord: list/array of float -- Set the axis start point coordinate.
        """
        ptr = <_CSPrimCylinder*>self.thisptr
        assert len(coord)==3, "CSPrimCylinder:SetStart: length of array needs to be 3"
        for n in range(3):
            ptr.SetCoord(2*n, coord[n])

    def SetStop(self, coord):
        """ SetStop(coord)

        Set the stop coordinate for the cylinder axis.

        :param coord: list/array of float -- Set the axis stop point coordinate.
        """
        ptr = <_CSPrimCylinder*>self.thisptr
        assert len(coord)==3, "CSPrimCylinder:SetStop: length of array needs to be 3"
        for n in range(3):
            ptr.SetCoord(2*n+1, coord[n])

    def GetStart(self):
        """
        Get the axis start coordinate.

        :returns: (3,) ndarray -- Axis start coordinate.
        """
        ptr = <_CSPrimCylinder*>self.thisptr
        coord = np.zeros(3)
        for n in range(3):
            coord[n] = ptr.GetCoord(2*n)
        return coord

    def GetStop(self):
        """
        Get the axis stop coordinate.

        :returns: (3,) ndarray -- Axis stop coordinate.
        """
        ptr = <_CSPrimCylinder*>self.thisptr
        coord = np.zeros(3)
        for n in range(3):
            coord[n] = ptr.GetCoord(2*n+1)
        return coord

    def SetRadius(self, val):
        """ SetRadius(val)

        Set the cylinder radius

        :param val: float -- Set the cylinder radius
        """
        ptr = <_CSPrimCylinder*>self.thisptr
        ptr.SetRadius(val)

    def GetRadius(self):
        """
        Get the cylinder radius.

        :returns: float -- Cylinder radius.
        """
        ptr = <_CSPrimCylinder*>self.thisptr
        return ptr.GetRadius()

###############################################################################
cdef class CSPrimCylindricalShell(CSPrimCylinder):
    """ Cylindrical Shell (hollow cylinder)

    A cylindrical shell is defined like a cylinder with an additional shell width.

    Parameters
    ----------
    shell_width : float
        Width of the cylindrical shell

    """
    def __init__(self, ParameterSet pset, CSProperties prop, *args, no_init=False, **kw):
        if no_init:
            self.thisptr = NULL
            return
        if not self.thisptr:
            self.thisptr = new _CSPrimCylindricalShell(pset.thisptr, prop.thisptr)
        if 'shell_width' in kw:
            self.SetShellWidth(kw['shell_width'])
            del kw['shell_width']
        super(CSPrimCylindricalShell, self).__init__(pset, prop, *args, **kw)

    def SetShellWidth(self, val):
        """ SetShellWidth(val)

        Set the cylinder shell width.

        :param val: float -- Set the cylinder shell width
        """
        ptr = <_CSPrimCylindricalShell*>self.thisptr
        ptr.SetShellWidth(val)

    def GetShellWidth(self):
        """
        Get the cylinder shell width.

        :returns: float -- Cylinder shell width.
        """
        ptr = <_CSPrimCylindricalShell*>self.thisptr
        return ptr.GetShellWidth()

###############################################################################
cdef class CSPrimSphere(CSPrimitives):
    """ Sphere Primitive

    A sphere is defined by its center and radius.
    The center is defined by a 3D coordinate.

    Parameters
    ----------
    center : (3,) array
        Center point vector (3,) array
    radius : float
        The sphere radius

    """
    def __init__(self, ParameterSet pset, CSProperties prop, *args, no_init=False, **kw):
        if no_init:
            self.thisptr = NULL
            return
        if not self.thisptr:
            self.thisptr   = new _CSPrimSphere(pset.thisptr, prop.thisptr)
        if 'center' in kw:
            self.SetCenter(kw['center'])
            del kw['center']
        if 'radius' in kw:
            self.SetRadius(kw['radius'])
            del kw['radius']
        super(CSPrimSphere, self).__init__(pset, prop, *args, **kw)

    def SetCenter(self, coord):
        """ SetRadius(val)

        Set the sphere center point.

        :param coord: (3,) array -- Set the sphere center point.
        """
        ptr = <_CSPrimSphere*>self.thisptr
        assert len(coord)==3, "CSPrimSphere:SetCenter: length of array needs to be 3"
        ptr.SetCenter(coord[0], coord[1], coord[2])

    def GetCenter(self):
        """
        Get the sphere center point.

        :returns: (3,) ndarray -- Center coordinate.
        """
        ptr = <_CSPrimSphere*>self.thisptr
        coord = np.zeros(3)
        for n in range(3):
            coord[n] = ptr.GetCoord(n)
        return coord

    def SetRadius(self, val):
        """ SetRadius(val)

        Set the sphere radius

        :param val: float -- Set the sphere radius
        """
        ptr = <_CSPrimSphere*>self.thisptr
        ptr.SetRadius(val)

    def GetRadius(self):
        """
        Get the sphere radius.

        :returns: float -- Sphere radius.
        """
        ptr = <_CSPrimSphere*>self.thisptr
        return ptr.GetRadius()

###############################################################################
cdef class CSPrimSphericalShell(CSPrimSphere):
    """ Spherical Shell (hollow sphere) Primitive Class

    A spherical shell is defined like a sphere with an additional shell width.

    Parameters
    ----------
    shell_width : float
        Width of the spherical shell

    """
    def __init__(self, ParameterSet pset, CSProperties prop, *args, no_init=False, **kw):
        if no_init:
            self.thisptr = NULL
            return
        if not self.thisptr:
            self.thisptr = new _CSPrimSphericalShell(pset.thisptr, prop.thisptr)
        if 'shell_width' in kw:
            self.SetShellWidth(kw['shell_width'])
            del kw['shell_width']
        super(CSPrimSphericalShell, self).__init__(pset, prop, *args, **kw)

    def SetShellWidth(self, val):
        """ SetShellWidth(val)

        Set the sphere shell width.

        :param val: float -- Set the sphere shell width
        """
        ptr = <_CSPrimSphericalShell*>self.thisptr
        ptr.SetShellWidth(val)

    def GetShellWidth(self):
        """
        Get the sphere shell width.

        :returns: float -- sphere shell width.
        """
        ptr = <_CSPrimSphericalShell*>self.thisptr
        return ptr.GetShellWidth()

###############################################################################
cdef class CSPrimPolygon(CSPrimitives):
    """ Polygon Primitive

    A polygon is a surface defined by a set of 2D points/coordinates.
    To place the polygon in a 3D space, a normal direction (x/y/z) and an
    elevation in this direction have to be specified.

    Parameters
    ----------
    points : (N,2) array
        Array of coordinates
    norm_dir : float or str
        Normal direction, either 0,1,2 or x/y/z respectively
    elevation : float
        Elevation in normal direciton

    Examples
    --------
    Define a half circle as polygon

    >>> polygon = CSPrimitives.CSPrimBox(pset, metal, norm_dir='z', elevation=1.0)
    >>> ang     = np.linspace(0, np.pi, 21)
    >>> polygon.SetCoords(5*np.cos(ang), 5*np.sin(ang))
    """
    def __init__(self, ParameterSet pset, CSProperties prop, *args, no_init=False, **kw):
        if no_init:
            self.thisptr = NULL
            return
        if not self.thisptr:
            self.thisptr = new _CSPrimPolygon(pset.thisptr, prop.thisptr)
        if 'points' in kw:
            assert len(kw['points'])==2, 'points must be a list of length 2'
            self.SetCoords(kw['points'][0], kw['points'][1])
            del kw['points']
        if 'norm_dir' in kw:
            self.SetNormDir(kw['norm_dir'])
            del kw['norm_dir']
        if 'elevation' in kw:
            self.SetElevation(kw['elevation'])
            del kw['elevation']
        super(CSPrimPolygon, self).__init__(pset, prop, *args, **kw)

    def SetCoords(self, x0, x1):
        """ SetCoords(x0, x1)

        Set the coordinates for the polygon. This will clear all previous coordinates.

        :param x0, x1: (N,), (N,) Two arrays for x0/x1 of the polygon coordinates.
        """
        assert len(x0)==len(x1), 'SetCoords: x0/x1 do not have the same length'
        assert len(x0)>0, 'SetCoords: empty coordinates'

        ptr = <_CSPrimPolygon*>self.thisptr
        ptr.ClearCoords()
        for n in range(len(x0)):
            ptr.AddCoord(x0[n])
            ptr.AddCoord(x1[n])

    def GetCoords(self):
        """
        Get the coordinates for the polygon

        :return x0, x1: (N,), (N,) Arrays for x0,x1 of the polygon coordinates
        """
        ptr = <_CSPrimPolygon*>self.thisptr
        N = ptr.GetQtyCoords()
        x0 = np.zeros(N)
        x1 = np.zeros(N)
        for n in range(N):
            x0[n] = ptr.GetCoord(2*n)
            x1[n] = ptr.GetCoord(2*n+1)
        return x0, x1

    def GetQtyCoords(self):
        """
        Get the number of coordinates for the polygon

        :return val: int -- Number of polygon coordinates.
        """
        ptr = <_CSPrimPolygon*>self.thisptr
        return ptr.GetQtyCoords()

    def ClearCoords(self):
        """
        Remove all coordinates.
        """
        ptr = <_CSPrimPolygon*>self.thisptr
        ptr.ClearCoords()

    def SetNormDir(self, ny):
        """ SetNormDir(ny)

        Set the normal direction.

        :param ny: int or string -- Normal direction, either 0/1/2 or 'x'/'y'/'z'
        """
        ptr = <_CSPrimPolygon*>self.thisptr
        ptr.SetNormDir(CheckNyDir(ny))

    def GetNormDir(self):
        """
        Get the normal direction.

        :return ny: int -- Normal direction as 0, 1 or 2 meaning x,y or z
        """
        ptr = <_CSPrimPolygon*>self.thisptr
        return ptr.GetNormDir()

    def SetElevation(self, val):
        """ SetElevation(val)

        Set the elevation in normal direction.

        :param val: float -- Elevation in normal direction.
        """
        ptr = <_CSPrimPolygon*>self.thisptr
        ptr.SetElevation(val)

    def GetElevation(self):
        """
        Get the elevation in normal direction.

        :return val: float -- Get the elevation in normal direction.
        """
        ptr = <_CSPrimPolygon*>self.thisptr
        return ptr.GetElevation()

###############################################################################
cdef class CSPrimLinPoly(CSPrimPolygon):
    """ Linear Extruded Polygon

    A linear extruded polygon is a polygon that is extruded in normal direction
    for a certain length above the elevation.

    Parameters
    ----------
    length : float
        Extrusion length in normal direction

    Examples
    --------
    Define a half circle with a height (in z direction) of 1

    >>> linpoly = CSPrimitives.CSPrimLinPoly(pset, metal, norm_dir='z', elevation=0.5)
    >>> ang     = np.linspace(0, np.pi, 21)
    >>> linpoly.SetCoords(5*np.cos(ang), 5*np.sin(ang))
    >>> linpoly.SetLength(1.0)
    """
    def __init__(self, ParameterSet pset, CSProperties prop, *args, no_init=False, **kw):
        if no_init:
            self.thisptr = NULL
            return
        if not self.thisptr:
            self.thisptr = new _CSPrimLinPoly(pset.thisptr, prop.thisptr)
        if 'length' in kw:
            self.SetLength(kw['length'])
            del kw['length']
        super(CSPrimLinPoly, self).__init__(pset, prop, *args, **kw)

    def SetLength(self, val):
        """ SetLength(val)

        Set the extrusion length in normal direction.

        :param val: float -- Extrusion length in normal direction.
        """
        ptr = <_CSPrimLinPoly*>self.thisptr
        ptr.SetLength(val)

    def GetLength(self):
        """
        Get the extrusion length in normal direction.

        :return val: float -- Get the extrusion length in normal direction.
        """
        ptr = <_CSPrimLinPoly*>self.thisptr
        return ptr.GetLength()

###############################################################################
cdef class CSPrimRotPoly(CSPrimPolygon):
    """ Rotation Extruded Polygon

    A rotation extruded polygon is a polygon that is rotated around a Cartesian
    axis with a given start and stop angle.

    Parameters
    ----------
    rot_axis : float or str
        Rotation axis direction, either 0,1,2 or x/y/z respectively.
    angle : float, float
        Start/Stop angle (rad) of rotation. Default is (0, 2*pi).

    Examples
    --------
    Define a half circle on the xy-plane, rotated around the x axis

    >>> rotpoly = CSPrimitives.CSPrimRotPoly(pset, metal, norm_dir='z')
    >>> ang     = np.linspace(0, np.pi, 21)
    >>> rotpoly.SetCoords(5*np.cos(ang), 5*np.sin(ang))
    >>> rotpoly.SetRotAxisDir('x')
    """
    def __init__(self, ParameterSet pset, CSProperties prop, *args, no_init=False, **kw):
        if no_init:
            self.thisptr = NULL
            return
        if not self.thisptr:
            self.thisptr = new _CSPrimRotPoly(pset.thisptr, prop.thisptr)
        if 'rot_axis' in kw:
            self.SetRotAxisDir(kw['rot_axis'])
            del kw['rot_axis']
        if 'angle' in kw:
            assert len(kw['angle'])==2, 'angle must be a list/array of length 2'
            self.SetAngle(kw['angle'][0],kw['angle'][1])
            del kw['angle']
        super(CSPrimRotPoly, self).__init__(pset, prop, *args, **kw)

    def SetRotAxisDir(self, ny):
        """ SetRotAxisDir(ny)

        Set the rotation axis direction, either 0,1,2 or x/y/z respectively.

        :param ny: int or str -- Rotation axis direction, either 0,1,2 or x/y/z respectively.
        """
        ptr = <_CSPrimRotPoly*>self.thisptr
        ptr.SetRotAxisDir(CheckNyDir(ny))

    def GetRotAxisDir(self):
        """
        Get the rotation axis direction

        :returns ny: int -- Rotation axis direction as 0, 1 or 2 meaning x,y or z
        """
        ptr = <_CSPrimRotPoly*>self.thisptr
        return ptr.GetRotAxisDir()

    def SetAngle(self, a0, a1):
        """ SetAngle(a0, a1)

        Set the start/stop angle (rad) of rotation. Default is (0, 2*pi).

        :param a0: float -- Start angle (rad) of rotation.
        :param a1: float -- Stop angle (rad) of rotation.
        """
        ptr = <_CSPrimRotPoly*>self.thisptr
        ptr.SetAngle(0, a0)
        ptr.SetAngle(1, a1)

    def GetAngle(self):
        """
        Get the start/stop angle (rad) of rotation.

        :returns a0, a1: float, float -- Start/Stop angle (rad) of rotation.
        """
        ptr = <_CSPrimRotPoly*>self.thisptr
        return ptr.GetAngle(0), ptr.GetAngle(1)

###############################################################################
cdef class CSPrimCurve(CSPrimitives):
    """ Curve Primitive

    A curve is a set of consequtive 3D coordinates.

    Parameters
    ----------
    points : (3, N) array
        Array of 3D coordinates.

    Examples
    --------

    >>> x = np.array([0, 0, 1, 1]) + 1.5
    >>> y = np.array([0, 1, 1, 0]) + 2.5
    >>> z = np.array([0, 1, 3, 4])
    >>> curve = CSPrimitives.CSPrimCurve(pset, metal, points=[x,y,z])
    >>> curve.AddPoint([2, 0, 5])
    """
    def __init__(self, ParameterSet pset, CSProperties prop, *args, no_init=False, **kw):
        if no_init:
            self.thisptr = NULL
            return
        if not self.thisptr:
            self.thisptr = new _CSPrimCurve(pset.thisptr, prop.thisptr)
        if 'points' in kw:
            assert len(kw['points'])==3, 'points list not of length 3'
            self.SetPoints(kw['points'][0], kw['points'][1], kw['points'][2])
            del kw['points']
        super(CSPrimCurve, self).__init__(pset, prop, *args, **kw)

    def AddPoint(self, point):
        """ AddPoint(point)

        Add a single point to the list.

        :param point: (3,) array -- Add a single 3D point
        """
        assert len(point)==3, "CSPrimSphere:SetCenter: length of array needs to be 3"
        ptr = <_CSPrimCurve*>self.thisptr
        cdef double dp[3]
        for n in range(3):
            dp[n] = point[n]
        ptr.AddPoint(dp)

    def SetPoints(self, x, y, z):
        """ SetPoints(x, y, z)

        Set an array of 3D coordinates

        :param x,y,z: each (N,) array -- Array of 3D point components
        """
        assert len(x)==len(y)==len(z), 'SetPoints: each component must be of equal length'
        assert len(x)>0, 'SetPoints: empty list!'
        ptr = <_CSPrimCurve*>self.thisptr
        ptr.ClearPoints()
        cdef double dp[3]
        for n in range(len(x)):
            dp[0] = x[n]
            dp[1] = y[n]
            dp[2] = z[n]
            ptr.AddPoint(dp)

    def GetPoint(self, idx):
        """ GetPoint(idx)

        Get a point with a given index.

        :param idx: int -- Index of point requested.
        :return point: (3,) array -- Point coordinate at index `idx`
        """
        ptr = <_CSPrimCurve*>self.thisptr
        cdef double dp[3]
        assert ptr.GetPoint(idx, dp), 'GetPoint: invalid index'
        point = np.zeros(3)
        for n in range(3):
            point[n] = dp[n]
        return point

    def ClearPoints(self):
        """
        Clear all points.
        """
        ptr = <_CSPrimCurve*>self.thisptr
        ptr.ClearPoints()

    def GetNumberOfPoints(self):
        """
        Get the number of points.

        :return num: int -- Get the number of points.
        """
        ptr = <_CSPrimCurve*>self.thisptr
        return ptr.GetNumberOfPoints()


###############################################################################
cdef class CSPrimWire(CSPrimCurve):
    """ Wire Primitive

    A wire is a curve with a given radius.

    Parameters
    ----------
    radius : float
        Wire radius
    """
    def __init__(self, ParameterSet pset, CSProperties prop, *args, no_init=False, **kw):
        if no_init:
            self.thisptr = NULL
            return
        if not self.thisptr:
            self.thisptr = new _CSPrimWire(pset.thisptr, prop.thisptr)
        if 'radius' in kw:
            self.SetWireRadius(kw['radius'])
            del kw['radius']
        super(CSPrimWire, self).__init__(pset, prop, *args, **kw)


    def SetWireRadius(self, val):
        """ SetWireRadius(val)

        Set the wire radius

        :param val: float -- Set the wire radius
        """
        ptr = <_CSPrimWire*>self.thisptr
        ptr.SetWireRadius(val)

    def GetWireRadius(self):
        """
        Get the wire radius.

        :returns: float -- Wire radius.
        """
        ptr = <_CSPrimWire*>self.thisptr
        return ptr.GetWireRadius()

###############################################################################
cdef class CSPrimPolyhedron(CSPrimitives):
    """ Polyhedron Primitive

    A polyhedron is a 3D solid with flat polygonal faces (currently only triangles).

    """
    def __init__(self, ParameterSet pset, CSProperties prop, *args, no_init=False, **kw):
        if no_init:
            self.thisptr = NULL
            return
        if not self.thisptr:
            self.thisptr = new _CSPrimPolyhedron(pset.thisptr, prop.thisptr)
        super(CSPrimPolyhedron, self).__init__(pset, prop, *args, **kw)

    def Reset(self):
        """
        Reset the polyhedron, that means removeing all faces.
        """
        ptr = <_CSPrimPolyhedron*>self.thisptr
        ptr.Reset()

    def AddVertex(self, x, y, z):
        """ AddVertex(x, y, z)

        Add a single 3D vertex.

        :param x,y,z: float,float,float -- 3D vertex
        """
        ptr = <_CSPrimPolyhedron*>self.thisptr
        ptr.AddVertex(x, y, z)

    def GetVertex(self, idx):
        """ GetVertex(idx)

        Get the vertex with index `idx`.

        :param idx: int -- Vertex index to return
        :returns point: (3,) array -- Vertex coordinate at index `idx`
        """
        ptr = <_CSPrimPolyhedron*>self.thisptr
        assert idx>=0 and idx<ptr.GetNumVertices(), "Error: invalid vertex index"
        cdef float* p
        p = ptr.GetVertex(idx)
        assert p!=NULL
        pyp = np.zeros(3)
        for n in range(3):
            pyp[n] = p[n]
        return pyp

    def GetNumVertices(self):
        """
        Get the number of vertices.

        :returns num: int -- Number of vertices
        """
        ptr = <_CSPrimPolyhedron*>self.thisptr
        return ptr.GetNumVertices()

    def AddFace(self, verts):
        """ AddFace(verts)

        Add a face with a given list of vertices.
        The vertices have to be added already.
        Currently only triangle faces are possible.

        :params verts: (N,) array -- Face with N vericies (currently N=3!)
        """
        assert len(verts)==3, 'AddFace: currently only triangles allowed as faces'
        cdef int i_v[3]
        for n in range(3):
            i_v[n] = verts[n]
        ptr = <_CSPrimPolyhedron*>self.thisptr
        ptr.AddFace(len(verts), i_v)

    def GetFace(self, idx):
        """ GetFace(idx)

        Get the face vertex indicies for the given face index `idx`

        :param idx: int -- Face index to return
        :returns: (N,) array -- Vertices array for face with index `idx`
        """
        ptr = <_CSPrimPolyhedron*>self.thisptr
        assert idx>=0 and idx<ptr.GetNumFaces(), "Error: invalid face index"
        cdef int *i_v
        cdef unsigned int numVert=0
        i_v = ptr.GetFace(idx, numVert)
        assert i_v!=NULL
        face = np.zeros(numVert, np.int)
        for n in range(numVert):
            face[n] = i_v[n]
        return face

    def GetNumFaces(self):
        """
        Get the number of faces.

        :return num: int -- number of faces
        """
        ptr = <_CSPrimPolyhedron*>self.thisptr
        return ptr.GetNumFaces()

###############################################################################
cdef class CSPrimPolyhedronReader(CSPrimPolyhedron):
    """ Polyhedron Reader

    This primives creates a polyhedron by reading a STL or PLY file.

    Parameters
    ----------
    filename : str
        File name to read
    """
    def __init__(self, ParameterSet pset, CSProperties prop, *args, no_init=False, **kw):
        if no_init:
            self.thisptr = NULL
            return
        if not self.thisptr:
            self.thisptr = new _CSPrimPolyhedronReader(pset.thisptr, prop.thisptr)
        if 'filename' in kw:
            self.SetFilename(kw['filename'])
            del kw['filename']
        super(CSPrimPolyhedronReader, self).__init__(pset, prop, *args, **kw)

    def SetFilename(self, fn):
        """ SetFilename(fn)

        Set the file name to read. This will try set the propper file type as well.

        :param fn: str -- File name to read
        """
        ptr = <_CSPrimPolyhedronReader*>self.thisptr
        if fn.endswith('.stl'):
            self.SetFileType(1)
        elif fn.endswith('.ply'):
            self.SetFileType(2)
        else:
            self.SetFileType(0)
        ptr.SetFilename(fn.encode('UTF-8'))

    def GetFilename(self):
        """
        Get the file name.

        :returns fn: str -- File name to read
        """
        ptr = <_CSPrimPolyhedronReader*>self.thisptr
        return ptr.GetFilename()

    def SetFileType(self, t):
        """ SetFileType(t)

        Set the file type. 1 --> STL-File, 2 --> PLY, 0 --> other/unknown

        :param t: int -- File type (see above)
        """
        ptr = <_CSPrimPolyhedronReader*>self.thisptr
        ptr.SetFileType(t)

    def GetFileType(self):
        """
        Get the file type. 1 --> STL-File, 2 --> PLY, 0 --> other/unknown

        :return t: int -- File type (see above)
        """
        ptr = <_CSPrimPolyhedronReader*>self.thisptr
        return ptr.GetFileType()

    def ReadFile(self):
        """
        Issue to read the file.

        :return succ: bool -- True on successful read
        """
        ptr = <_CSPrimPolyhedronReader*>self.thisptr
        return ptr.ReadFile()
