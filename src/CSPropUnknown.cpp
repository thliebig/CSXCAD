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

#include "tinyxml.h"

#include "CSPropUnknown.h"

CSPropUnknown::CSPropUnknown(ParameterSet* paraSet) : CSProperties(paraSet) {Type=UNKNOWN;bVisisble=false;}
CSPropUnknown::CSPropUnknown(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=UNKNOWN;bVisisble=false;}
CSPropUnknown::CSPropUnknown(CSProperties* prop) : CSProperties(prop) {Type=UNKNOWN;bVisisble=false;}
CSPropUnknown::~CSPropUnknown() {}

void CSPropUnknown::SetProperty(const std::string val) {sUnknownProperty=std::string(val);}
const std::string CSPropUnknown::GetProperty() {return sUnknownProperty;}


bool CSPropUnknown::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSProperties::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	prop->SetAttribute("Property",sUnknownProperty.c_str());

	return true;
}

bool CSPropUnknown::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;
	TiXmlElement* prob=root.ToElement();

	const char* chProp=prob->Attribute("Property");
	if (chProp==NULL)
		sUnknownProperty=std::string("unknown");
	else sUnknownProperty=std::string(chProp);
	return true;
}
