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
	virtual const std::string GetTypeXMLString() const {return std::string("Metal");}

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);
};
