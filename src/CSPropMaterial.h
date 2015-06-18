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


//! Continuous Structure Material Property
/*!
  This Property can hold information about the properties of materials, such as epsilon, mue, kappa or sigma (aka. magnetic losses in FDTD).
  The material can be location dependent and unisotropic.
  */
class CSXCAD_EXPORT CSPropMaterial : public CSProperties
{
public:
	CSPropMaterial(ParameterSet* paraSet);
	CSPropMaterial(CSProperties* prop);
	CSPropMaterial(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropMaterial();

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const std::string GetTypeXMLString() const {return std::string("Material");}

	void SetEpsilon(double val, int ny=0)		{SetValue(val,Epsilon,ny);}
	int SetEpsilon(const std::string val, int ny=0)	{return SetValue(val,Epsilon,ny);}
	double GetEpsilon(int ny=0)					{return GetValue(Epsilon,ny);}
	const std::string GetEpsilonTerm(int ny=0)		{return GetTerm(Epsilon,ny);}

	int SetEpsilonWeightFunction(const std::string fct, int ny)	{return SetValue(fct,WeightEpsilon,ny);}
	const std::string GetEpsilonWeightFunction(int ny)			{return GetTerm(WeightEpsilon,ny);}
	virtual double GetEpsilonWeighted(int ny, const double* coords)	{return GetWeight(WeightEpsilon,ny,coords)*GetEpsilon(ny);}

	void SetMue(double val, int ny=0)			{SetValue(val,Mue,ny);}
	int SetMue(const std::string val, int ny=0)		{return SetValue(val,Mue,ny);}
	double GetMue(int ny=0)						{return GetValue(Mue,ny);}
	const std::string GetMueTerm(int ny=0)			{return GetTerm(Mue,ny);}

	int SetMueWeightFunction(const std::string fct, int ny)	{return SetValue(fct,WeightMue,ny);}
	const std::string GetMueWeightFunction(int ny)			{return GetTerm(WeightMue,ny);}
	virtual double GetMueWeighted(int ny, const double* coords)	{return GetWeight(WeightMue,ny,coords)*GetMue(ny);}

	void SetKappa(double val, int ny=0)			{SetValue(val,Kappa,ny);}
	int SetKappa(const std::string val, int ny=0)	{return SetValue(val,Kappa,ny);}
	double GetKappa(int ny=0)					{return GetValue(Kappa,ny);}
	const std::string GetKappaTerm(int ny=0)			{return GetTerm(Kappa,ny);}

	int SetKappaWeightFunction(const std::string fct, int ny)	{return SetValue(fct,WeightKappa,ny);}
	const std::string GetKappaWeightFunction(int ny)				{return GetTerm(WeightKappa,ny);}
	virtual double GetKappaWeighted(int ny, const double* coords)	{return GetWeight(WeightKappa,ny,coords)*GetKappa(ny);}

	void SetSigma(double val, int ny=0)			{SetValue(val,Sigma,ny);}
	int SetSigma(const std::string val, int ny=0)	{return SetValue(val,Sigma,ny);}
	double GetSigma(int ny=0)					{return GetValue(Sigma,ny);}
	const std::string GetSigmaTerm(int ny=0)			{return GetTerm(Sigma,ny);}

	int SetSigmaWeightFunction(const std::string fct, int ny)	{return SetValue(fct,WeightSigma,ny);}
	const std::string GetSigmaWeightFunction(int ny)				{return GetTerm(WeightSigma,ny);}
	virtual double GetSigmaWeighted(int ny, const double* coords)	{return GetWeight(WeightSigma,ny,coords)*GetSigma(ny);}

	void SetDensity(double val)			{Density.SetValue(val);}
	int SetDensity(const std::string val)	{return Density.SetValue(val);}
	double GetDensity()					{return Density.GetValue();}
	const std::string GetDensityTerm()		{return Density.GetString();}

	int SetDensityWeightFunction(const std::string fct) {return WeightDensity.SetValue(fct);}
	const std::string GetDensityWeightFunction() {return WeightDensity.GetString();}
	virtual double GetDensityWeighted(const double* coords)	{return GetWeight(WeightDensity,coords)*GetDensity();}

	void SetIsotropy(bool val) {bIsotropy=val;}
	bool GetIsotropy() {return bIsotropy;}

	virtual void Init();
	virtual bool Update(std::string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void ShowPropertyStatus(std::ostream& stream);

protected:
	double GetValue(ParameterScalar *ps, int ny);
	std::string GetTerm(ParameterScalar *ps, int ny);
	void SetValue(double val, ParameterScalar *ps, int ny);
	int SetValue(std::string val, ParameterScalar *ps, int ny);

	//electro-magnetic properties
	ParameterScalar Epsilon[3],Mue[3],Kappa[3],Sigma[3];
	ParameterScalar WeightEpsilon[3],WeightMue[3],WeightKappa[3],WeightSigma[3];

	//other physical properties
	ParameterScalar Density, WeightDensity;

	double GetWeight(ParameterScalar &ps, const double* coords);
	double GetWeight(ParameterScalar *ps, int ny, const double* coords);
	bool bIsotropy;
};
