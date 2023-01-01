/*
*	Copyright (C) 2008,2009,2010 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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
*/

#pragma once
/*
 * Author:	Thorsten Liebig
 * Date:	03-12-2008
 * Lib:		CSXCAD
 * Version:	0.1a
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "ParameterObjects.h"
#include "ParameterCoord.h"
#include "CSXCAD_Global.h"

class CSPrimPoint;
class CSPrimBox;
class CSPrimMultiBox;
class CSPrimSphere;
	class CSPrimSphericalShell;
class CSPrimCylinder;
	class CSPrimCylindricalShell;
class CSPrimPolygon;
	class CSPrimLinPoly;
	class CSPrimRotPoly;
class CSPrimPolyhedron;
	class CSPrimPolyhedronReader;
class CSPrimCurve;
	class CSPrimWire;
class CSPrimUserDefined;

class CSProperties; //include VisualProperties

class TiXmlNode;
class CSFunctionParser;
class CSTransform;

/*!
	Calculate the distance of a point to a line (defined by start/stop coordinates).
	Foot will return the normalized foot-point on the line. A value between 0..1 means the foot-point is one the given line.
	foot == 0 --> foot-point is on start, foot == 1 --> foot-point is on stop
*/
void CSXCAD_EXPORT Point_Line_Distance(const double P[], const double start[], const double stop[], double &foot, double &dist, CoordinateSystem c_system=UNDEFINED_CS);

bool CSXCAD_EXPORT CoordInRange(const double* p, const double* start, const double* stop, CoordinateSystem cs_in);

//! Abstract base class for different geometrical primitives.
/*!
 This is an abstract base class for different geometrical primitives like boxes, spheres, cylinders etc.
 !!! Tolerances not yet obeyed !!!
 */
class CSXCAD_EXPORT CSPrimitives
{
public:
	virtual ~CSPrimitives();

	virtual void Init();

	//! Primitive type definitions.
	enum PrimitiveType
	{
		POINT,BOX,MULTIBOX,SPHERE,SPHERICALSHELL,CYLINDER,CYLINDRICALSHELL,POLYGON,LINPOLY,ROTPOLY,POLYHEDRON,CURVE,WIRE,USERDEFINED,
		POLYHEDRONREADER
	};

	//! Set or change the property for this primitive.
	void SetProperty(CSProperties *prop);
	//! Get the property for this primitive.
	CSProperties* GetProperty() {return clProperty;}

	//! Getthe unique ID for this primitive.
	unsigned int GetID() {return uiID;}
	//! Change the unique ID for this primitive. This is not recommended! Be sure what you are doing!
	void SetID(unsigned int ID) {uiID=ID;}

	//! Get the type of this primitive. \sa PrimitiveType
	int GetType() {return Type;}
	
	std::string GetTypeName() {return PrimTypeName;}

