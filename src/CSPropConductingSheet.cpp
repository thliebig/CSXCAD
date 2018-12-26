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

#include "CSPropConductingSheet.h"

CSPropConductingSheet::CSPropConductingSheet(ParameterSet* paraSet) : CSPropMetal(paraSet) {Type=(CSProperties::PropertyType)(CONDUCTINGSHEET | METAL);Init();}
CSPropConductingSheet::CSPropConductingSheet(CSProperties* prop) : CSPropMetal(prop) {Type=(CSProperties::PropertyType)(CONDUCTINGSHEET | METAL);Init();}
CSPropConductingSheet::CSPropConductingSheet(unsigned int ID, ParameterSet* paraSet) : CSPropMetal(ID,paraSet) {Type=(CSProperties::PropertyType)(CONDUCTINGSHEET | METAL);Init();}

CSPropConductingSheet::~CSPropConductingSheet()
{
}


void CSPropConductingSheet::Init()
{
	Conductivity.SetValue(0);
	Thickness.SetValue(0);
}


bool CSPropConductingSheet::Update(std::string *ErrStr)
{
	int EC=Conductivity.Evaluate();
	bool bOK=true;
	if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
	{
		std::stringstream stream;
		stream << std::endl << "Error in ConductingSheet-Property Conductivity-Value";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

	EC=Thickness.Evaluate();
	if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
	{
		std::stringstream stream;
		stream << std::endl << "Error in ConductingSheet-Property Thickness-Value";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

	return bOK & CSPropMetal::Update(ErrStr);
}

bool CSPropConductingSheet::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSPropMetal::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	WriteTerm(Conductivity,*prop,"Conductivity",parameterised);
	WriteTerm(Thickness,*prop,"Thickness",parameterised);

	return true;
}

bool CSPropConductingSheet::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;

	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	if (ReadTerm(Conductivity,*prop,"Conductivity")==false)
		std::cerr << "CSPropConductingSheet::ReadFromXML: Warning: Failed to read Conductivity. Set to 0." << std::endl;
	if (ReadTerm(Thickness,*prop,"Thickness")==false)
		std::cerr << "CSPropConductingSheet::ReadFromXML: Warning: Failed to read Thickness. Set to 0." << std::endl;

	return true;
}

void CSPropConductingSheet::ShowPropertyStatus(std::ostream& stream)
{
	CSPropMetal::ShowPropertyStatus(stream);
	stream << " --- Conducting Sheet Properties --- " << std::endl;
	stream << "  Conductivity: " << Conductivity.GetValueString() << std::endl;
	stream << "  Thickness: "   << Thickness.GetValueString() << std::endl;
}
