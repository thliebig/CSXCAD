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

using namespace std;

class CSPrimitives;

class CSPropUnknown;
class CSPropMaterial;
class CSPropMetal;
class CSPropElectrode;
class CSPropProbeBox;
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
		ANY = 0xff,UNKNOWN = 0x01,MATERIAL = 0x02,METAL = 0x04,ELECTRODE = 0x08,PROBEBOX = 0x10,RESBOX = 0x20,DUMPBOX = 0x40
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
	//! Convert to ProbeBox Property, returns NULL if type is different! \return Returns a ToProbeBox* or NULL if type is different!
	CSPropProbeBox* ToProbeBox();
	//! Convert to ResBox Property, returns NULL if type is different! \return Returns a ToResBox* or NULL if type is different!
	CSPropResBox* ToResBox();
	//! Convert to DumpBox Property, returns NULL if type is different! \return Returns a ToDumpBox* or NULL if type is different!
	CSPropDumpBox* ToDumpBox();

	//! Update all parameters. Nothing to do in this base class. \param ErrStr Methode writes error messages to this string! \return Update success
	virtual bool Update(string *ErrStr=NULL);

	//! Write this property to a xml-node. \param parameterised Use false if parameters should be written as values. Parameters are lost!
	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	//! Read property from xml-node. \return Successful read-operation. 
	virtual bool ReadFromXML(TiXmlNode &root);

	//! Define the input type for the weighting coordinate system 0=cartesian, 1=cylindrical, 2=spherical
	void SetCoordInputType(int type) {coordInputType=type;}

protected:
	CSProperties(ParameterSet* paraSet);
	CSProperties(unsigned int ID, ParameterSet* paraSet);
	ParameterSet* clParaSet;
	ParameterSet* coordParaSet;
	//! x,y,z,rho,r,a,t one for all coord-systems (rho distance to z-axis (cylinder-coords), r for distance to origin)
	void InitCoordParameter();
	Parameter* coordPara[7];
	int coordInputType;
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

//! Continuous Structure Unknown Property
/*!
  This is a property that is getting designated for new properties which are unknown so far (e.g. written by a newer version of CSXCAD)
*/
class CSXCAD_EXPORT CSPropUnknown : public CSProperties
{
public:
	CSPropUnknown(ParameterSet* paraSet);
	CSPropUnknown(unsigned int ID, ParameterSet* paraSet);
	CSPropUnknown(CSProperties* prop);
	virtual ~CSPropUnknown();

	void SetProperty(const string val);
	const string GetProperty();

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	string sUnknownProperty;
};

//! Continuous Structure Material Property
/*!
  This Property can hold information about the properties of materials, such as epsilon, mue, kappa or sigma (aka. magnetic losses in FDTD).
  The material can be location dependent an unisotropic.
  */
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

	int SetEpsilonWeightFunction(const string fct, int ny);
	const string GetEpsilonWeightFunction(int ny);
	double GetEpsilonWeighted(int ny, double* coords);

        void SetMue(double val, int direction=0);
        void SetMue(const string val, int direction=0);
        double GetMue(int direction=0);
        const string GetMueTerm(int direction=0);

	int SetMueWeightFunction(const string fct, int ny);
	const string GetMueWeightFunction(int ny);
	double GetMueWeighted(int ny, double* coords);

        void SetKappa(double val, int direction=0);
        void SetKappa(const string val, int direction=0);
        double GetKappa(int direction=0);
        const string GetKappaTerm(int direction=0);

	int SetKappaWeightFunction(const string fct, int ny);
	const string GetKappaWeightFunction(int ny);
	double GetKappaWeighted(int ny, double* coords);
		
        void SetSigma(double val, int direction=0);
        void SetSigma(const string val, int direction=0);
        double GetSigma(int direction=0);
        const string GetSigmaTerm(int direction=0);

	int SetSigmaWeightFunction(const string fct, int ny);
	const string GetSigmaWeightFunction(int ny);
	double GetSigmaWeighted(int ny, double* coords);
		
        void SetIsotropy(bool val) {bIsotropy=val;};
        bool GetIsotropy() {return bIsotropy;};

        virtual void Init();
	virtual bool Update(string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
        ParameterScalar Epsilon[3],Mue[3],Kappa[3],Sigma[3];
		ParameterScalar WeightEpsilon[3],WeightMue[3],WeightKappa[3],WeightSigma[3];
		double GetWeight(ParameterScalar &ps, double* coords);
        bool bIsotropy;
};

