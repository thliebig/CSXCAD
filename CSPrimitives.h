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
#include <vector>
#include "ParameterObjects.h"
#include "CSXCAD_Global.h"

class CSPrimBox;
class CSPrimMultiBox;
class CSPrimSphere;
	class CSPrimSphericalShell;
class CSPrimCylinder;
	class CSPrimCylindricalShell;
class CSPrimPolygon;
	class CSPrimLinPoly;
	class CSPrimRotPoly;
class CSPrimCurve;
	class CSPrimWire;
class CSPrimUserDefined;

class CSProperties; //include VisualProperties

class TiXmlNode;
class CSFunctionParser;

void Point_Line_Distance(double P[], double start[], double stop[], double &foot, double &dist);

//! Abstract base class for different geometrical primitives.
/*!
 This is an abstract base class for different geometrical primitives like boxes, spheres, cylinders etc.
 !!! Tolerances not yet obeyed !!!
 */
class CSXCAD_EXPORT CSPrimitives
{
public:
	virtual ~CSPrimitives();

	//! Primitive type definitions.
	enum PrimitiveType
	{
		BOX,MULTIBOX,SPHERE,SPHERICALSHELL,CYLINDER,CYLINDRICALSHELL,POLYGON,LINPOLY,ROTPOLY,CURVE,WIRE,USERDEFINED
	};

	//! Set or change the property for this primitive.
	void SetProperty(CSProperties *prop);
	//! Get the property for this primitive.
	CSProperties* GetProperty() {return clProperty;};

	//! Getthe unique ID for this primitive.
	unsigned int GetID() {return uiID;};
	//! Change the unique ID for this primitive. This is not recommended! Be sure what you are doing!
	void SetID(unsigned int ID) {uiID=ID;};

	//! Get the type of this primitive. \sa PrimitiveType
	int GetType() {return Type;};
	
	string GetTypeName() {return PrimTypeName;};

