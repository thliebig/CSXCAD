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

using namespace std;

class CSPrimitives;

class CSPropUnknown;
class CSPropMaterial;
class CSPropMetal;
class CSPropElectrode;
class CSPropChargeBox;
class CSPropResBox;
class CSPropDumpBox;

class TiXmlNode;

typedef struct
{
	unsigned char R,G,B,a;
} RGBa;

//! Continuous Structure (CS)Properties (layer)
/*!
 CSProperties is a class that contains geometrical primitive structures as boxes, spheres, cylinders etc. (CSPrimitives)
 All values in this class can contain parameters and mathematical equations.
 This absract base-class contains basic property methodes, e.g. set/get primitives, mesh-relations, color-information etc.
*/
class CSXCAD_EXPORT CSProperties
{
public:
	virtual ~CSProperties();
	//! Copy constructor
	CSProperties(CSProperties* prop);
	//! Enumeration of all possible sub-types of this base-class
	enum PropertyType
	{
		ANY,UNKNOWN,MATERIAL,METAL,ELECTRODE,CHARGEBOX,RESBOX,DUMPBOX
	};
	
	//! Get PropertyType \sa PropertyType and GetTypeString
	int GetType();
	//! Get PropertyType as a string \sa PropertyType and GetType
	const string GetTypeString();
	//! Check if Property is a physical material. Current PropertyType: MATERIAL & METAL
	bool GetMaterial() {return bMaterial;};
	//!Get ID of this property. Used for primitive-->property mapping. \sa SetID	
	unsigned int GetID();
	//!Set ID to this property. USE ONLY WHEN YOU KNOW WHAT YOU ARE DOING!!!! \sa GetID
	void SetID(unsigned int ID);

	//!Get unique ID of this property. Used internally. \sa SetUniqueID	
	unsigned int GetUniqueID();
	//!Set unique ID of this property. Used internally. USE ONLY WHEN YOU KNOW WHAT YOU ARE DOING!!!! \sa GetUniqueID	
	void SetUniqueID(unsigned int uID);

	//! Set Name for this Property. \sa GetName
	void SetName(const string name);
	//! Get Name for this Property. \sa SetName
	const string GetName();

	//! Add a primitive to this Propertie. Takes ownership of this primitive! \sa CSPrimitives, RemovePrimitive, TakePrimitive
	void AddPrimitive(CSPrimitives *prim);
	//! Removes and deletes a primitive of this Property. \sa CSPrimitives, AddPrimitive, TakePrimitive
	void RemovePrimitive(CSPrimitives *prim);
	//! Take a primitive of this Propertie at index. Releases ownership of this primitive to caller! \sa CSPrimitives, RemovePrimitive, AddPrimitive \return NULL if not found!
	CSPrimitives* TakePrimitive(size_t index);

	//! Check whether the given coord is inside of a primitive assigned to this property and return found priority.
	bool CheckCoordInPrimitive(double *coord, int &priority, double tol=0);

	//! Get the quentity of primitives assigned to this property! \return Number of primitives in this property
	size_t GetQtyPrimitives();
	//! Get the Primitive at certain index position. \sa GetQtyPrimitives
	CSPrimitives* GetPrimitive(size_t index);
	
	//! Set a fill-color for this property. \sa GetFillColor
	void SetFillColor(RGBa color);
	//! Get a fill-color for this property. \sa SetFillColor \return RGBa color object.
	RGBa GetFillColor();

	//void SetEdgeColor(int val) {iEdgeColor=val;};
	//! Set a edge-color for this property. \sa SetEdgeColor
	void SetEdgeColor(RGBa color);
	//! Get a fill-color for this property. \sa GetEdgeColor \return RGBa color object.
	RGBa GetEdgeColor();

	//! Get visibility for this property. \sa SetVisibility
	bool GetVisibility();
	//! Set visibility for this property. \sa GetVisibility
	void SetVisibility(bool val);

