/*
*	Copyright (C) 2008-2012 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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
	CSPropLorentzMaterial(CSProperties* prop);
	CSPropLorentzMaterial(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropLorentzMaterial();

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const string GetTypeXMLString() const {return string("LorentzMaterial");}

	//! Set the epsilon plasma frequency
	void SetEpsPlasmaFreq(int order, double val, int ny=0) {SetValue(val,EpsPlasma[order],ny);}
	//! Set the epsilon plasma frequency
	int  SetEpsPlasmaFreq(int order, const string val, int ny=0)  {return SetValue(val,EpsPlasma[order],ny);}
	//! Get the epsilon plasma frequency
	double GetEpsPlasmaFreq(int order, int ny=0) {return GetValue(EpsPlasma[order],ny);}
	//! Get the epsilon plasma frequency as a string
	const string GetEpsPlasmaFreqTerm(int order, int ny=0) {return GetTerm(EpsPlasma[order],ny);}

	//! Set the epsilon plasma frequency weighting
	int SetEpsPlasmaFreqWeightFunction(int order, const string val, int ny) {return SetValue(val,WeightEpsPlasma[order],ny);}
	//! Get the epsilon plasma frequency weighting string
	const string GetEpsPlasmaFreqWeightFunction(int order, int ny) {return GetTerm(WeightEpsPlasma[order],ny);}
	//! Get the epsilon plasma frequency weighting
	double GetEpsPlasmaFreqWeighted(int order, int ny, const double* coords) {return GetWeight(WeightEpsPlasma[order],ny,coords)*GetEpsPlasmaFreq(order,ny);}

	//! Set the epsilon relaxation time
	void SetEpsRelaxTime(int order, double val, int ny=0) {SetValue(val,EpsRelaxTime[order],ny);}
	//! Set the epsilon relaxation time
	int  SetEpsRelaxTime(int order, const string val, int ny=0)  {return SetValue(val,EpsRelaxTime[order],ny);}
	//! Get the epsilon relaxation time
	double GetEpsRelaxTime(int order, int ny=0) {return GetValue(EpsRelaxTime[order],ny);}
	//! Get the epsilon relaxation time as a string
	const string GetEpsRelaxTimeTerm(int order, int ny=0) {return GetTerm(EpsRelaxTime[order],ny);}

	//! Set the epsilon relaxation time weighting
	int SetEpsRelaxTimeWeightFunction(int order, const string val, int ny) {return SetValue(val,WeightEpsRelaxTime[order],ny);}
	//! Get the epsilon relaxation time weighting string
	const string GetEpsRelaxTimeWeightFunction(int order, int ny) {return GetTerm(WeightEpsRelaxTime[order],ny);}
	//! Get the epsilon relaxation time weighting
	double GetEpsRelaxTimeWeighted(int order, int ny, const double* coords) {return GetWeight(WeightEpsRelaxTime[order],ny,coords)*GetEpsPlasmaFreq(order,ny);}

	//! Set the mue plasma frequency
	void SetMuePlasmaFreq(int order, double val, int ny=0)  {SetValue(val,MuePlasma[order],ny);}
	//! Set the mue plasma frequency
	int SetMuePlasmaFreq(int order, const string val, int ny=0)  {return SetValue(val,MuePlasma[order],ny);}
	//! Get the mue plasma frequency
	double GetMuePlasmaFreq(int order, int ny=0)  {return GetValue(MuePlasma[order],ny);}
	//! Get the mue plasma frequency string
	const string GetMueTermPlasmaFreq(int order, int ny=0)  {return GetTerm(MuePlasma[order],ny);}

	//! Set the mue plasma frequency weighting
	int SetMuePlasmaFreqWeightFunction(int order, const string val, int ny) {return SetValue(val,WeightMuePlasma[order],ny);}
	//! Get the mue plasma frequency weighting string
	const string GetMuePlasmaFreqWeightFunction(int order, int ny) {return GetTerm(WeightMuePlasma[order],ny);}
	//! Get the mue plasma frequency weighting
	double GetMuePlasmaFreqWeighted(int order, int ny, const double* coords)  {return GetWeight(WeightMuePlasma[order],ny,coords)*GetMuePlasmaFreq(order,ny);}

	//! Set the mue relaxation time
	void SetMueRelaxTime(int order, double val, int ny=0)  {SetValue(val,MueRelaxTime[order],ny);}
	//! Set the mue relaxation time
	int SetMueRelaxTime(int order, const string val, int ny=0)  {return SetValue(val,MueRelaxTime[order],ny);}
	//! Get the mue relaxation time
	double GetMueRelaxTime(int order, int ny=0)  {return GetValue(MueRelaxTime[order],ny);}
	//! Get the mue relaxation time string
	const string GetMueTermRelaxTime(int order, int ny=0)  {return GetTerm(MueRelaxTime[order],ny);}

	//! Set the mue relaxation time weighting
	int SetMueRelaxTimeWeightFunction(int order, const string val, int ny) {return SetValue(val,WeightMueRelaxTime[order],ny);}
	//! Get the mue relaxation time weighting string
	const string GetMueRelaxTimeWeightFunction(int order, int ny) {return GetTerm(WeightMueRelaxTime[order],ny);}
	//! Get the mue relaxation time weighting
	double GetMueRelaxTimeWeighted(int order, int ny, const double* coords)  {return GetWeight(WeightMueRelaxTime[order],ny,coords)*GetMueRelaxTime(order,ny);}

	virtual void Init();
	virtual bool Update(string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	virtual void InitValues();
	virtual void DeleteValues();
	//! Epsilon and mue plasma frequncies
	ParameterScalar** EpsPlasma;
	ParameterScalar** MuePlasma;
	//! Epsilon and mue plasma frequncies weighting functions
	ParameterScalar** WeightEpsPlasma;
	ParameterScalar** WeightMuePlasma;

	//! Relaxation times for epsilon and mue
	ParameterScalar** EpsRelaxTime;
	ParameterScalar** MueRelaxTime;
	//! Relaxation times for epsilon and mue weighting functions
	ParameterScalar** WeightEpsRelaxTime;
	ParameterScalar** WeightMueRelaxTime;
};