	//! Create a copy of ths primitive with different property.
	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimitives(this,prop);};

	//! Get the bounding box for this special primitive.
	virtual double* GetBoundBox(bool &accurate, bool PreserveOrientation=false) {UNUSED(PreserveOrientation);accurate=false;return dBoundBox;};

	//! Check if given Coordinate is inside the Primitive.
	virtual bool IsInside(double* Coord, double tol=0) {UNUSED(Coord);UNUSED(tol);return false;};

	//! Set or change the priotity for this primitive.
	void SetPriority(int val) {iPriority=val;};
	//! Get the priotity for this primitive.
	int GetPriority() {return iPriority;};

	//! Update this primitive with respect to the parameters set.
	virtual bool Update(string *ErrStr=NULL) {UNUSED(ErrStr);return true;};
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
	//! Get the corresponing Curve-Primitive or NULL in case of different type.
	CSPrimCurve* ToCurve() { return ( this && Type == CURVE ) ? (CSPrimCurve*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing UserDefined-Primitive or NULL in case of different type.
	CSPrimUserDefined* ToUserDefined() { return ( this && Type == USERDEFINED ) ? (CSPrimUserDefined*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.

	bool operator<(CSPrimitives& vgl) { return iPriority<vgl.GetPriority();};
	bool operator>(CSPrimitives& vgl) { return iPriority>vgl.GetPriority();};
	bool operator==(CSPrimitives& vgl) { return iPriority==vgl.GetPriority();};
	bool operator!=(CSPrimitives& vgl) { return iPriority!=vgl.GetPriority();};

protected:
	CSPrimitives(ParameterSet* paraSet, CSProperties* prop);
	CSPrimitives(CSPrimitives* prim, CSProperties *prop=NULL);
	CSPrimitives(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);

	unsigned int uiID;
	int iPriority;
	double dBoundBox[6];
	PrimitiveType Type;
	ParameterSet* clParaSet;
	CSProperties* clProperty;
	string PrimTypeName;
};

//! Box Primitive (Cube)
/*!
 This is a cube primitive defined by its start-, end-coordinates.
 */
class CSXCAD_EXPORT CSPrimBox : public CSPrimitives
{
public:
	CSPrimBox(ParameterSet* paraSet, CSProperties* prop);
	CSPrimBox(CSPrimBox* primBox, CSProperties *prop=NULL);
	CSPrimBox(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimBox();

	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimBox(this,prop);};

	void SetCoord(int index, double val) {if ((index>=0) && (index<6)) psCoords[index].SetValue(val);};
	void SetCoord(int index, const char* val) {if ((index>=0) && (index<6)) psCoords[index].SetValue(val);};

	double GetCoord(int index) {if ((index>=0) && (index<6)) return psCoords[index].GetValue(); else return 0;};
	ParameterScalar* GetCoordPS(int index) {if ((index>=0) && (index<6)) return &psCoords[index]; else return NULL;};

	virtual double* GetBoundBox(bool &accurate, bool PreserveOrientation=false);
	virtual bool IsInside(double* Coord, double tol=0);

	virtual bool Update(string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	ParameterScalar psCoords[6];
};

//! Multi-Box Primitive (Multi-Cube)
/*!
 This is a primitive defined by multiple cubes. Mostly used for already discretized objects.
 */
class CSXCAD_EXPORT CSPrimMultiBox : public CSPrimitives
{
public:
	CSPrimMultiBox(ParameterSet* paraSet, CSProperties* prop);
	CSPrimMultiBox(CSPrimMultiBox* multiBox,CSProperties *prop=NULL);
	CSPrimMultiBox(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimMultiBox();

	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimMultiBox(this,prop);};

	void SetCoord(int index, double val);
	void SetCoord(int index, const char* val);

	void AddCoord(double val);
	void AddCoord(const char* val);

	void AddBox(int initBox=-1);
	void DeleteBox(size_t box);

	double GetCoord(int index);
	ParameterScalar* GetCoordPS(int index);

	double* GetAllCoords(size_t &Qty, double* array);

	void ClearOverlap();

	virtual double* GetBoundBox(bool &accurate);
	virtual bool IsInside(double* Coord, double tol=0);

	unsigned int GetQtyBoxes() {return (unsigned int) vCoords.size()/6;};

	virtual bool Update(string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	vector<ParameterScalar> vCoords;
};

//! Sphere Primitive
/*!
 This is a spherical primitive defined by its center coordinates and a radius.
 */
class CSXCAD_EXPORT CSPrimSphere : public CSPrimitives
{
public:
	CSPrimSphere(ParameterSet* paraSet, CSProperties* prop);
	CSPrimSphere(CSPrimSphere* sphere, CSProperties *prop=NULL);
	CSPrimSphere(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimSphere();

	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimSphere(this,prop);};

	void SetCoord(int index, double val) {if ((index>=0) && (index<3)) psCenter[index].SetValue(val);};
	void SetCoord(int index, const char* val) {if ((index>=0) && (index<3)) psCenter[index].SetValue(val);};

	double GetCoord(int index) {if ((index>=0) && (index<3)) return psCenter[index].GetValue(); else return 0;};
	ParameterScalar* GetCoordPS(int index) {if ((index>=0) && (index<3)) return &psCenter[index]; else return NULL;};

	void SetRadius(double val) {psRadius.SetValue(val);};
	void SetRadius(const char* val) {psRadius.SetValue(val);};

	double GetRadius() {return psRadius.GetValue();};
	ParameterScalar* GetRadiusPS() {return &psRadius;};

	virtual double* GetBoundBox(bool &accurate);
	virtual bool IsInside(double* Coord, double tol=0);

	virtual bool Update(string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	ParameterScalar psCenter[3];
	ParameterScalar psRadius;
};

//! SphericalShell Primitive
/*!
 This is a spherical shell primitive derived from the sphere primitive, adding a shell width which is centered around the sphere radius.
 \sa CSPrimSphere
 */
class CSXCAD_EXPORT CSPrimSphericalShell : public CSPrimSphere
{
public:
	CSPrimSphericalShell(ParameterSet* paraSet, CSProperties* prop);
	CSPrimSphericalShell(CSPrimSphericalShell* sphereshell, CSProperties *prop=NULL);
	CSPrimSphericalShell(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimSphericalShell();

	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimSphericalShell(this,prop);};

	void SetShellWidth(double val) {psShellWidth.SetValue(val);};
	void SetShellWidth(const char* val) {psShellWidth.SetValue(val);};

	double GetShellWidth() {return psShellWidth.GetValue();};
	ParameterScalar* GetShellWidthPS() {return &psShellWidth;};

	virtual double* GetBoundBox(bool &accurate);
	virtual bool IsInside(double* Coord, double tol=0);

	virtual bool Update(string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	ParameterScalar psShellWidth;
};

//! Cylinder Primitive
/*!
 This is a cylindrical primitive defined by its axis start-, end-coordinates and a radius.
 */
class CSXCAD_EXPORT CSPrimCylinder : public CSPrimitives
{
public:
	CSPrimCylinder(ParameterSet* paraSet, CSProperties* prop);
	CSPrimCylinder(CSPrimCylinder* cylinder, CSProperties *prop=NULL);
	CSPrimCylinder(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimCylinder();

	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimCylinder(this,prop);};

	void SetCoord(int index, double val) {if ((index>=0) && (index<6)) psCoords[index].SetValue(val);};
	void SetCoord(int index, const char* val) {if ((index>=0) && (index<6)) psCoords[index].SetValue(val);};

	double GetCoord(int index) {if ((index>=0) && (index<6)) return psCoords[index].GetValue(); else return 0;};
	ParameterScalar* GetCoordPS(int index) {if ((index>=0) && (index<6)) return &psCoords[index]; else return NULL;};

	void SetRadius(double val) {psRadius.SetValue(val);};
	void SetRadius(const char* val) {psRadius.SetValue(val);};

	double GetRadius() {return psRadius.GetValue();};
	ParameterScalar* GetRadiusPS() {return &psRadius;};

	virtual double* GetBoundBox(bool &accurate);
	virtual bool IsInside(double* Coord, double tol=0);

	virtual bool Update(string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	ParameterScalar psCoords[6];
	ParameterScalar psRadius;
};

//! CylindicalShell Primitive
/*!
 This is a cylindrical shell primitive derived from the cylinder primitive, adding a shell width which is centered around the cylinder radius.
 \sa CSPrimCylinder
 */
class CSXCAD_EXPORT CSPrimCylindricalShell : public CSPrimCylinder
{
public:
	CSPrimCylindricalShell(ParameterSet* paraSet, CSProperties* prop);
	CSPrimCylindricalShell(CSPrimCylindricalShell* cylinder, CSProperties *prop=NULL);
	CSPrimCylindricalShell(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimCylindricalShell();

	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimCylindricalShell(this,prop);};

	void SetShellWidth(double val) {psShellWidth.SetValue(val);};
	void SetShellWidth(const char* val) {psShellWidth.SetValue(val);};

	double GetShellWidth() {return psShellWidth.GetValue();};
	ParameterScalar* GetShellWidthPS() {return &psShellWidth;};

	virtual double* GetBoundBox(bool &accurate);
	virtual bool IsInside(double* Coord, double tol=0);

	virtual bool Update(string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	ParameterScalar psShellWidth;
};

//! 2D Polygon Primitive
/*!
 This is an area polygon primitive defined by a number of points in space.
 */
class CSXCAD_EXPORT CSPrimPolygon : public CSPrimitives
{
public:
	CSPrimPolygon(ParameterSet* paraSet, CSProperties* prop);
	CSPrimPolygon(CSPrimPolygon* primPolygon, CSProperties *prop=NULL);
	CSPrimPolygon(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimPolygon();

	virtual CSPrimPolygon* GetCopy(CSProperties *prop=NULL) {return new CSPrimPolygon(this,prop);};

	void SetCoord(int index, double val);
	void SetCoord(int index, const string val);

	void AddCoord(double val);
	void AddCoord(const string val);
	
	void RemoveCoords(int index);
	void ClearCoords() {vCoords.clear();};

	double GetCoord(int index);
	ParameterScalar* GetCoordPS(int index);

	size_t GetQtyCoords() {return vCoords.size()/2;};
	double* GetAllCoords(size_t &Qty, double* array);
	
	void SetNormDir(int index, double val) {if ((index>=0) && (index<3)) NormDir[index].SetValue(val);};
	void SetNormDir(int index, const char* val) {if ((index>=0) && (index<3)) NormDir[index].SetValue(val);};

	double GetNormDir(int index) {if ((index>=0) && (index<3)) return NormDir[index].GetValue(); else return 0;};
	ParameterScalar* GetNormDirPS(int index) {if ((index>=0) && (index<3)) return &NormDir[index]; else return NULL;};

	void SetElevation(double val) {Elevation.SetValue(val);};
	void SetElevation(const char* val) {Elevation.SetValue(val);};

	double GetElevation() {return Elevation.GetValue();};
	ParameterScalar* GetElevationPS() {return &Elevation;};

	virtual double* GetBoundBox(bool &accurate);
	virtual bool IsInside(double* Coord, double tol=0);

	virtual bool Update(string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	///Vector describing the polygon, x1,y1,x2,y2 ... xn,yn
	vector<ParameterScalar> vCoords;
	///The polygon plane normal vector (only cartesian so far)
	ParameterScalar NormDir[3];
	///The polygon plane elevation in direction of the normal vector
	ParameterScalar Elevation;
};

//! Linear extruded polygon Primitive
/*!
 This is a linear extruded area polygon primitive defined by a number of points in space and an extrude vector.
 */
class CSXCAD_EXPORT CSPrimLinPoly : public CSPrimPolygon
{
public:
	CSPrimLinPoly(ParameterSet* paraSet, CSProperties* prop);
	CSPrimLinPoly(CSPrimLinPoly* primPolygon, CSProperties *prop=NULL);
	CSPrimLinPoly(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimLinPoly();

	virtual CSPrimLinPoly* GetCopy(CSProperties *prop=NULL) {return new CSPrimLinPoly(this,prop);};

	void SetLength(double val) {extrudeLength.SetValue(val);};
	void SetLength(const string val) {extrudeLength.SetValue(val);};

	double GetLength() {return extrudeLength.GetValue();};
	ParameterScalar* GetLengthPS() {return &extrudeLength;};

	virtual double* GetBoundBox(bool &accurate);
	virtual bool IsInside(double* Coord, double tol=0);

	virtual bool Update(string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	ParameterScalar extrudeLength;
};

//! Rotational extruded polygon Primitive
/*!
 This is a rotation extruded area polygon primitive defined by a number of points in space, an extrude axis and start-, stop-angle.
 */
class CSXCAD_EXPORT CSPrimRotPoly : public CSPrimPolygon
{
public:
	CSPrimRotPoly(ParameterSet* paraSet, CSProperties* prop);
	CSPrimRotPoly(CSPrimRotPoly* primPolygon, CSProperties *prop=NULL);
	CSPrimRotPoly(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimRotPoly();

	virtual CSPrimRotPoly* GetCopy(CSProperties *prop=NULL) {return new CSPrimRotPoly(this,prop);};

	void SetRotAxis(int index, double val) {if ((index>=0) && (index<3)) RotAxis[index].SetValue(val);};
	void SetRotAxis(int index, const string val) {if ((index>=0) && (index<3)) RotAxis[index].SetValue(val);};

	double GetRotAxis(int index) {if ((index>=0) && (index<3)) return RotAxis[index].GetValue(); else return 0;};
	ParameterScalar* GetRotAxisPS(int index) {if ((index>=0) && (index<3)) return &RotAxis[index]; else return NULL;};

	void SetAngle(int index, double val) {if ((index>=0) && (index<2)) StartStopAngle[index].SetValue(val);};
	void SetAngle(int index, const string val) {if ((index>=0) && (index<2)) StartStopAngle[index].SetValue(val);};

	double GetAngle(int index) {if ((index>=0) && (index<2)) return StartStopAngle[index].GetValue(); else return 0;};
	ParameterScalar* GetAnglePS(int index) {if ((index>=0) && (index<2)) return &StartStopAngle[index]; else return NULL;};

	virtual double* GetBoundBox(bool &accurate);
	virtual bool IsInside(double* Coord, double tol=0);

	virtual bool Update(string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	//start-stop angle
	ParameterScalar StartStopAngle[2];
	//rot axis
	ParameterScalar RotAxis[3];
};

//! Curve Primitive (Polygonal chain)
/*!
 This is a curve primitive defined by a number of 3D points
 */
class CSXCAD_EXPORT CSPrimCurve : public CSPrimitives
{
public:
	CSPrimCurve(ParameterSet* paraSet, CSProperties* prop);
	CSPrimCurve(CSPrimCurve* primCurve, CSProperties *prop=NULL);
	CSPrimCurve(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimCurve();

	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimCurve(this,prop);};

	virtual size_t AddPoint(double coords[]);
	virtual void SetCoord(size_t point_index, int nu, double val);
	virtual void SetCoord(size_t point_index, int nu, string val);

	virtual size_t GetNumberOfPoints() {return points[0].size();}
	virtual bool GetPoint(size_t point_index, double* point);

	virtual double* GetBoundBox(bool &accurate, bool PreserveOrientation=false);
	virtual bool IsInside(double* Coord, double tol=0);

	virtual bool Update(string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	vector<ParameterScalar> points[3];
};

//! Wire Primitive (Polygonal chain with finite radius)
/*!
 This is a wire primitive derived from a curve with an additional wire radius
 \sa CSPrimCurve
 */
class CSXCAD_EXPORT CSPrimWire : public CSPrimCurve
{
public:
	CSPrimWire(ParameterSet* paraSet, CSProperties* prop);
	CSPrimWire(CSPrimWire* primCurve, CSProperties *prop=NULL);
	CSPrimWire(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimWire();

	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimWire(this,prop);};

	void SetWireRadius(double val) {wireRadius.SetValue(val);};
	void SetWireRadius(const char* val) {wireRadius.SetValue(val);};

	double GetWireRadius() {return wireRadius.GetValue();};
	ParameterScalar* GetWireRadiusPS() {return &wireRadius;};

	virtual double* GetBoundBox(bool &accurate, bool PreserveOrientation=false);
	virtual bool IsInside(double* Coord, double tol=0);

	virtual bool Update(string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	ParameterScalar wireRadius;
};

//! User defined Primitive given by an analytic formula
/*!
 This primitive is defined by a boolean result analytic formula. If a given coordinate results in a true result the primitive is assumed existing at these coordinate.
 */
class CSXCAD_EXPORT CSPrimUserDefined: public CSPrimitives
{
public:
	enum UserDefinedCoordSystem
	{
		CARESIAN_SYSTEM,CYLINDER_SYSTEM,SPHERE_SYSTEM
	};
	CSPrimUserDefined(ParameterSet* paraSet, CSProperties* prop);
	CSPrimUserDefined(CSPrimUserDefined* primUDef, CSProperties *prop=NULL);
	CSPrimUserDefined(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimUserDefined();

	virtual CSPrimUserDefined* GetCopy(CSProperties *prop=NULL) {return new CSPrimUserDefined(this,prop);};

	void SetCoordSystem(UserDefinedCoordSystem newSystem);
	UserDefinedCoordSystem GetCoordSystem() {return CoordSystem;};

	void SetCoordShift(int index, double val) {if ((index>=0) && (index<3)) dPosShift[index].SetValue(val);};
	void SetCoordShift(int index, const char* val) {if ((index>=0) && (index<3)) dPosShift[index].SetValue(val);};

	double GetCoordShift(int index) {if ((index>=0) && (index<3)) return dPosShift[index].GetValue(); else return 0;};;
	ParameterScalar* GetCoordShiftPS(int index) {if ((index>=0) && (index<3)) return &dPosShift[index]; else return NULL;};

	void SetFunction(const char* func);
	const char* GetFunction() {return stFunction.c_str();};

	virtual double* GetBoundBox(bool &accurate);
	virtual bool IsInside(double* Coord, double tol=0);

	virtual bool Update(string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	string stFunction;
	UserDefinedCoordSystem CoordSystem;
	CSFunctionParser* fParse;
	string fParameter;
	int iQtyParameter;
	ParameterScalar dPosShift[3];
};