	//! Convert to Unknown Property, returns NULL if type is different! \return Returns a CSPropUnknown* or NULL if type is different!
	CSPropUnknown* ToUnknown();
	//! Convert to Material Property, returns NULL if type is different! \return Returns a ToMaterial* or NULL if type is different!
	CSPropMaterial* ToMaterial();
	//! Convert to Metal Property, returns NULL if type is different! \return Returns a ToMetal* or NULL if type is different!
	CSPropMetal* ToMetal();
	//! Convert to Electrode Property, returns NULL if type is different! \return Returns a ToElectrode* or NULL if type is different!
	CSPropElectrode* ToElectrode();
	//! Convert to ChargeBox Property, returns NULL if type is different! \return Returns a ToChargeBox* or NULL if type is different!
	CSPropChargeBox* ToChargeBox();
	//! Convert to ResBox Property, returns NULL if type is different! \return Returns a ToResBox* or NULL if type is different!
	CSPropResBox* ToResBox();
	//! Convert to DumpBox Property, returns NULL if type is different! \return Returns a ToDumpBox* or NULL if type is different!
	CSPropDumpBox* ToDumpBox();

	//! Update all parameters. Nothing to do in this base class. \param ErrStr Methode writes error messages to this string! \return Update success
	virtual bool Update(string *ErrStr=NULL);

	//! Write this property to a xml-node. \param parameterised Use false if parameters should be written as values. Parameters are lost!
	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true);
	//! Read property from xml-node. \return Successful read-operation. 
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	CSProperties(ParameterSet* paraSet);
	CSProperties(unsigned int ID, ParameterSet* paraSet);
	ParameterSet* clParaSet;
	PropertyType Type;
	bool bMaterial;
	unsigned int uiID;
	unsigned int UniqueID;
	string sName;
	string sType;
	RGBa FillColor;
	RGBa EdgeColor;

	bool bVisisble;

	vector<CSPrimitives*> vPrimitives;
};

class CSXCAD_EXPORT CSPropUnknown : public CSProperties
{
public:
	CSPropUnknown(ParameterSet* paraSet);
	CSPropUnknown(unsigned int ID, ParameterSet* paraSet);
	CSPropUnknown(CSProperties* prop);
	virtual ~CSPropUnknown();

	void SetProperty(const string val);
	const string GetProperty();

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	string sUnknownProperty;
};

