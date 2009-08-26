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
class CSPrimCylinder;
class CSPrimPolygon;
	class CSPrimLinPoly;
	class CSPrimRotPoly;
class CSPrimUserDefined;

class CSProperties; //include VisualProperties

class TiXmlNode;
class FunctionParser;

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
		BOX,MULTIBOX,SPHERE,CYLINDER,POLYGON,LINPOLY,ROTPOLY,USERDEFINED
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

	//! Create a copy of ths primitive with different property.
	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimitives(this,prop);};

	//! Get the bounding box for this special primitive.
	virtual double* GetBoundBox(bool &accurate) {return dBoundBox;accurate=false;};

	//! Check if given Coordinate is inside the Primitive.
	virtual bool IsInside(double* Coord, double tol=0) {return false;};

	//! Set or change the priotity for this primitive.
	void SetPriority(int val) {iPriority=val;};
	//! Get the priotity for this primitive.
	int GetPriority() {return iPriority;};

	//! Update this primitive with respect to the parameters set.
	virtual bool Update(string *ErrStr=NULL) {return true;};
	//! Write this primitive to a XML-node.
	virtual bool Write2XML(TiXmlNode &root, bool parameterised=true);
	//! Read this primitive to a XML-node.
	virtual bool ReadFromXML(TiXmlNode &root);

	//! Get the corresponing Box-Primitive or NULL in case of different type.
	CSPrimBox* ToBox() { return ( this && Type == BOX ) ? (CSPrimBox*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing MultiBox-Primitive or NULL in case of different type.
	CSPrimMultiBox* ToMultiBox() { return ( this && Type == MULTIBOX ) ? (CSPrimMultiBox*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing Sphere-Primitive or NULL in case of different type.
	CSPrimSphere* ToSphere() { return ( this && Type == SPHERE ) ? (CSPrimSphere*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing Cylinder-Primitive or NULL in case of different type.
	CSPrimCylinder* ToCylinder() { return ( this && Type == CYLINDER ) ? (CSPrimCylinder*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing Polygon-Primitive or NULL in case of different type.
	CSPrimPolygon* ToPolygon() { return ( this && Type == POLYGON ) ? (CSPrimPolygon*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing LinPoly-Primitive or NULL in case of different type.
	CSPrimLinPoly* ToLinPoly() { return ( this && Type == LINPOLY ) ? (CSPrimLinPoly*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
	//! Get the corresponing Cylinder-Primitive or NULL in case of different type.
	CSPrimRotPoly* ToRotPoly() { return ( this && Type == ROTPOLY ) ? (CSPrimRotPoly*) this : 0; } /// Cast Primitive to a more defined type. Will return null if not of the requested type.
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
};

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

	virtual double* GetBoundBox(bool &accurate);
	virtual bool IsInside(double* Coord, double tol=0);

	virtual bool Update(string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlNode &root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	ParameterScalar psCoords[6];
};

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
	virtual bool Write2XML(TiXmlNode &root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	vector<ParameterScalar> vCoords;
};

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
	virtual bool Write2XML(TiXmlNode &root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	ParameterScalar psCenter[3];
	ParameterScalar psRadius;
};

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
	virtual bool Write2XML(TiXmlNode &root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	ParameterScalar psCoords[6];
	ParameterScalar psRadius;
};

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

	double GetCoord(int index);
	ParameterScalar* GetCoordPS(int index);

	size_t GetQtyCoords() {return vCoords.size()/2;};
	double* GetAllCoords(size_t &Qty, double* array);

	virtual double* GetBoundBox(bool &accurate);
	virtual bool IsInside(double* Coord, double tol=0);

	virtual bool Update(string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlNode &root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	///Vector describing the polygon, x1,y1,x2,y2 ... xn,yn
	vector<ParameterScalar> vCoords;
	///The polygon plane normal vector (only cartesian so far)
	ParameterScalar NormDir[3];
	///The polygon plane elevation in direction of the normal vector
	ParameterScalar Elevation;
};

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

	double GetLength() {extrudeLength.GetValue();};
	ParameterScalar* GetLengthPS() {return &extrudeLength;};

	virtual double* GetBoundBox(bool &accurate);
	virtual bool IsInside(double* Coord, double tol=0);

	virtual bool Update(string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlNode &root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	ParameterScalar extrudeLength;
};

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
	virtual bool Write2XML(TiXmlNode &root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	//start-stop angle
	ParameterScalar StartStopAngle[2];
	//rot axis
	ParameterScalar RotAxis[3];
};

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
	virtual bool Write2XML(TiXmlNode &root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	string stFunction;
	UserDefinedCoordSystem CoordSystem;
	FunctionParser* fParse;
	string fParameter;
	int iQtyParameter;
	ParameterScalar dPosShift[3];
};
