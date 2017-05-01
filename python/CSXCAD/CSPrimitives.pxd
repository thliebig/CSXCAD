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

from libcpp.string cimport string
from libcpp cimport bool

from ParameterObjects cimport _ParameterSet, ParameterSet
from CSProperties cimport _CSProperties, CSProperties
from CSTransform cimport _CSTransform, CSTransform
from CSRectGrid cimport CoordinateSystem

cdef extern from "CSXCAD/CSPrimitives.h":
    cpdef enum PrimitiveType "CSPrimitives::PrimitiveType":
        POINT             "CSPrimitives::POINT"
        BOX               "CSPrimitives::BOX"
        MULTIBOX          "CSPrimitives::MULTIBOX"
        SPHERE            "CSPrimitives::SPHERE"
        SPHERICALSHELL    "CSPrimitives::SPHERICALSHELL"
        CYLINDER          "CSPrimitives::CYLINDER"
        CYLINDRICALSHELL  "CSPrimitives::CYLINDRICALSHELL"
        POLYGON           "CSPrimitives::POLYGON"
        LINPOLY           "CSPrimitives::LINPOLY"
        ROTPOLY           "CSPrimitives::ROTPOLY"
        POLYHEDRON        "CSPrimitives::POLYHEDRON"
        CURVE             "CSPrimitives::CURVE"
        WIRE              "CSPrimitives::WIRE"
        USERDEFINED       "CSPrimitives::USERDEFINED"
        POLYHEDRONREADER  "CSPrimitives::POLYHEDRONREADER"


cdef extern from "CSXCAD/CSPrimitives.h":
    cdef cppclass _CSPrimitives "CSPrimitives":
            _CSPrimitives(_ParameterSet*, _CSProperties*) except +
            unsigned int GetID()
            int GetType()
            string GetTypeName()

            _CSProperties* GetProperty()

            void SetPriority(int val)
            int GetPriority()
            bool Update(string *ErrStr)

            bool GetBoundBox(double dBoundBox[6])

            int GetDimension()

            bool IsInside(double* Coord, double tol)

            bool GetPrimitiveUsed()
            void SetPrimitiveUsed(bool val)

            void SetCoordinateSystem(CoordinateSystem cs_type)
            CoordinateSystem GetCoordinateSystem()

            _CSTransform* GetTransform()

cdef class CSPrimitives:
    cdef _CSPrimitives *thisptr
    cdef readonly CSTransform  __transform
    cdef readonly CSProperties __prop
    cdef __GetProperty(self)

###############################################################################
cdef extern from "CSXCAD/CSPrimPoint.h":
    cdef cppclass _CSPrimPoint "CSPrimPoint" (_CSPrimitives):
            _CSPrimPoint(_ParameterSet*, _CSProperties*) except +
            void   SetCoord(int idx, double val)
            double GetCoord(int idx)

cdef class CSPrimPoint(CSPrimitives):
    pass

###############################################################################
cdef extern from "CSXCAD/CSPrimBox.h":
    cdef cppclass _CSPrimBox "CSPrimBox" (_CSPrimitives):
            _CSPrimBox(_ParameterSet*, _CSProperties*) except +
            void   SetCoord(int idx, double val)
            double GetCoord(int idx)

cdef class CSPrimBox(CSPrimitives):
    pass

###############################################################################
cdef extern from "CSXCAD/CSPrimCylinder.h":
    cdef cppclass _CSPrimCylinder "CSPrimCylinder" (_CSPrimitives):
            _CSPrimCylinder(_ParameterSet*, _CSProperties*) except +
            void   SetCoord(int idx, double val)
            double GetCoord(int idx)
            void SetRadius(double val)
            double GetRadius()

cdef class CSPrimCylinder(CSPrimitives):
    pass

###############################################################################
cdef extern from "CSXCAD/CSPrimCylindricalShell.h":
    cdef cppclass _CSPrimCylindricalShell "CSPrimCylindricalShell" (_CSPrimCylinder):
            _CSPrimCylindricalShell(_ParameterSet*, _CSProperties*) except +
            void SetShellWidth(double val)
            double GetShellWidth()

cdef class CSPrimCylindricalShell(CSPrimCylinder):
    pass


###############################################################################
cdef extern from "CSXCAD/CSPrimSphere.h":
    cdef cppclass _CSPrimSphere "CSPrimSphere" (_CSPrimitives):
            _CSPrimSphere(_ParameterSet*, _CSProperties*) except +
            void SetCenter(double x1, double x2, double x3)
            double GetCoord(int idx)
            void SetRadius(double val)
            double GetRadius()

