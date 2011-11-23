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
#include "CSTransform.h"
#include "CSXCAD_Global.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

class CSPrimitives;

class CSPropUnknown;
class CSPropMaterial;
	class CSPropDispersiveMaterial;
		class CSPropLorentzMaterial;
		class CSPropDebyeMaterial;
	class CSPropDiscMaterial;
class CSPropLumpedElement;
class CSPropMetal;
class CSPropElectrode;
class CSPropProbeBox;
	class CSPropDumpBox;
class CSPropResBox;

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
		ANY = 0xfff, UNKNOWN = 0x001, MATERIAL = 0x002, METAL = 0x004, ELECTRODE = 0x008, PROBEBOX = 0x010, RESBOX = 0x020, DUMPBOX = 0x040, /* unused = 0x080, */
		DISPERSIVEMATERIAL = 0x100, LORENTZMATERIAL = 0x200, DEBYEMATERIAL = 0x400, /* unused dispersive material = 0x800 */
		DISCRETE_MATERIAL = 0x1000, LUMPED_ELEMENT = 0x2000
	};
	
	//! Get PropertyType \sa PropertyType and GetTypeString
	int GetType();
	//! Get PropertyType as a string \sa PropertyType and GetType
	const string GetTypeString();
	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const string GetTypeXMLString() {return string("Any");}
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

	//! Check if given attribute exists
	bool ExistAttribute(string name);
	//! Get the value of a given attribute
	string GetAttributeValue(string name);
	//! Add a new attribute
	void AddAttribute(string name, string value);

	//! Add a primitive to this Propertie. Takes ownership of this primitive! \sa CSPrimitives, RemovePrimitive, TakePrimitive
	void AddPrimitive(CSPrimitives *prim);
	//! Removes and deletes a primitive of this Property. \sa CSPrimitives, AddPrimitive, TakePrimitive
	void RemovePrimitive(CSPrimitives *prim);
	//! Take a primitive of this Propertie at index. Releases ownership of this primitive to caller! \sa CSPrimitives, RemovePrimitive, AddPrimitive \return NULL if not found!
	CSPrimitives* TakePrimitive(size_t index);

	//! Check whether the given coord is inside of a primitive assigned to this property and return the found primitive and \a priority.
	CSPrimitives* CheckCoordInPrimitive(const double *coord, int &priority, bool markFoundAsUsed=false, double tol=0);

	//! Get the quentity of primitives assigned to this property! \return Number of primitives in this property
	size_t GetQtyPrimitives();
	//! Get the Primitive at certain index position. \sa GetQtyPrimitives
	CSPrimitives* GetPrimitive(size_t index);

	//! Get all Primitives \sa GetPrimitive
	vector<CSPrimitives*> GetAllPrimitives() {return vPrimitives;}
	
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
	//! Convert to Material Property, returns NULL if type is different! \return Returns a CSPropMaterial* or NULL if type is different!
	CSPropMaterial* ToMaterial();
	//! Convert to Lorentzs-Material Property, returns NULL if type is different! \return Returns a CSPropLorentzMaterial* or NULL if type is different!
	CSPropLorentzMaterial* ToLorentzMaterial();
	//! Convert to Metal Property, returns NULL if type is different! \return Returns a CSPropMetal* or NULL if type is different!
	CSPropMetal* ToMetal();
	//! Convert to Electrode Property, returns NULL if type is different! \return Returns a CSPropElectrode* or NULL if type is different!
	CSPropElectrode* ToElectrode();
	//! Convert to ProbeBox Property, returns NULL if type is different! \return Returns a CSPropProbeBox* or NULL if type is different!
	CSPropProbeBox* ToProbeBox();
	//! Convert to ResBox Property, returns NULL if type is different! \return Returns a CSPropResBox* or NULL if type is different!
	CSPropResBox* ToResBox();
	//! Convert to DumpBox Property, returns NULL if type is different! \return Returns a CSPropDumpBox* or NULL if type is different!
	CSPropDumpBox* ToDumpBox();

	//! Update all parameters. Nothing to do in this base class. \param ErrStr Methode writes error messages to this string! \return Update success
	virtual bool Update(string *ErrStr=NULL);

	//! Write this property to a xml-node. \param parameterised Use false if parameters should be written as values. Parameters are lost!
	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	//! Read property from xml-node. \return Successful read-operation. 
	virtual bool ReadFromXML(TiXmlNode &root);

	//! Define the input type for the weighting coordinate system 0=cartesian, 1=cylindrical, 2=spherical
	void SetCoordInputType(CoordinateSystem type, bool CopyToPrimitives=true);
	//! Get the input type for the weighting coordinate system 0=cartesian, 1=cylindrical, 2=spherical
	int GetCoordInputType() const {return coordInputType;}

	//! Check and warn for unused primitives
	void WarnUnusedPrimitves(ostream& stream);

	//! Show status of this property, incl. all primitives
	virtual void ShowPropertyStatus(ostream& stream);

