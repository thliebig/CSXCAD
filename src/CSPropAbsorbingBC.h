/*
*	Copyright (C) 2023-2025 Gadi Lahav (gadi@rfwithcare.com)
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

//! Continuous Structure Absorbing Boundary Condition Sheet Property
/*!
  This is a localized absorbing sheet dispersive model for an efficient Analysis of waveguide structures.
  */
class CSXCAD_EXPORT CSPropAbsorbingBC : public CSProperties
{
public:
	enum ABCtype
	{
		UNDEFINED	= 0,
		MUR_1ST 	= 1,	// Mur's BC, 1st order
		MUR_1ST_SA 	= 2		// Mur's BC, 1st order, with Super Absorption
	};

	CSPropAbsorbingBC(ParameterSet* paraSet);
	CSPropAbsorbingBC(CSPropAbsorbingBC* prop, bool copyPrim = false);
	CSPropAbsorbingBC(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropAbsorbingBC();

	virtual void Init();

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const std::string GetTypeXMLString() const {return std::string("AbsorbingBC");}

	//! Set the normal sign. Direction will be set by post-processing the primitive itself.
	void SetNormalSignPositive(bool val) {this->NormSignPositive = val;}

	//! Get the normal direction
	bool GetNormalSignPositive() {return NormSignPositive;}

	//! Set the phase velocity using a double value
	void SetPhaseVelocity(double val);

	//! Get the phase velocity
	double GetPhaseVelocity() {return PhaseVelocity.GetValue();}
	const std::string GetPhaseVelocityTerm() const	{return PhaseVelocity.GetString();}

	void SetAbsorbingBoundaryType(ABCtype val) {AbsorbingBoundaryType = val;};
	ABCtype GetAbsorbingBoundaryType() {return AbsorbingBoundaryType;};

	virtual bool Update(std::string *ErrStr = NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised = true, bool sparse = false);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void ShowPropertyStatus(std::ostream& stream);

protected:

	bool			NormSignPositive;
	ParameterScalar	PhaseVelocity;
	ABCtype			AbsorbingBoundaryType;
};
