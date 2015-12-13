# -*- coding: utf-8 -*-
"""
Created on Sat Dec  5 11:45:33 2015

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
from libcpp.string cimport string
from libcpp cimport bool

cimport CSPrimitives
from Utilities import CheckNyDir

cdef class CSPrimitives:
    def __init__(self, ParameterSet pset, CSProperties prop):
        assert self.thisptr, 'Error, object cannot be created (protected)'

    def __dealloc__(self):
        pass
#        print("del")
#        del self.thisptr
#        print("del done")
    def GetID(self):
        return self.thisptr.GetID()
    def GetType(self):
        return self.thisptr.GetType()
    def GetTypeName(self):
        return self.thisptr.GetTypeName().decode('UTF-8')

    def SetPriority(self, val):
        self.thisptr.SetPriority(val)
    def GetPriority(self):
        return self.thisptr.GetPriority()

    def GetBoundBox(self):
        bb = np.zeros([2,3])
        cdef double _bb[6]
        self.thisptr.GetBoundBox(_bb)
        for n in range(3):
            bb[0,n] = _bb[2*n]
            bb[1,n] = _bb[2*n+1]
        return bb

###############################################################################
cdef class CSPrimPoint(CSPrimitives):
    def __init__(self, ParameterSet pset, CSProperties prop):
        if not self.thisptr:
            self.thisptr   = new _CSPrimPoint(pset.thisptr, prop.thisptr)
        super(CSPrimPoint, self).__init__(pset, prop)

    def SetCoord(self, coord):
        ptr = <_CSPrimPoint*>self.thisptr
        assert len(coord)==3, "CSPrimPoint:SetCoord: length of array needs to be 3"
        for n in range(3):
            ptr.SetCoord(n, coord[n])

    def GetCoord(self):
        ptr = <_CSPrimPoint*>self.thisptr
        coord = np.zeros(3)
        for n in range(3):
            coord[n] = ptr.GetCoord(n)
        return coord

###############################################################################
cdef class CSPrimBox(CSPrimitives):
    def __init__(self, ParameterSet pset, CSProperties prop):
        if not self.thisptr:
            self.thisptr   = new _CSPrimBox(pset.thisptr, prop.thisptr)
        super(CSPrimBox, self).__init__(pset, prop)

    def SetStart(self, coord):
        ptr = <_CSPrimBox*>self.thisptr
        assert len(coord)==3, "CSPrimBox:SetStart: length of array needs to be 3"
        for n in range(3):
            ptr.SetCoord(2*n, coord[n])
    def SetStop(self, coord):
        ptr = <_CSPrimBox*>self.thisptr
        assert len(coord)==3, "CSPrimBox:SetStop: length of array needs to be 3"
        for n in range(3):
            ptr.SetCoord(2*n+1, coord[n])

    def GetStart(self):
        ptr = <_CSPrimBox*>self.thisptr
        coord = np.zeros(3)
        for n in range(3):
            coord[n] = ptr.GetCoord(2*n)
        return coord
    def GetStop(self):
        ptr = <_CSPrimBox*>self.thisptr
        coord = np.zeros(3)
        for n in range(3):
            coord[n] = ptr.GetCoord(2*n+1)
        return coord

###############################################################################
cdef class CSPrimCylinder(CSPrimitives):
    def __init__(self, ParameterSet pset, CSProperties prop):
        if not self.thisptr:
            self.thisptr   = new _CSPrimCylinder(pset.thisptr, prop.thisptr)
        super(CSPrimCylinder, self).__init__(pset, prop)

    def SetStart(self, coord):
        ptr = <_CSPrimCylinder*>self.thisptr
        assert len(coord)==3, "CSPrimCylinder:SetStart: length of array needs to be 3"
        for n in range(3):
            ptr.SetCoord(2*n, coord[n])
    def SetStop(self, coord):
        ptr = <_CSPrimCylinder*>self.thisptr
        assert len(coord)==3, "CSPrimCylinder:SetStop: length of array needs to be 3"
        for n in range(3):
            ptr.SetCoord(2*n+1, coord[n])

    def GetStart(self):
        ptr = <_CSPrimCylinder*>self.thisptr
        coord = np.zeros(3)
        for n in range(3):
            coord[n] = ptr.GetCoord(2*n)
        return coord
    def GetStop(self):
        ptr = <_CSPrimCylinder*>self.thisptr
        coord = np.zeros(3)
        for n in range(3):
            coord[n] = ptr.GetCoord(2*n+1)
        return coord

    def SetRadius(self, val):
        ptr = <_CSPrimCylinder*>self.thisptr
        ptr.SetRadius(val)
    def GetRadius(self):
        ptr = <_CSPrimCylinder*>self.thisptr
        return ptr.GetRadius()

###############################################################################
cdef class CSPrimCylindricalShell(CSPrimCylinder):
    def __init__(self, ParameterSet pset, CSProperties prop):
        if not self.thisptr:
            self.thisptr = new _CSPrimCylindricalShell(pset.thisptr, prop.thisptr)
        super(CSPrimCylindricalShell, self).__init__(pset, prop)

    def SetShellWidth(self, val):
        ptr = <_CSPrimCylindricalShell*>self.thisptr
        ptr.SetShellWidth(val)

    def GetShellWidth(self):
        ptr = <_CSPrimCylindricalShell*>self.thisptr
        return ptr.GetShellWidth()

###############################################################################
cdef class CSPrimSphere(CSPrimitives):
    def __init__(self, ParameterSet pset, CSProperties prop):
        if not self.thisptr:
            self.thisptr   = new _CSPrimSphere(pset.thisptr, prop.thisptr)
        super(CSPrimSphere, self).__init__(pset, prop)

    def SetCenter(self, coord):
        ptr = <_CSPrimSphere*>self.thisptr
        assert len(coord)==3, "CSPrimSphere:SetCenter: length of array needs to be 3"
        ptr.SetCenter(coord[0], coord[1], coord[2])

    def GetCenter(self):
        ptr = <_CSPrimSphere*>self.thisptr
        coord = np.zeros(3)
        for n in range(3):
            coord[n] = ptr.GetCoord(n)
        return coord

    def SetRadius(self, val):
        ptr = <_CSPrimSphere*>self.thisptr
        ptr.SetRadius(val)
    def GetRadius(self):
        ptr = <_CSPrimSphere*>self.thisptr
        return ptr.GetRadius()

###############################################################################
cdef class CSPrimSphericalShell(CSPrimSphere):
    def __init__(self, ParameterSet pset, CSProperties prop):
        if not self.thisptr:
            self.thisptr = new _CSPrimSphericalShell(pset.thisptr, prop.thisptr)
        super(CSPrimSphericalShell, self).__init__(pset, prop)

    def SetShellWidth(self, val):
        ptr = <_CSPrimSphericalShell*>self.thisptr
        ptr.SetShellWidth(val)

    def GetShellWidth(self):
        ptr = <_CSPrimSphericalShell*>self.thisptr
        return ptr.GetShellWidth()

###############################################################################
cdef class CSPrimPolygon(CSPrimitives):
    def __init__(self, ParameterSet pset, CSProperties prop):
        if not self.thisptr:
            self.thisptr = new _CSPrimPolygon(pset.thisptr, prop.thisptr)
        super(CSPrimPolygon, self).__init__(pset, prop)

    def SetCoords(self, x0, x1):
        assert len(x0)==len(x1)
        assert len(x0)>0

        ptr = <_CSPrimPolygon*>self.thisptr
        ptr.ClearCoords()
        for n in range(len(x0)):
            ptr.AddCoord(x0[n])
            ptr.AddCoord(x1[n])

    def GetCoords(self):
        ptr = <_CSPrimPolygon*>self.thisptr
        N = ptr.GetQtyCoords()
        x0 = np.zeros(N)
        x1 = np.zeros(N)
        for n in range(N):
            x0[n] = ptr.GetCoord(2*n)
            x1[n] = ptr.GetCoord(2*n+1)
        return x0, x1

    def GetQtyCoords(self):
        ptr = <_CSPrimPolygon*>self.thisptr
        return ptr.GetQtyCoords()

    def Clear(self):
        ptr = <_CSPrimPolygon*>self.thisptr
        ptr.ClearCoords()

    def SetNormDir(self, ny):
        ptr = <_CSPrimPolygon*>self.thisptr
        ptr.SetNormDir(CheckNyDir(ny))

    def GetNormDir(self):
        ptr = <_CSPrimPolygon*>self.thisptr
        return ptr.GetNormDir()

    def SetElevation(self, val):
        ptr = <_CSPrimPolygon*>self.thisptr
        ptr.SetElevation(val)

    def GetElevation(self):
        ptr = <_CSPrimPolygon*>self.thisptr
        return ptr.GetElevation()

###############################################################################
cdef class CSPrimLinPoly(CSPrimPolygon):
    def __init__(self, ParameterSet pset, CSProperties prop):
        if not self.thisptr:
            self.thisptr = new _CSPrimLinPoly(pset.thisptr, prop.thisptr)
        super(CSPrimLinPoly, self).__init__(pset, prop)

    def SetLength(self, val):
        ptr = <_CSPrimLinPoly*>self.thisptr
        ptr.SetLength(val)

    def GetLength(self):
        ptr = <_CSPrimLinPoly*>self.thisptr
        return ptr.GetLength()

###############################################################################
cdef class CSPrimRotPoly(CSPrimPolygon):
    def __init__(self, ParameterSet pset, CSProperties prop):
        if not self.thisptr:
            self.thisptr = new _CSPrimRotPoly(pset.thisptr, prop.thisptr)
        super(CSPrimRotPoly, self).__init__(pset, prop)

    def SetRotAxisDir(self, ny):
        ptr = <_CSPrimRotPoly*>self.thisptr
        ptr.SetRotAxisDir(CheckNyDir(ny))

    def GetRotAxisDir(self):
        ptr = <_CSPrimRotPoly*>self.thisptr
        return ptr.GetRotAxisDir()

    def SetAngle(self, a0, a1):
        ptr = <_CSPrimRotPoly*>self.thisptr
        ptr.SetAngle(0, a0)
        ptr.SetAngle(1, a1)

    def GetAngle(self):
        ptr = <_CSPrimRotPoly*>self.thisptr
        return ptr.GetAngle(0), ptr.GetAngle(1)

###############################################################################
cdef class CSPrimCurve(CSPrimitives):
    def __init__(self, ParameterSet pset, CSProperties prop):
        if not self.thisptr:
            self.thisptr = new _CSPrimCurve(pset.thisptr, prop.thisptr)
        super(CSPrimCurve, self).__init__(pset, prop)

    def AddPoint(self, point):
        assert len(point)==3, "CSPrimSphere:SetCenter: length of array needs to be 3"
        ptr = <_CSPrimCurve*>self.thisptr
        cdef double dp[3]
        for n in range(3):
            dp[n] = point[n]
        ptr.AddPoint(dp)

    def SetPoints(self, x, y, z):
        assert len(x)==len(y)==len(z)
        assert len(x)>0
        ptr = <_CSPrimCurve*>self.thisptr
        ptr.ClearPoints()
        cdef double dp[3]
        for n in range(len(x)):
            dp[0] = x[n]
            dp[1] = y[n]
            dp[2] = z[n]
            ptr.AddPoint(dp)

    def GetPoint(self, idx):
        ptr = <_CSPrimCurve*>self.thisptr
        cdef double dp[3]
        assert ptr.GetPoint(idx, dp)
        point = np.zeros(3)
        for n in range(3):
            point[n] = dp[n]
        return point

    def ClearPoints(self):
        ptr = <_CSPrimCurve*>self.thisptr
        ptr.ClearPoints()

    def GetNumberOfPoints(self):
        ptr = <_CSPrimCurve*>self.thisptr
        return ptr.GetNumberOfPoints()


###############################################################################
cdef class CSPrimWire(CSPrimCurve):
    def __init__(self, ParameterSet pset, CSProperties prop):
        if not self.thisptr:
            self.thisptr = new _CSPrimWire(pset.thisptr, prop.thisptr)
        super(CSPrimWire, self).__init__(pset, prop)

    def SetWireRadius(self, val):
        ptr = <_CSPrimWire*>self.thisptr
        ptr.SetWireRadius(val)

    def GetWireRadius(self):
        ptr = <_CSPrimWire*>self.thisptr
        return ptr.GetWireRadius()

###############################################################################
cdef class CSPrimPolyhedron(CSPrimitives):
    def __init__(self, ParameterSet pset, CSProperties prop):
        if not self.thisptr:
            self.thisptr = new _CSPrimPolyhedron(pset.thisptr, prop.thisptr)
        super(CSPrimPolyhedron, self).__init__(pset, prop)

    def Reset(self):
        ptr = <_CSPrimPolyhedron*>self.thisptr
        ptr.Reset()

    def AddVertex(self, x, y, z):
        ptr = <_CSPrimPolyhedron*>self.thisptr
        ptr.AddVertex(x, y, z)

    def GetVertex(self, idx):
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
        ptr = <_CSPrimPolyhedron*>self.thisptr
        return ptr.GetNumVertices()

    def AddFace(self, verts):
        assert len(verts)==3, 'AddFace: currently only triangles allowed as faces'
        cdef int i_v[3]
        for n in range(3):
            i_v[n] = verts[n]
        ptr = <_CSPrimPolyhedron*>self.thisptr
        ptr.AddFace(len(verts), i_v)

    def GetFace(self, idx):
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
        ptr = <_CSPrimPolyhedron*>self.thisptr
        return ptr.GetNumFaces()

###############################################################################
cdef class CSPrimPolyhedronReader(CSPrimPolyhedron):
    def __init__(self, ParameterSet pset, CSProperties prop):
        if not self.thisptr:
            self.thisptr = new _CSPrimPolyhedronReader(pset.thisptr, prop.thisptr)
        super(CSPrimPolyhedronReader, self).__init__(pset, prop)

    def SetFilename(self, fn):
        ptr = <_CSPrimPolyhedronReader*>self.thisptr
        if fn.endswith('.stl'):
            self.SetFileType(1)
        elif fn.endswith('.ply'):
            self.SetFileType(2)
        else:
            self.SetFileType(0)
        ptr.SetFilename(fn.encode('UTF-8'))

    def GetFilename(self):
        ptr = <_CSPrimPolyhedronReader*>self.thisptr
        return ptr.GetFilename()

    def SetFileType(self, t):
        ptr = <_CSPrimPolyhedronReader*>self.thisptr
        ptr.SetFileType(t)

    def GetFileType(self):
        ptr = <_CSPrimPolyhedronReader*>self.thisptr
        return ptr.GetFileType()

    def ReadFile(self):
        ptr = <_CSPrimPolyhedronReader*>self.thisptr
        return ptr.ReadFile()