cdef class CSPrimSphere(CSPrimitives):
    pass

###############################################################################
cdef extern from "CSXCAD/CSPrimSphericalShell.h":
    cdef cppclass _CSPrimSphericalShell "CSPrimSphericalShell" (_CSPrimSphere):
            _CSPrimSphericalShell(_ParameterSet*, _CSProperties*) except +
            void SetShellWidth(double val)
            double GetShellWidth()

cdef class CSPrimSphericalShell(CSPrimSphere):
    pass

###############################################################################
cdef extern from "CSXCAD/CSPrimPolygon.h":
    cdef cppclass _CSPrimPolygon "CSPrimPolygon" (_CSPrimitives):
            _CSPrimPolygon(_ParameterSet*, _CSProperties*) except +
            void AddCoord(double val)
            double GetCoord(int index)
            void ClearCoords()
            size_t GetQtyCoords()
            void SetNormDir(int ny)
            int GetNormDir()
            void SetElevation(double val)
            double GetElevation()

cdef class CSPrimPolygon(CSPrimitives):
    pass

###############################################################################
cdef extern from "CSXCAD/CSPrimLinPoly.h":
    cdef cppclass _CSPrimLinPoly "CSPrimLinPoly" (_CSPrimPolygon):
            _CSPrimLinPoly(_ParameterSet*, _CSProperties*) except +
            void SetLength(double val)
            double GetLength()

cdef class CSPrimLinPoly(CSPrimPolygon):
    pass


###############################################################################
cdef extern from "CSXCAD/CSPrimRotPoly.h":
    cdef cppclass _CSPrimRotPoly "CSPrimRotPoly" (_CSPrimPolygon):
            _CSPrimRotPoly(_ParameterSet*, _CSProperties*) except +
            void SetRotAxisDir(int ny)
            int GetRotAxisDir()
            void SetAngle(int index, double val)
            double GetAngle(int index)

cdef class CSPrimRotPoly(CSPrimPolygon):
    pass


###############################################################################
cdef extern from "CSXCAD/CSPrimCurve.h":
    cdef cppclass _CSPrimCurve "CSPrimCurve" (_CSPrimitives):
            _CSPrimCurve(_ParameterSet*, _CSProperties*) except +
            size_t AddPoint(double *coords)
            size_t GetNumberOfPoints()
            bool GetPoint(size_t point_index, double point[3])
            void ClearPoints()

cdef class CSPrimCurve(CSPrimitives):
    pass

###############################################################################
cdef extern from "CSXCAD/CSPrimWire.h":
    cdef cppclass _CSPrimWire "CSPrimWire" (_CSPrimCurve):
            _CSPrimWire(_ParameterSet*, _CSProperties*) except +
            void SetWireRadius(double val)
            double GetWireRadius()

cdef class CSPrimWire(CSPrimCurve):
    pass

###############################################################################
cdef extern from "CSXCAD/CSPrimPolyhedron.h":
    cdef cppclass _CSPrimPolyhedron "CSPrimPolyhedron" (_CSPrimitives):
            _CSPrimPolyhedron(_ParameterSet*, _CSProperties*) except +
            void Reset()
            void AddVertex(float px, float py, float pz)
            float* GetVertex(unsigned int n)
            unsigned int GetNumVertices()
            void AddFace(int numVertex, int* vertices)
            int* GetFace(unsigned int n, unsigned int &numVertices)
            unsigned int GetNumFaces()

cdef class CSPrimPolyhedron(CSPrimitives):
    pass

###############################################################################
cdef extern from "CSXCAD/CSPrimPolyhedronReader.h":
    ctypedef enum FileType "CSPrimPolyhedronReader::FileType":
        UNKNOWN "CSPrimPolyhedronReader::UNKNOWN"
        STL_FILE "CSPrimPolyhedronReader::STL_FILE"
        PLY_FILE "CSPrimPolyhedronReader::PLY_FILE"
    cdef cppclass _CSPrimPolyhedronReader "CSPrimPolyhedronReader" (_CSPrimPolyhedron):
        _CSPrimPolyhedronReader(_ParameterSet*, _CSProperties*) except +
        void SetFilename(string name)
        string GetFilename()
        void SetFileType(FileType ft)
        FileType GetFileType()
        bool ReadFile()

cdef class CSPrimPolyhedronReader(CSPrimPolyhedron):
    pass