protected:
	CSProperties(ParameterSet* paraSet);
	CSProperties(unsigned int ID, ParameterSet* paraSet);
	ParameterSet* clParaSet;
	ParameterSet* coordParaSet;
	//! x,y,z,rho,r,a,t one for all coord-systems (rho distance to z-axis (cylinder-coords), r for distance to origin)
	void InitCoordParameter();
	Parameter* coordPara[7];
	CoordinateSystem coordInputType;
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

	//! List of additional attribute names
	vector<string> m_Attribute_Name;
	//! List of additional attribute values
	vector<string> m_Attribute_Value;
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

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const string GetTypeXMLString() {return string("Unknown");}

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

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const string GetTypeXMLString() {return string("Material");}

	void SetEpsilon(double val, int ny=0)		{SetValue(val,Epsilon,ny);}
	int SetEpsilon(const string val, int ny=0)	{return SetValue(val,Epsilon,ny);}
	double GetEpsilon(int ny=0)					{return GetValue(Epsilon,ny);}
	const string GetEpsilonTerm(int ny=0)		{return GetTerm(Epsilon,ny);}

	int SetEpsilonWeightFunction(const string fct, int ny)	{return SetValue(fct,WeightEpsilon,ny);}
	const string GetEpsilonWeightFunction(int ny)			{return GetTerm(WeightEpsilon,ny);}
	virtual double GetEpsilonWeighted(int ny, const double* coords)	{return GetWeight(WeightEpsilon,ny,coords)*GetEpsilon(ny);}

	void SetMue(double val, int ny=0)			{SetValue(val,Mue,ny);}
	int SetMue(const string val, int ny=0)		{return SetValue(val,Mue,ny);}
	double GetMue(int ny=0)						{return GetValue(Mue,ny);}
	const string GetMueTerm(int ny=0)			{return GetTerm(Mue,ny);}

	int SetMueWeightFunction(const string fct, int ny)	{return SetValue(fct,WeightMue,ny);}
	const string GetMueWeightFunction(int ny)			{return GetTerm(WeightMue,ny);}
	virtual double GetMueWeighted(int ny, const double* coords)	{return GetWeight(WeightMue,ny,coords)*GetMue(ny);}

	void SetKappa(double val, int ny=0)			{SetValue(val,Kappa,ny);}
	int SetKappa(const string val, int ny=0)	{return SetValue(val,Kappa,ny);}
	double GetKappa(int ny=0)					{return GetValue(Kappa,ny);}
	const string GetKappaTerm(int ny=0)			{return GetTerm(Kappa,ny);}

	int SetKappaWeightFunction(const string fct, int ny)	{return SetValue(fct,WeightKappa,ny);}
	const string GetKappaWeightFunction(int ny)				{return GetTerm(WeightKappa,ny);}
	virtual double GetKappaWeighted(int ny, const double* coords)	{return GetWeight(WeightKappa,ny,coords)*GetKappa(ny);}
		
	void SetSigma(double val, int ny=0)			{SetValue(val,Sigma,ny);}
	int SetSigma(const string val, int ny=0)	{return SetValue(val,Sigma,ny);}
	double GetSigma(int ny=0)					{return GetValue(Sigma,ny);}
	const string GetSigmaTerm(int ny=0)			{return GetTerm(Sigma,ny);}

	int SetSigmaWeightFunction(const string fct, int ny)	{return SetValue(fct,WeightSigma,ny);}
	const string GetSigmaWeightFunction(int ny)				{return GetTerm(WeightSigma,ny);}
	virtual double GetSigmaWeighted(int ny, const double* coords)	{return GetWeight(WeightSigma,ny,coords)*GetSigma(ny);}

	void SetDensity(double val)			{Density.SetValue(val);}
	int SetDensity(const string val)	{return Density.SetValue(val);}
	double GetDensity()					{return Density.GetValue();}
	const string GetDensityTerm()		{return Density.GetString();}

	int SetDensityWeightFunction(const string fct) {return WeightDensity.SetValue(fct);}
	const string GetDensityWeightFunction() {return WeightDensity.GetString();}
	virtual double GetDensityWeighted(const double* coords)	{return GetWeight(WeightDensity,coords)*GetDensity();}

	void SetIsotropy(bool val) {bIsotropy=val;};
	bool GetIsotropy() {return bIsotropy;};

	virtual void Init();
	virtual bool Update(string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void ShowPropertyStatus(ostream& stream);

protected:
	double GetValue(ParameterScalar *ps, int ny);
	string GetTerm(ParameterScalar *ps, int ny);
	void SetValue(double val, ParameterScalar *ps, int ny);
	int SetValue(string val, ParameterScalar *ps, int ny);

	//electro-magnetic properties
	ParameterScalar Epsilon[3],Mue[3],Kappa[3],Sigma[3];
	ParameterScalar WeightEpsilon[3],WeightMue[3],WeightKappa[3],WeightSigma[3];

	//other physical properties
	ParameterScalar Density, WeightDensity;

	double GetWeight(ParameterScalar &ps, const double* coords);
	double GetWeight(ParameterScalar *ps, int ny, const double* coords);
	bool bIsotropy;
};