class CSXCAD_EXPORT CSPropMaterial : public CSProperties
{
public:
	CSPropMaterial(ParameterSet* paraSet);
	CSPropMaterial(CSProperties* prop);
	CSPropMaterial(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropMaterial();

        void SetEpsilon(double val, int direction=0);
        void SetEpsilon(const string val, int direction=0);
        double GetEpsilon(int direction=0);
        const string GetEpsilonTerm(int direction=0);

        void SetMue(double val, int direction=0);
        void SetMue(const string val, int direction=0);
        double GetMue(int direction=0);
        const string GetMueTerm(int direction=0);

        void SetKappa(double val, int direction=0);
        void SetKappa(const string val, int direction=0);
        double GetKappa(int direction=0);
        const string GetKappaTerm(int direction=0);

        void SetSigma(double val, int direction=0);
        void SetSigma(const string val, int direction=0);
        double GetSigma(int direction=0);
        const string GetSigmaTerm(int direction=0);

        void SetIsotropy(bool val) {bIsotropy=val;};
        bool GetIsotropy() {return bIsotropy;};

        virtual void Init();
	virtual bool Update(string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
        ParameterScalar Epsilon[3],Mue[3],Kappa[3],Sigma[3];
        bool bIsotropy;
};

class CSXCAD_EXPORT CSPropMetal : public CSProperties
{
public:
	CSPropMetal(ParameterSet* paraSet);
	CSPropMetal(CSProperties* prop);
	CSPropMetal(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropMetal();

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);
};

class CSXCAD_EXPORT CSPropElectrode : public CSProperties
{
public:
	CSPropElectrode(ParameterSet* paraSet,unsigned int number=0);
	CSPropElectrode(CSProperties* prop);
	CSPropElectrode(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropElectrode();

	void SetNumber(unsigned int val);
	unsigned int GetNumber();

	void SetExcitType(int val);
	int GetExcitType();

	//! Set the active direction for the source, only necessary with hard sources (by default all active)
	void SetActiveDir(bool active, int Component=0);
	//! Get the active direction for the source, only necessary with hard sources (by default all active)
	bool GetActiveDir(int Component=0);

	void SetExcitation(double val, int Component=0);
	void SetExcitation(const string val, int Component=0);
	double GetExcitation(int Component=0);
	const string GetExcitationString(int Comp=0);

	void SetWeightFct(const string fct, int ny);
	const string GetWeightFct(int ny);

	double GetWeightedExcitation(int ny, double* coords);

	void SetDelay(double val);
	void SetDelay(const string val);
	double GetDelay();
	const string GetDelayString();

//	void SetWeightVars(const char* vars,int ny) {if ((ny>=0) && (ny<3)) sWeightVars[ny]=string(vars);};
//	const char* GetWeightVars(int ny) {if ((ny>=0) && (ny<3)) {if (sWeightVars[ny].empty()) return NULL; else return sWeightVars[ny].c_str();} else return NULL;};

	virtual void Init();
	virtual bool Update(string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	string sWeightFct[3];
//	string sWeightVars[3];
	unsigned int uiNumber;
	int iExcitType;
	bool ActiveDir[3];
	ParameterScalar Excitation[3];
	ParameterScalar Delay;//only for transient solver
};


class CSXCAD_EXPORT CSPropChargeBox : public CSProperties
{
public:
	CSPropChargeBox(ParameterSet* paraSet);
	CSPropChargeBox(CSProperties* prop);
	CSPropChargeBox(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropChargeBox();

	void SetNumber(unsigned int val);
	unsigned int GetNumber();

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);
protected:
	unsigned int uiNumber;
};

class CSXCAD_EXPORT CSPropResBox : public CSProperties
{
public:
	CSPropResBox(ParameterSet* paraSet);
	CSPropResBox(CSProperties* prop);
	CSPropResBox(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropResBox();

	void SetResFactor(unsigned int val);
	unsigned int GetResFactor();

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	unsigned int uiFactor;
};

class CSXCAD_EXPORT CSPropDumpBox : public CSProperties
{
public:
	CSPropDumpBox(ParameterSet* paraSet);
	CSPropDumpBox(CSProperties* prop);
	CSPropDumpBox(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropDumpBox();

	bool GetGlobalSetting();
	bool GetPhiDump();
	bool GetDivEDump();
	bool GetDivDDump();
	bool GetDivPDump();
	bool GetFieldWDump();
	bool GetChargeWDump();
	bool GetEFieldDump();
	bool GetDFieldDump();
	bool GetPFieldDump();
	bool GetSGDump();
	bool GetSimpleDump();
	int GetSGLevel();

	void SetGlobalSetting(bool val);
	void SetPhiDump(bool val);
	void SetDivEDump(bool val);
	void SetDivDDump(bool val);
	void SetDivPDump(bool val);
	void SetFieldWDump(bool val);
	void SetChargeWDump(bool val);
	void SetEFieldDump(bool val);
	void SetDFieldDump(bool val);
	void SetPFieldDump(bool val);
	void SetSGDump(bool val);
	void SetSimpleDump(bool val);
	void SetSGLevel(int val);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void Init();

protected:
	bool GlobalSetting;
	bool PhiDump,divE,divD,divP,FieldW,ChargeW;
	bool EField,DField,PField;
	bool SGDump,SimpleDump;
	int SGLevel;
};

