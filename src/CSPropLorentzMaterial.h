/*
*	Copyright (C) 2008-2025 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#include "CSProperties.h"
#include "CSPropDispersiveMaterial.h"

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
	CSPropLorentzMaterial(CSPropLorentzMaterial* prop, bool copyPrim=false);
	CSPropLorentzMaterial(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropLorentzMaterial();

	//! Create a copy of this property. Optional: Copy all primitives assigned to this property too.
	virtual CSProperties* GetCopy(bool incl_prim=false) {return new CSPropLorentzMaterial(this, incl_prim);}

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const std::string GetTypeXMLString() const {return std::string("LorentzMaterial");}

	//! Set the epsilon plasma frequency
	void SetEpsPlasmaFreq(int order, double val, int ny=0) {SetValue3(val,EpsPlasma,order,ny);}
	//! Set the epsilon plasma frequency
	int  SetEpsPlasmaFreq(int order, const std::string val, int ny=0)  {return SetValue3(val,EpsPlasma,order,ny);}
	//! Get the epsilon plasma frequency
	double GetEpsPlasmaFreq(int order, int ny=0) {return GetValue3(EpsPlasma,order,ny);}
	//! Get the epsilon plasma frequency as a string
	const std::string GetEpsPlasmaFreqTerm(int order, int ny=0) {return GetTerm3(EpsPlasma,order,ny);}

	//! Set the epsilon plasma frequency weighting
	int SetEpsPlasmaFreqWeightFunction(int order, const std::string val, int ny) {return SetValue3(val,WeightEpsPlasma,order,ny);}
	//! Get the epsilon plasma frequency weighting string
	const std::string GetEpsPlasmaFreqWeightFunction(int order, int ny) {return GetTerm3(WeightEpsPlasma,order,ny);}
	//! Get the epsilon plasma frequency weighting
	double GetEpsPlasmaFreqWeighted(int order, int ny, const double* coords) {return GetWeight3(WeightEpsPlasma,order,ny,coords)*GetEpsPlasmaFreq(order,ny);}

	//! Set the epsilon lorentz pole frequency
	void SetEpsLorPoleFreq(int order, double val, int ny=0) {SetValue3(val,EpsLorPole,order,ny);}
	//! Set the epsilon lorentz pole frequency
	int  SetEpsLorPoleFreq(int order, const std::string val, int ny=0)  {return SetValue3(val,EpsLorPole,order,ny);}
	//! Get the epsilon lorentz pole frequency
	double GetEpsLorPoleFreq(int order, int ny=0) {return GetValue3(EpsLorPole,order,ny);}
	//! Get the epsilon lorentz pole frequency as a string
	const std::string GetEpsLorPoleFreqTerm(int order, int ny=0) {return GetTerm3(EpsLorPole,order,ny);}

	//! Set the epsilon lorentz pole frequency weighting
	int SetEpsLorPoleFreqWeightFunction(int order, const std::string val, int ny) {return SetValue3(val,WeightEpsLorPole,order,ny);}
	//! Get the epsilon lorentz pole frequency weighting string
	const std::string GetEpsLorPoleFreqWeightFunction(int order, int ny) {return GetTerm3(WeightEpsLorPole,order,ny);}
	//! Get the epsilon lorentz pole frequency weighting
	double GetEpsLorPoleFreqWeighted(int order, int ny, const double* coords) {return GetWeight3(WeightEpsLorPole,order,ny,coords)*GetEpsLorPoleFreq(order,ny);}

	//! Set the epsilon relaxation time
	void SetEpsRelaxTime(int order, double val, int ny=0) {SetValue3(val,EpsRelaxTime,order,ny);}
	//! Set the epsilon relaxation time
	int  SetEpsRelaxTime(int order, const std::string val, int ny=0)  {return SetValue3(val,EpsRelaxTime,order,ny);}
	//! Get the epsilon relaxation time
	double GetEpsRelaxTime(int order, int ny=0) {return GetValue3(EpsRelaxTime,order,ny);}
	//! Get the epsilon relaxation time as a string
	const std::string GetEpsRelaxTimeTerm(int order, int ny=0) {return GetTerm3(EpsRelaxTime,order,ny);}

	//! Set the epsilon relaxation time weighting
	int SetEpsRelaxTimeWeightFunction(int order, const std::string val, int ny) {return SetValue3(val,WeightEpsRelaxTime,order,ny);}
	//! Get the epsilon relaxation time weighting string
	const std::string GetEpsRelaxTimeWeightFunction(int order, int ny) {return GetTerm3(WeightEpsRelaxTime,order,ny);}
	//! Get the epsilon relaxation time weighting
	double GetEpsRelaxTimeWeighted(int order, int ny, const double* coords) {return GetWeight3(WeightEpsRelaxTime,order,ny,coords)*GetEpsRelaxTime(order,ny);}

	//! Set the mue plasma frequency
	void SetMuePlasmaFreq(int order, double val, int ny=0)  {SetValue3(val,MuePlasma,order,ny);}
	//! Set the mue plasma frequency
	int SetMuePlasmaFreq(int order, const std::string val, int ny=0)  {return SetValue3(val,MuePlasma,order,ny);}
	//! Get the mue plasma frequency
	double GetMuePlasmaFreq(int order, int ny=0)  {return GetValue3(MuePlasma,order,ny);}
	//! Get the mue plasma frequency string
	const std::string GetMueTermPlasmaFreq(int order, int ny=0)  {return GetTerm3(MuePlasma,order,ny);}

	//! Set the mue plasma frequency weighting
	int SetMuePlasmaFreqWeightFunction(int order, const std::string val, int ny) {return SetValue3(val,WeightMuePlasma,order,ny);}
	//! Get the mue plasma frequency weighting string
	const std::string GetMuePlasmaFreqWeightFunction(int order, int ny) {return GetTerm3(WeightMuePlasma,order,ny);}
	//! Get the mue plasma frequency weighting
	double GetMuePlasmaFreqWeighted(int order, int ny, const double* coords)  {return GetWeight3(WeightMuePlasma,order,ny,coords)*GetMuePlasmaFreq(order,ny);}

	//! Set the mue lorentz pole frequency
	void SetMueLorPoleFreq(int order, double val, int ny=0)  {SetValue3(val,MueLorPole,order,ny);}
	//! Set the mue lorentz pole frequency
	int SetMueLorPoleFreq(int order, const std::string val, int ny=0)  {return SetValue3(val,MueLorPole,order,ny);}
	//! Get the mue lorentz pole frequency
	double GetMueLorPoleFreq(int order, int ny=0)  {return GetValue3(MueLorPole,order,ny);}
	//! Get the mue lorentz pole frequency string
	const std::string GetMueTermLorPoleFreq(int order, int ny=0)  {return GetTerm3(MueLorPole,order,ny);}

	//! Set the mue lorentz pole frequency weighting
	int SetMueLorPoleFreqWeightFunction(int order, const std::string val, int ny) {return SetValue3(val,WeightMueLorPole,order,ny);}
	//! Get the mue lorentz pole frequency weighting string
	const std::string GetMueLorPoleFreqWeightFunction(int order, int ny) {return GetTerm3(WeightMueLorPole,order,ny);}
	//! Get the mue lorentz pole frequency weighting
	double GetMueLorPoleFreqWeighted(int order, int ny, const double* coords)  {return GetWeight3(WeightMueLorPole,order,ny,coords)*GetMueLorPoleFreq(order,ny);}

	//! Set the mue relaxation time
	void SetMueRelaxTime(int order, double val, int ny=0)  {SetValue3(val,MueRelaxTime,order,ny);}
	//! Set the mue relaxation time
	int SetMueRelaxTime(int order, const std::string val, int ny=0)  {return SetValue3(val,MueRelaxTime,order,ny);}
	//! Get the mue relaxation time
	double GetMueRelaxTime(int order, int ny=0)  {return GetValue3(MueRelaxTime,order,ny);}
	//! Get the mue relaxation time string
	const std::string GetMueTermRelaxTime(int order, int ny=0)  {return GetTerm3(MueRelaxTime,order,ny);}

	//! Set the mue relaxation time weighting
	int SetMueRelaxTimeWeightFunction(int order, const std::string val, int ny) {return SetValue3(val,WeightMueRelaxTime,order,ny);}
	//! Get the mue relaxation time weighting string
	const std::string GetMueRelaxTimeWeightFunction(int order, int ny) {return GetTerm3(WeightMueRelaxTime,order,ny);}
	//! Get the mue relaxation time weighting
	double GetMueRelaxTimeWeighted(int order, int ny, const double* coords)  {return GetWeight3(WeightMueRelaxTime,order,ny,coords)*GetMueRelaxTime(order,ny);}

	virtual bool Update(std::string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void ShowPropertyStatus(std::ostream& stream);

protected:
	virtual void Init();
	virtual void InitValues();
	virtual void DeleteValues();
	//! Epsilon and mue plasma frequncies
	ParameterScalar** EpsPlasma;
	ParameterScalar** MuePlasma;
	//! Epsilon and mue plasma frequncies weighting functions
	ParameterScalar** WeightEpsPlasma;
	ParameterScalar** WeightMuePlasma;

	//! Epsilon and mue lorentz pole frequncies
	ParameterScalar** EpsLorPole;
	ParameterScalar** MueLorPole;
	//! Epsilon and mue lorentz pole frequncies weighting functions
	ParameterScalar** WeightEpsLorPole;
	ParameterScalar** WeightMueLorPole;

	//! Relaxation times for epsilon and mue
	ParameterScalar** EpsRelaxTime;
	ParameterScalar** MueRelaxTime;
	//! Relaxation times for epsilon and mue weighting functions
	ParameterScalar** WeightEpsRelaxTime;
	ParameterScalar** WeightMueRelaxTime;
};
