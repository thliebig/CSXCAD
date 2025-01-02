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

#include "CSPropAbsorbingBC.h"

#define _C0_ 299792458.0

CSPropAbsorbingBC::CSPropAbsorbingBC(ParameterSet* paraSet) : CSProperties(paraSet) {Type=(CSProperties::PropertyType)(ABSORBING_BC); Init();}
CSPropAbsorbingBC::CSPropAbsorbingBC(CSProperties* prop) : CSProperties(prop) {Type=(CSProperties::PropertyType)(ABSORBING_BC); Init();}
CSPropAbsorbingBC::CSPropAbsorbingBC(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=(CSProperties::PropertyType)(ABSORBING_BC); Init();}

CSPropAbsorbingBC::~CSPropAbsorbingBC()
{
}

int CSPropAbsorbingBC::SetNormDir(const std::string val)
{

	char ch_axis = val[1];
	char ch_sign = val[0];

	NormDir.SetValue((1 - 2*(ch_axis == '-'))*(ch_axis - 'x' + 1));

	return (int)(NormDir.GetValue());
}

void CSPropAbsorbingBC::SetNormDir(int val)
{
	if (!((abs(val) >= 1) && (abs(val) <= 3)))
	{
		std::stringstream stream;
		stream << std::endl << "Error in AbsorbingBC - Property NormDir needs to be +-1,2,3";
	}

	NormDir.SetValue(val);
}

const std::string CSPropAbsorbingBC::GetNormDirTerm()
{
	char ch_axis[3] = {'x','y','z'};
	char ch_sign = (NormDir.GetValue() >= 0.0) ? '+' : '-';

	std::string outStr(2,' ');

	outStr[0] = ch_sign;
	outStr[1] = ch_axis[(int)(NormDir.GetValue()) - 1];

	return outStr;
}

void CSPropAbsorbingBC::Init()
{
	NormDir.SetValue(0);
	PhaseVelocity.SetValue(0);
	BoundaryType = CSPropAbsorbingBC::UNDEFINED;
}

bool CSPropAbsorbingBC::Update(std::string *ErrStr)
{
	int EC = NormDir.Evaluate();
	bool bOK = true;
	if (EC != ParameterScalar::PS_NO_ERROR) bOK = false;
	if ((EC != ParameterScalar::PS_NO_ERROR)  && (ErrStr != NULL))
	{
		std::stringstream stream;
		stream << std::endl << "Error in AbsorbingBC-Property NormDir-Value";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

	EC = PhaseVelocity.Evaluate();
	if (EC != ParameterScalar::PS_NO_ERROR) bOK = false;
	if ((EC != ParameterScalar::PS_NO_ERROR)  && (ErrStr != NULL))
	{
		std::stringstream stream;
		stream << std::endl << "Error in AbsorbingBC-Property PhaseVelocity-Value";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

	return bOK & CSProperties::Update(ErrStr);
}

bool CSPropAbsorbingBC::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSProperties::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	WriteTerm(NormDir,*prop,"NormDir",parameterised);
	WriteTerm(PhaseVelocity,*prop,"PhaseVelocity",parameterised);

	ParameterScalar s_BoundaryType;
	s_BoundaryType.SetValue((double)(this->BoundaryType));
	WriteTerm(s_BoundaryType,*prop,"ABCtype",parameterised);

	return true;
}

bool CSPropAbsorbingBC::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;

	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	if (ReadTerm(NormDir,*prop,"NormDir")==false)
		std::cerr << "CSPropAbsorbingBC::ReadFromXML: Warning: Failed to read Normal direction. Set to 0." << std::endl;

	if (ReadTerm(PhaseVelocity,*prop,"PhaseVelocity")==false)
		std::cerr << "CSPropAbsorbingBC::ReadFromXML: Warning: Failed to read Phase velocity. Set to C0." << std::endl;

	ParameterScalar s_BoundaryType;
	if (ReadTerm(s_BoundaryType,*prop,"BCtype")==false)
		std::cerr << "CSPropAbsorbingBC::ReadFromXML: Warning: Failed to read Boundary Condition type. Set to 'Mur 1st Order, single speed' (0)." << std::endl;
	else
		BoundaryType = (CSPropAbsorbingBC::ABCtype)(s_BoundaryType.GetValue());



	return true;
}

void CSPropAbsorbingBC::ShowPropertyStatus(std::ostream& stream)
{
	char ch_axis[3] = {'x','y','z'};
	char ch_sign = (NormDir.GetValue() >= 0.0) ? '+' : '-';

	std::string s_BoundaryType;

	switch (BoundaryType)
	{
	case MUR_1ST:
		s_BoundaryType = "Mur 1st order";
		break;
	case MUR_1ST_SA:
		s_BoundaryType = "Mur 1st order with Super Absorption";
		break;
	}


	CSProperties::ShowPropertyStatus(stream);
	stream << " --- Absorbing BC Properties --- " << std::endl;
	stream << "  Normal direction: " << ch_sign << ch_axis[abs((int)(NormDir.GetValue())) - 1] << std::endl;
	stream << "  Phase velocity: "   << PhaseVelocity.GetValue()/_C0_ << "*C0" << std::endl;
	stream << "  Boundary condition type: "   << s_BoundaryType << std::endl;

}
