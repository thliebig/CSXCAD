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
#include "CSPrimitives.h"
#include "CSXCAD_Global.h"

using namespace std;

class CSPrimitives;
	class CSPrimBox;
	class CSPrimMultiBox;
	class CSPrimSphere;
	//usw

class CSProperties; //include VisualProperties
	class CSPropUnknown;
	class CSPropMaterial;
	class CSPropMetal;
	class CSPropElectrode;
	class CSPropChargeBox;
	class CSPropResBox;
	class CSPropDumpBox;
	//usw.
class TiXmlNode;

typedef struct
{
	unsigned char R,G,B,a;
} RGBa;

class CSXCAD_EXPORT CSProperties
{
public:
	virtual ~CSProperties();
	CSProperties(CSProperties* prop);
	enum PropertyType
	{
		ANY,UNKNOWN,MATERIAL,METAL,ELECTRODE,CHARGEBOX,RESBOX,DUMPBOX
	};

	int GetType();
	const char* GetTypeString();

	unsigned int GetID();
	void SetID(unsigned int ID);

	unsigned int GetUniqueID();
	void SetUniqueID(unsigned int uID);

	void SetName(const char* name);
	const char* GetName();

	void AddPrimitive(CSPrimitives *prim);
	void RemovePrimitive(CSPrimitives *prim);
	CSPrimitives* TakePrimitive(size_t index);

	//! Check whether the given coord is inside of a primitive assigned to this property and return found priority.
	bool CheckCoordInPrimitive(double *coord, int &priority, double tol=0);

	size_t GetQtyPrimitives();
	CSPrimitives* GetPrimitive(size_t index);
	void SetFillColor(RGBa color);
	RGBa GetFillColor();

	//void SetEdgeColor(int val) {iEdgeColor=val;};
	RGBa GetEdgeColor();
	void SetEdgeColor(RGBa color);

	bool GetVisibility();
	void SetVisibility(bool val);

	CSPropUnknown* ToUnknown();
	CSPropMaterial* ToMaterial();
	CSPropMetal* ToMetal();
	CSPropElectrode* ToElectrode();
	CSPropChargeBox* ToChargeBox();
	CSPropResBox* ToResBox();
	CSPropDumpBox* ToDumpBox();

	virtual bool Update(string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	CSProperties(ParameterSet* paraSet);
	CSProperties(unsigned int ID, ParameterSet* paraSet);
	ParameterSet* clParaSet;
	PropertyType Type;
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

	void SetProperty(const char* val);
	const char* GetProperty();

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

	void SetEpsilon(double val);
	void SetEpsilon(const char* val);
	double GetEpsilon();
	const char* GetEpsilonTerm();

	void SetMue(double val);
	void SetMue(const char* val);
	double GetMue();
	const char* GetMueTerm();

	void SetKappa(double val);
	void SetKappa(const char* val);
	double GetKappa();
	const char* GetKappaTerm();

	virtual void Init();
	virtual bool Update(string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	ParameterScalar Epsilon,Mue,Kappa;
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

	void SetExcitation(double val, int Component=0);
	void SetExcitation(const char* val, int Component=0);
	double GetExcitation(int Component=0);
	const char* GetExcitationString(int Comp=0);

	void SetWeightFct(const char* fct, int ny);
	const char* GetWeightFct(int ny);

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
	ParameterScalar Excitation[3];
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

