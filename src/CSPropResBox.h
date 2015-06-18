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
	virtual const std::string GetTypeXMLString() const {return std::string("ResBox");}

	void SetResFactor(unsigned int val);
	unsigned int GetResFactor();

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	unsigned int uiFactor;
};