	//! Create a copy of ths primitive with different property.
	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimitives(this,prop);}

	//! Get the bounding box (for the given mesh type) for this special primitive. \sa GetBoundBoxCoordSystem
	virtual bool GetBoundBox(double dBoundBox[6], bool PreserveOrientation=false) {UNUSED(PreserveOrientation);UNUSED(dBoundBox);return false;}

	virtual CoordinateSystem GetBoundBoxCoordSystem() const {return m_BoundBox_CoordSys;}

	//! Get the dimension of this primitive
	virtual int GetDimension();

	//! Check if given Coordinate (in the given mesh type) is inside the Primitive.
	virtual bool IsInside(const double* Coord, double tol=0) {UNUSED(Coord);UNUSED(tol);return false;}

	//! Check if the primitive is inside a given box (box must be specified in the bounding box coordinate system)
	//! @return -1 if not, +1 if it is, 0 if unknown
	virtual int IsInsideBox(const double*  boundbox);

	//! Check whether this primitive was used. (--> IsInside() return true) \sa SetPrimitiveUsed
	bool GetPrimitiveUsed() {return m_Primtive_Used;}
	//! Set the primitve uses flag. \sa GetPrimitiveUsed
	void SetPrimitiveUsed(bool val) {m_Primtive_Used=val;}

	//! Set or change the priotity for this primitive.
	void SetPriority(int val) {iPriority=val;}
	//! Get the priotity for this primitive.
	int GetPriority() {return iPriority;}

	//! Update this primitive with respect to the parameters set.
	virtual bool Update(std::string *ErrStr=NULL) {UNUSED(ErrStr);return true;}
	//! Write this primitive to a XML-node.
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	//! Read this primitive from a XML-node.
	virtual bool ReadFromXML(TiXmlNode &root);

	//! Get the corresponing Box-Primitive or NULL in case of different type.
	CSPrimBox* ToBox() { return ( this && Type == BOX ) ? (CSPrimBox*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing MultiBox-Primitive or NULL in case of different type.
	CSPrimMultiBox* ToMultiBox() { return ( this && Type == MULTIBOX ) ? (CSPrimMultiBox*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing Sphere-Primitive or NULL in case of different type.
	CSPrimSphere* ToSphere() { return ( this && Type == SPHERE ) ? (CSPrimSphere*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing SphereicalShell-Primitive or NULL in case of different type.
	CSPrimSphericalShell* ToSphericalShell() { return ( this && Type == SPHERICALSHELL ) ? (CSPrimSphericalShell*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing Cylinder-Primitive or NULL in case of different type.
	CSPrimCylinder* ToCylinder() { return ( this && Type == CYLINDER ) ? (CSPrimCylinder*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing CylindricalShell-Primitive or NULL in case of different type.
	CSPrimCylindricalShell* ToCylindricalShell() { return ( this && Type == CYLINDRICALSHELL ) ? (CSPrimCylindricalShell*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing Polygon-Primitive or NULL in case of different type.
	CSPrimPolygon* ToPolygon() { return ( this && Type == POLYGON ) ? (CSPrimPolygon*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing LinPoly-Primitive or NULL in case of different type.
	CSPrimLinPoly* ToLinPoly() { return ( this && Type == LINPOLY ) ? (CSPrimLinPoly*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing Cylinder-Primitive or NULL in case of different type.
	CSPrimRotPoly* ToRotPoly() { return ( this && Type == ROTPOLY ) ? (CSPrimRotPoly*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing Polyhedron-Primitive or NULL in case of different type.
	CSPrimPolyhedron* ToPolyhedron() { return ( this && Type == POLYHEDRON ) ? (CSPrimPolyhedron*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing Polyhedron-Import-Primitive or NULL in case of different type.
	CSPrimPolyhedronReader* ToPolyhedronReader() { return ( this && Type == POLYHEDRONREADER ) ? (CSPrimPolyhedronReader*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing Curve-Primitive or NULL in case of different type.
	CSPrimCurve* ToCurve() { return ( this && Type == CURVE ) ? (CSPrimCurve*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing Wire-Primitive or NULL in case of different type.
	CSPrimWire* ToWire() { return ( this && Type == WIRE ) ? (CSPrimWire*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing UserDefined-Primitive or NULL in case of different type.
	CSPrimUserDefined* ToUserDefined() { return ( this && Type == USERDEFINED ) ? (CSPrimUserDefined*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing Point-Primitive or 0 in case of different type.
	CSPrimPoint* ToPoint() { return ( this && Type == POINT ) ? (CSPrimPoint*) this : 0; } //!< Cast Primitive to a more defined type. Will return 0 if not of the requested type.

	bool operator<(CSPrimitives& vgl) { return iPriority<vgl.GetPriority();}
	bool operator>(CSPrimitives& vgl) { return iPriority>vgl.GetPriority();}
	bool operator==(CSPrimitives& vgl) { return iPriority==vgl.GetPriority();}
	bool operator!=(CSPrimitives& vgl) { return iPriority!=vgl.GetPriority();}

	//! Define the input type for the weighting coordinate system 0=cartesian, 1=cylindrical, 2=spherical
	void SetCoordInputType(CoordinateSystem type, bool doUpdate=true) {m_MeshType=type; if (doUpdate) Update();}
	//! Get the input type for the weighting coordinate system 0=cartesian, 1=cylindrical, 2=spherical
	CoordinateSystem GetCoordInputType() const {return m_MeshType;}

	//! Define the coordinate system this primitive is defined in (may be different to the input mesh type) \sa SetCoordInputType
	void SetCoordinateSystem(CoordinateSystem cs) {m_PrimCoordSystem=cs;}
	//! Read the coordinate system for this primitive (may be different to the input mesh type) \sa GetCoordInputType
	CoordinateSystem GetCoordinateSystem() const {return m_PrimCoordSystem;}

	//! Get the CSTransform if it exists already or create a new one
	CSTransform* GetTransform();

	//! Show status of this primitve
	virtual void ShowPrimitiveStatus(std::ostream& stream);

protected:
	CSPrimitives(ParameterSet* paraSet, CSProperties* prop);
	CSPrimitives(CSPrimitives* prim, CSProperties *prop=NULL);
	CSPrimitives(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);

	//! Invalidate some cached data, e.g. object dimension and bounding box, should be called when data is modified
	virtual void Invalidate();

	//! Apply (invers) transformation to the given coordinate in the given coordinate system
	void TransformCoords(double* Coord, bool invers, CoordinateSystem cs_in) const;

	unsigned int uiID;
	int iPriority;
	CoordinateSystem m_PrimCoordSystem;
	CoordinateSystem m_MeshType;
	PrimitiveType Type;
	ParameterSet* clParaSet;
	CSProperties* clProperty;
	CSTransform* m_Transform;
	std::string PrimTypeName;
	bool m_Primtive_Used;

	//internal bounding box, updated by Update(), can be used to speedup IsInside
	bool m_BoundBoxValid;
	double m_BoundBox[6];
	CoordinateSystem m_BoundBox_CoordSys;

	int m_Dimension;
};


