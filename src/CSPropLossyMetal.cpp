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

#include "tinyxml.h"

#include "CSPropLossyMetal.h"

CSPropLossyMetal::CSPropLossyMetal(ParameterSet* paraSet) : CSPropMetal(paraSet) {Type=(CSProperties::PropertyType)(LOSSYMETAL | METAL);Init();}
CSPropLossyMetal::CSPropLossyMetal(CSPropLossyMetal* prop, bool copyPrim) : CSPropMetal(prop, copyPrim)
{
	Type=(CSProperties::PropertyType)(LOSSYMETAL | METAL);
	Init();
	Conductivity.Copy(&prop->Conductivity);
	m_RoughnessModel = prop->m_RoughnessModel;
	Rq.Copy(&prop->Rq);
	Thickness.Copy(&prop->Thickness);
}
CSPropLossyMetal::CSPropLossyMetal(unsigned int ID, ParameterSet* paraSet) : CSPropMetal(ID,paraSet) {Type=(CSProperties::PropertyType)(LOSSYMETAL | METAL);Init();}

CSPropLossyMetal::~CSPropLossyMetal()
{
}

void CSPropLossyMetal::Init()
{
	Conductivity.SetValue(0);
	m_RoughnessModel = ROUGHNESS_NONE;
	Rq.SetValue(0);
	Thickness.SetValue(0);
}

bool CSPropLossyMetal::SetRoughnessModel(const std::string val)
{
	if (val.empty() || (val=="none") || (val=="None"))
		m_RoughnessModel = ROUGHNESS_NONE;
	else if ((val=="gradient") || (val=="Gradient"))
		m_RoughnessModel = ROUGHNESS_GRADIENT;
	else
		return false;
	return true;
}

std::string CSPropLossyMetal::GetRoughnessModelName() const
{
	if (m_RoughnessModel==ROUGHNESS_GRADIENT)
		return std::string("gradient");
	return std::string("none");
}

bool CSPropLossyMetal::Update(std::string *ErrStr)
{
	bool bOK=true;
	ParameterScalar* values[] = {&Conductivity, &Rq, &Thickness};
	const char* names[] = {"Conductivity", "Rq", "Thickness"};
	for (int n=0; n<3; ++n)
	{
		int EC=values[n]->Evaluate();
		if (EC==ParameterScalar::PS_NO_ERROR)
			continue;
		bOK=false;
		if (ErrStr!=NULL)
		{
			std::stringstream stream;
			stream << std::endl << "Error in LossyMetal-Property " << names[n] << "-Value";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}
	return bOK & CSPropMetal::Update(ErrStr);
}

bool CSPropLossyMetal::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSPropMetal::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	WriteTerm(Conductivity,*prop,"Conductivity",parameterised);
	prop->SetAttribute("RoughnessModel",GetRoughnessModelName().c_str());
	WriteTerm(Rq,*prop,"Rq",parameterised);
	WriteTerm(Thickness,*prop,"Thickness",parameterised);

	return true;
}

bool CSPropLossyMetal::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;

	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	if (ReadTerm(Conductivity,*prop,"Conductivity")==false)
		std::cerr << "CSPropLossyMetal::ReadFromXML: Warning: Failed to read Conductivity. Set to 0." << std::endl;

	const char* model = prop->Attribute("RoughnessModel");
	if (model && (SetRoughnessModel(std::string(model))==false))
	{
		std::cerr << "CSPropLossyMetal::ReadFromXML: Warning: Unknown roughness model \"" << model << "\". Set to none." << std::endl;
		m_RoughnessModel = ROUGHNESS_NONE;
	}

	// optional attributes, keep the defaults if missing
	ReadTerm(Rq,*prop,"Rq");
	ReadTerm(Thickness,*prop,"Thickness");

	return true;
}

void CSPropLossyMetal::ShowPropertyStatus(std::ostream& stream)
{
	CSPropMetal::ShowPropertyStatus(stream);
	stream << " --- Lossy Metal Properties --- " << std::endl;
	stream << "  Conductivity: " << Conductivity.GetValueString() << std::endl;
	stream << "  Roughness model: " << GetRoughnessModelName() << std::endl;
	stream << "  Rq: " << Rq.GetValueString() << std::endl;
	stream << "  Thickness: " << Thickness.GetValueString() << std::endl;
}
