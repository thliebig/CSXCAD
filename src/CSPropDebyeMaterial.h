/*
*	Copyright (C) 2013-2025 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

//! Continuous Structure Debye Dispersive Material Property
/*!
  This Property can hold information about the special properties of Debye dispersive materials.
  \todo Add all the other parameter needed by this model
  */
class CSXCAD_EXPORT CSPropDebyeMaterial : public CSPropDispersiveMaterial
{
public:
	CSPropDebyeMaterial(ParameterSet* paraSet);
	CSPropDebyeMaterial(CSPropDebyeMaterial* prop, bool copyPrim=false);
	CSPropDebyeMaterial(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropDebyeMaterial();

	//! Create a copy of this property. Optional: Copy all primitives assigned to this property too.
	virtual CSProperties* GetCopy(bool incl_prim=false) {return new CSPropDebyeMaterial(this, incl_prim);}

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const std::string GetTypeXMLString() const {return std::string("DebyeMaterial");}

	//! Set the epsilon plasma frequency
	void SetEpsDelta(int order, double val, int ny=0) {SetValue3(val,EpsDelta,order,ny);}
	//! Set the epsilon plasma frequency
	int  SetEpsDelta(int order, const std::string val, int ny=0)  {return SetValue3(val,EpsDelta,order,ny);}
	//! Get the epsilon plasma frequency
	double GetEpsDelta(int order, int ny=0) {return GetValue3(EpsDelta,order,ny);}
	//! Get the epsilon plasma frequency as a string
	const std::string GetEpsDeltaTerm(int order, int ny=0) {return GetTerm3(EpsDelta,order,ny);}

	//! Set the epsilon plasma frequency weighting
	int SetEpsDeltaWeightFunction(int order, const std::string val, int ny) {return SetValue3(val,WeightEpsDelta,order,ny);}
	//! Get the epsilon plasma frequency weighting string
	const std::string GetEpsDeltaWeightFunction(int order, int ny) {return GetTerm3(WeightEpsDelta,order,ny);}
	//! Get the epsilon plasma frequency weighting
	double GetEpsDeltaWeighted(int order, int ny, const double* coords) {return GetWeight3(WeightEpsDelta,order,ny,coords)*GetEpsDelta(order,ny);}

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

	virtual bool Update(std::string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void ShowPropertyStatus(std::ostream& stream);

protected:
	virtual void Init();
	virtual void InitValues();
	virtual void DeleteValues();
	//! Epsilon delta
	ParameterScalar** EpsDelta;
	//! Epsilon delta weighting functions
	ParameterScalar** WeightEpsDelta;

	//! Relaxation times for epsilon
	ParameterScalar** EpsRelaxTime;
	//! Relaxation times for epsilon weighting functions
	ParameterScalar** WeightEpsRelaxTime;
};
