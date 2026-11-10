/*
*	Copyright (C) 2026 Roel Budé (roel.bude@proton.me)
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

#include "CSPropMetal.h"

//! Continuous Structure Lossy Metal Property
/*!
  A metal with a finite conductivity and an optional surface roughness.
  Unlike a ConductingSheet it is meant for ordinary 3D bodies: a solver
  applies the (rough) surface impedance to the faces of the body.
  The thickness is only used for zero-thickness (2D) primitives.
  CSXCAD only stores these values, it does not interpret them.
  */
class CSXCAD_EXPORT CSPropLossyMetal : public CSPropMetal
{
public:
	//! Surface roughness models
	enum RoughnessModel
	{
		ROUGHNESS_NONE = 0, ROUGHNESS_GRADIENT = 1
	};

	CSPropLossyMetal(ParameterSet* paraSet);
	CSPropLossyMetal(CSPropLossyMetal* prop, bool copyPrim=false);
	CSPropLossyMetal(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropLossyMetal();

	//! Create a copy of this property. Optional: Copy all primitives assigned to this property too.
	virtual CSProperties* GetCopy(bool incl_prim=false) {return new CSPropLossyMetal(this, incl_prim);}

	virtual void Init();

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const std::string GetTypeXMLString() const {return std::string("LossyMetal");}

	//! Set the Conductivity (S/m)
	void SetConductivity(double val) {Conductivity.SetValue(val);}
	//! Set the Conductivity (S/m)
	int  SetConductivity(const std::string val)  {return Conductivity.SetValue(val);}
	//! Get the Conductivity (S/m)
	double GetConductivity() {return Conductivity.GetValue();}
	//! Get the Conductivity as a string
	const std::string GetConductivityTerm() {return Conductivity.GetString();}

	//! Set the roughness model
	void SetRoughnessModel(RoughnessModel val) {m_RoughnessModel=val;}
	//! Set the roughness model by name ("none" or "gradient"), returns false for an unknown name
	bool SetRoughnessModel(const std::string val);
	//! Get the roughness model
	RoughnessModel GetRoughnessModel() const {return m_RoughnessModel;}
	//! Get the roughness model name
	std::string GetRoughnessModelName() const;

	//! Set the rms surface roughness Rq (m)
	void SetRq(double val) {Rq.SetValue(val);}
	//! Set the rms surface roughness Rq (m)
	int  SetRq(const std::string val)  {return Rq.SetValue(val);}
	//! Get the rms surface roughness Rq (m)
	double GetRq() {return Rq.GetValue();}
	//! Get the rms surface roughness as a string
	const std::string GetRqTerm() {return Rq.GetString();}

	//! Set the Thickness (m), used for zero-thickness primitives only
	void SetThickness(double val) {Thickness.SetValue(val);}
	//! Set the Thickness (m), used for zero-thickness primitives only
	int  SetThickness(const std::string val)  {return Thickness.SetValue(val);}
	//! Get the Thickness (m)
	double GetThickness() {return Thickness.GetValue();}
	//! Get the Thickness as a string
	const std::string GetThicknessTerm() {return Thickness.GetString();}

	virtual bool Update(std::string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void ShowPropertyStatus(std::ostream& stream);

protected:
	ParameterScalar Conductivity;
	RoughnessModel m_RoughnessModel;
	ParameterScalar Rq;
	ParameterScalar Thickness;
};
