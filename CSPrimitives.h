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
		BOX,MULTIBOX,SPHERE,CYLINDER,USERDEFINED
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

	double* GetAllCoords(int &Qty, double* array);

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