//! Continuous Structure Metal Property
/*!
  This Property defines a metal property (aka. PEC).
  */
class CSXCAD_EXPORT CSPropMetal : public CSProperties
{
public:
	CSPropMetal(ParameterSet* paraSet);
	CSPropMetal(CSProperties* prop);
	CSPropMetal(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropMetal();

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);
};

//! Continuous Structure Electrode Property for excitations
/*!
  This Property defines an excitation which can be location and direction depedent.
  */
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

	//! This will override the weight-function
	void SetWeight(double val, int ny);
	//! Define a weight function
	int SetWeightFunction(const string fct, int ny);
	const string GetWeightFunction(int ny);

	double GetWeightedExcitation(int ny, double* coords);

	void SetDelay(double val);
	void SetDelay(const string val);
	double GetDelay();
	const string GetDelayString();

//	void SetWeightVars(const char* vars,int ny) {if ((ny>=0) && (ny<3)) sWeightVars[ny]=string(vars);};
//	const char* GetWeightVars(int ny) {if ((ny>=0) && (ny<3)) {if (sWeightVars[ny].empty()) return NULL; else return sWeightVars[ny].c_str();} else return NULL;};

	virtual void Init();
	virtual bool Update(string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	unsigned int uiNumber;
	int iExcitType;
	bool ActiveDir[3];
	ParameterScalar Excitation[3];
	ParameterScalar WeightFct[3];
	ParameterScalar Delay;//only for transient solver
};

//! Continuous Structure Probe Property for calculating integral properties
/*!
 CSProbProbeBox is a class for calculating integral properties such as (static) charges, voltages or currents.
*/
class CSXCAD_EXPORT CSPropProbeBox : public CSProperties
{
public:
	CSPropProbeBox(ParameterSet* paraSet);
	CSPropProbeBox(CSProperties* prop);
	CSPropProbeBox(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropProbeBox();

	//! Define a number for this probe property \sa GetNumber
	void SetNumber(unsigned int val);
	//! Get the number designated to this probe property \sa SetNumber
	unsigned int GetNumber();

	//! Define the probe type (e.g. type=0 for a charge integration, can/must be defined by the user interface) \sa GetProbeType
	void SetProbeType(int type) {ProbeType=type;}
	//! Get the probe type \sa SetProbeType
	int GetProbeType() {return ProbeType;}

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);
protected:
	unsigned int uiNumber;
	int ProbeType;
};

//! Continuous Structure Resolution Property
/*!
  This Property defines a refined mesh area.
  */
class CSXCAD_EXPORT CSPropResBox : public CSProperties
{
public:
	CSPropResBox(ParameterSet* paraSet);
	CSPropResBox(CSProperties* prop);
	CSPropResBox(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropResBox();

	void SetResFactor(unsigned int val);
	unsigned int GetResFactor();

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	unsigned int uiFactor;
};

//! Continuous Structure Dump Property
/*!
  This Property defines an area (box) designated for field dumps.
  */
class CSXCAD_EXPORT CSPropDumpBox : public CSProperties
{
public:
	CSPropDumpBox(ParameterSet* paraSet);
	CSPropDumpBox(CSProperties* prop);
	CSPropDumpBox(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropDumpBox();

	//! Define an arbitrary dump-type \sa GetDumpType
	void SetDumpType(int type) {DumpType=type;}
	//! Get the arbitrary dump-type \sa SetDumpType
	int GetDumpType() {return DumpType;}

	//! Define an arbitrary dump-mode \sa GetDumpMode
	void SetDumpMode(int mode) {DumpMode=mode;}
	//! Get the arbitrary dump-mode \sa SetDumpMode
	int GetDumpMode() {return DumpMode;}

	//! Define an arbitrary file-type \sa GetFileType
	void SetFileType(int ftype) {FileType=ftype;}
	//! Get the arbitrary file-type \sa SetFileType
	int GetFileType() {return FileType;}

	void SetSubSampling(int ny, unsigned int val);
	void SetSubSampling(unsigned int val[]);
	void SetSubSampling(const char* vals);
	unsigned int GetSubSampling(int ny);

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

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void Init();

protected:
	bool GlobalSetting;
	bool PhiDump,divE,divD,divP,FieldW,ChargeW;
	bool EField,DField,PField;
	bool SGDump,SimpleDump;
	int SGLevel;

	int DumpType;
	int DumpMode;
	int FileType;
	unsigned int SubSampling[3];
};

