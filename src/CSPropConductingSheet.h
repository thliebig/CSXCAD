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

#include "CSPropMetal.h"

//! Continuous Structure Conductive Sheet Material Property
/*!
  This is a condiction sheet dispersive model for an efficient wide band Analysis of planar waveguides and circuits.
  */
class CSXCAD_EXPORT CSPropConductingSheet : public CSPropMetal
{
public:
	CSPropConductingSheet(ParameterSet* paraSet);
	CSPropConductingSheet(CSProperties* prop);
	CSPropConductingSheet(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropConductingSheet();

	virtual void Init();

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const std::string GetTypeXMLString() const {return std::string("ConductingSheet");}

	//! Set the Conductivity
	void SetConductivity(double val) {Conductivity.SetValue(val);}
	//! Set the Conductivity
	int  SetConductivity(const std::string val)  {return Conductivity.SetValue(val);}
	//! Get the Conductivity
	double GetConductivity() {return Conductivity.GetValue();}
	//! Get the Conductivity as a string
	const std::string GetConductivityTerm() {return Conductivity.GetString();}

	//! Set the Thickness
	void SetThickness(double val) {Thickness.SetValue(val);}
	//! Set the Thickness
	int  SetThickness(const std::string val)  {return Thickness.SetValue(val);}
	//! Get the Thickness
	double GetThickness() {return Thickness.GetValue();}
	//! Get the Thickness as a string
	const std::string GetThicknessTerm() {return Thickness.GetString();}

	virtual bool Update(std::string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void ShowPropertyStatus(std::ostream& stream);

protected:
	ParameterScalar Conductivity;
	ParameterScalar Thickness;
};