//! Continuous Structure Dispersive Material Property
/*!
  This abstarct Property can hold information about the special properties of dispersive materials.
  */
class CSXCAD_EXPORT CSPropDispersiveMaterial : public CSPropMaterial
{
public:
	virtual ~CSPropDispersiveMaterial();

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const string GetTypeXMLString() {return string("DispersiveMaterial");}

protected:
	virtual bool Update(string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

	CSPropDispersiveMaterial(ParameterSet* paraSet);
	CSPropDispersiveMaterial(CSProperties* prop);
	CSPropDispersiveMaterial(unsigned int ID, ParameterSet* paraSet);
};

//! Continuous Structure Lorentz/ Drude Dispersive Material Property
/*!
  This Property can hold information about the special properties of Lorentz or Drude dispersive materials.
  The Drude material model is a special case of the Lorentz material model.
  \todo Add all the other parameter needed by this model
  */
class CSXCAD_EXPORT CSPropLorentzMaterial : public CSPropDispersiveMaterial
{
public:
	CSPropLorentzMaterial(ParameterSet* paraSet);
	CSPropLorentzMaterial(CSProperties* prop);
	CSPropLorentzMaterial(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropLorentzMaterial();

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const string GetTypeXMLString() {return string("LorentzMaterial");}

	//! Set the epsilon plasma frequency
	void SetEpsPlasmaFreq(double val, int ny=0) {SetValue(val,EpsPlasma,ny);}
	//! Set the epsilon plasma frequency
	int  SetEpsPlasmaFreq(const string val, int ny=0)  {return SetValue(val,EpsPlasma,ny);}
	//! Get the epsilon plasma frequency
	double GetEpsPlasmaFreq(int ny=0) {return GetValue(EpsPlasma,ny);}
	//! Get the epsilon plasma frequency as a string
	const string GetEpsPlasmaFreqTerm(int ny=0) {return GetTerm(EpsPlasma,ny);}

	//! Set the epsilon plasma frequency weighting
	int SetEpsPlasmaFreqWeightFunction(const string val, int ny) {return SetValue(val,WeightEpsPlasma,ny);}
	//! Get the epsilon plasma frequency weighting string
	const string GetEpsPlasmaFreqWeightFunction(int ny) {return GetTerm(EpsPlasma,ny);}
	//! Get the epsilon plasma frequency weighting
	double GetEpsPlasmaFreqWeighted(int ny, const double* coords) {return GetWeight(WeightEpsPlasma,ny,coords)*GetEpsPlasmaFreq(ny);}

	//! Set the mue plasma frequency
	void SetMuePlasmaFreq(double val, int ny=0)  {SetValue(val,MuePlasma,ny);}
	//! Set the mue plasma frequency
	int SetMuePlasmaFreq(const string val, int ny=0)  {return SetValue(val,MuePlasma,ny);}
	//! Get the mue plasma frequency
	double GetMuePlasmaFreq(int ny=0)  {return GetValue(MuePlasma,ny);}
	//! Get the mue plasma frequency string
	const string GetMueTermPlasmaFreq(int ny=0)  {return GetTerm(MuePlasma,ny);}

	//! Set the mue plasma frequency weighting
	int SetMuePlasmaFreqWeightFunction(const string val, int ny) {return SetValue(val,WeightMuePlasma,ny);}
	//! Get the mue plasma frequency weighting string
	const string GetMuePlasmaFreqWeightFunction(int ny) {return GetTerm(WeightMuePlasma,ny);}
	//! Get the mue plasma frequency weighting
	double GetMuePlasmaFreqWeighted(int ny, const double* coords)  {return GetWeight(WeightMuePlasma,ny,coords)*GetMuePlasmaFreq(ny);}

	virtual void Init();
	virtual bool Update(string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	//! Epsilon and mue plasma frequncies
	ParameterScalar EpsPlasma[3],MuePlasma[3];
	//! Epsilon and mue plasma frequncies weighting functions
	ParameterScalar WeightEpsPlasma[3],WeightMuePlasma[3];
};

//! Continuous Structure Discrete Material Property
/*!
  This Property reads a discrete material distribution from a file. (currently only HDF5)
  */
class CSXCAD_EXPORT CSPropDiscMaterial : public CSPropMaterial
{
public:
	CSPropDiscMaterial(ParameterSet* paraSet);
	CSPropDiscMaterial(CSProperties* prop);
	CSPropDiscMaterial(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropDiscMaterial();

	virtual const string GetTypeXMLString() {return string("Discrete-Material");}

	virtual double GetEpsilonWeighted(int ny, const double* coords);
	virtual double GetMueWeighted(int ny, const double* coords);
	virtual double GetKappaWeighted(int ny, const double* coords);
	virtual double GetSigmaWeighted(int ny, const double* coords);

	virtual double GetDensityWeighted(const double* coords);

	virtual void Init();

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

	bool ReadHDF5(string filename);

protected:
	unsigned int GetWeightingPos(const double* coords);

	int m_FileType;
	string m_Filename;
	unsigned int m_Size[3];
	float *m_mesh[3];
	float *m_Disc_epsR;
	float *m_Disc_kappa;
	float *m_Disc_mueR;
	float *m_Disc_sigma;
	float *m_Disc_Density;
	double m_Scale;
	CSTransform* m_Transform;
};

//! Continuous Structure Lumped Element Property
/*!
  This property represents lumped elements, e.g. smd capacitors etc.
  */
class CSXCAD_EXPORT CSPropLumpedElement : public CSProperties
{
public:
	CSPropLumpedElement(ParameterSet* paraSet);
	CSPropLumpedElement(CSProperties* prop);
	CSPropLumpedElement(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropLumpedElement();

	virtual void Init();

	void SetResistance(double val)			{m_R.SetValue(val);}
	int SetResistance(const string val)		{return m_R.SetValue(val);}
	double GetResistance() const			{return m_R.GetValue();}
	const string GetResistanceTerm() const	{return m_R.GetString();}

	void SetCapacity(double val)			{m_C.SetValue(val);}
	int SetCapacity(const string val)		{return m_C.SetValue(val);}
	double GetCapacity() const				{return m_C.GetValue();}
	const string GetCapacityTerm() const	{return m_C.GetString();}

	void SetInductance(double val)			{m_L.SetValue(val);}
	int SetInductance(const string val)		{return m_L.SetValue(val);}
	double GetInductance() const			{return m_L.GetValue();}
	const string GetInductanceTerm() const	{return m_L.GetString();}

	void SetDirection(int ny);
	int GetDirection() const {return m_ny;}

	void SetCaps(bool val) {m_Caps=val;}
	int GetCaps() const {return m_Caps;}

	void ShowPropertyStatus(ostream& stream);

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const string GetTypeXMLString() const {return string("LumpedElement");}

protected:
	int m_ny;
	bool m_Caps;
	ParameterScalar m_R,m_C,m_L;
	virtual bool Update(string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);
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

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const string GetTypeXMLString() {return string("Metal");}

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

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const string GetTypeXMLString() {return string("Electrode");}

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

	double GetWeightedExcitation(int ny, const double* coords);

	void SetDelay(double val);
	void SetDelay(const string val);
	double GetDelay();
	const string GetDelayString();

	virtual void Init();
	virtual bool Update(string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void ShowPropertyStatus(ostream& stream);

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

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const string GetTypeXMLString() {return string("ProbeBox");}

	//! Define a number for this probe property \sa GetNumber
	void SetNumber(unsigned int val);
	//! Get the number designated to this probe property \sa SetNumber
	unsigned int GetNumber();

	//! Define/Set the probe weighting \sa GetWeighting
	void SetWeighting(double weight) {m_weight=weight;}
	//! Get the probe weighting \sa GetWeighting
	double GetWeighting() {return m_weight;}

	//! Define the probe type (e.g. type=0 for a charge integration, can/must be defined by the user interface) \sa GetProbeType
	void SetProbeType(int type) {ProbeType=type;}
	//! Get the probe type \sa SetProbeType
	int GetProbeType() {return ProbeType;}

	size_t CountFDSamples() {return m_FD_Samples.size();}
	vector<double> *GetFDSamples()	{return &m_FD_Samples;}
	void ClearFDSamples() {m_FD_Samples.clear();}
	void AddFDSample(double freq) {m_FD_Samples.push_back(freq);}
	void AddFDSample(vector<double> *freqs);
	void AddFDSample(string freqs);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);
protected:
	unsigned int uiNumber;
	double m_weight;
	int ProbeType;
	vector<double> m_FD_Samples;
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

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const string GetTypeXMLString() {return string("ResBox");}

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
class CSXCAD_EXPORT CSPropDumpBox : public CSPropProbeBox
{
public:
	CSPropDumpBox(ParameterSet* paraSet);
	CSPropDumpBox(CSProperties* prop);
	CSPropDumpBox(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropDumpBox();

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const string GetTypeXMLString() {return string("DumpBox");}

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

	bool GetSubSampling() {return m_SubSampling;}
	void SetSubSampling(bool val) {m_SubSampling=val;}
	void SetSubSampling(int ny, unsigned int val);
	void SetSubSampling(unsigned int val[]);
	void SetSubSampling(const char* vals);
	unsigned int GetSubSampling(int ny);

	//! Get status of opt resolution flag
	bool GetOptResolution() {return m_OptResolution;}
	//! Set status of opt resolution flag
	void SetOptResolution(bool val) {m_OptResolution=val;}
	//! Set the opt resoultion for a given direction
	void SetOptResolution(int ny, double val);
	//! Set the opt resoultion for all directions
	void SetOptResolution(double val[]);
	//! Set the opt resoultion for all directions as string
	void SetOptResolution(const char* vals);
	//! Get the optimal resolution for a given direction
	double GetOptResolution(int ny);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void Init();

	virtual void ShowPropertyStatus(ostream& stream);

protected:
	int DumpType;
	int DumpMode;
	int FileType;

	//sub-sampling
	bool m_SubSampling;
	unsigned int SubSampling[3];

	//sub-sampling
	bool m_OptResolution;
	double OptResolution[3];
};

