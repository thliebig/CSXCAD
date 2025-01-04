/*
*	Copyright (C) 2008-2025 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#include "CSPropLumpedElement.h"

CSPropLumpedElement::CSPropLumpedElement(ParameterSet* paraSet) : CSProperties(paraSet) {Type=LUMPED_ELEMENT;Init();}
CSPropLumpedElement::CSPropLumpedElement(CSPropLumpedElement* prop, bool copyPrim) : CSProperties(prop, copyPrim)
{
	Type=LUMPED_ELEMENT;
	Init();
	m_ny=prop->m_ny;
	m_Caps=prop->m_Caps;
	m_R.Copy(&prop->m_R);
	m_C.Copy(&prop->m_C);
	m_L.Copy(&prop->m_L);
	m_LEtype = prop->m_LEtype;
}
CSPropLumpedElement::CSPropLumpedElement(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=LUMPED_ELEMENT;Init();}
CSPropLumpedElement::~CSPropLumpedElement() {}

void CSPropLumpedElement::Init()
{
	m_ny=-1;
	m_Caps=true;
	m_R.SetValue(NAN);
	m_C.SetValue(NAN);
	m_L.SetValue(NAN);

	m_LEtype = LEtype::INVALID;
}

bool CSPropLumpedElement::Update(std::string *ErrStr)
{
	int EC=m_R.Evaluate();
	bool bOK=true;
	if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
	{
		std::stringstream stream;
		stream << std::endl << "Error in LumpedElement-Property Resistance-Value";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
		//cout << EC << std::endl;
	}

	EC=m_C.Evaluate();
	if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
	{
		std::stringstream stream;
		stream << std::endl << "Error in LumpedElement-Property Capacitor-Value";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
		//cout << EC << std::endl;
	}

	EC=m_L.Evaluate();
	if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
	{
		std::stringstream stream;
		stream << std::endl << "Error in LumpedElement-Property Inductance-Value";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
		//cout << EC << std::endl;
	}

	return bOK & CSProperties::Update(ErrStr);
}

void CSPropLumpedElement::SetDirection(int ny)
{
	if ((ny<0) || (ny>2)) return;
	m_ny = ny;
}

bool CSPropLumpedElement::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSProperties::Write2XML(root,parameterised,sparse)==false) return false;

	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	prop->SetAttribute("Direction",m_ny);
	prop->SetAttribute("Caps",(int)m_Caps);

	WriteTerm(m_R,*prop,"R",parameterised);
	WriteTerm(m_C,*prop,"C",parameterised);
	WriteTerm(m_L,*prop,"L",parameterised);

	// Write down the numeric value LEtype
	ParameterScalar s_LEtype;
	s_LEtype.SetValue(int(this->m_LEtype));
	WriteTerm(s_LEtype,*prop,"LEtype",parameterised);

	return true;
}

bool CSPropLumpedElement::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;

	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	if (prop->QueryIntAttribute("Direction",&m_ny)!=TIXML_SUCCESS) m_ny=-1;
	int caps=0;
	if (prop->QueryIntAttribute("Caps",&caps)!=TIXML_SUCCESS) m_Caps=true;
	else
		m_Caps = (bool)caps;

	if (ReadTerm(m_R,*prop,"R")==false)
		m_R.SetValue(NAN);
	if (ReadTerm(m_C,*prop,"C")==false)
		m_C.SetValue(NAN);
	if (ReadTerm(m_L,*prop,"L")==false)
		m_L.SetValue(NAN);

	// Read RLC type, "parallel" or "series"
	ParameterScalar s_LEtype;

	// The default is a parallel lumped circuit, in case none is supplied
	if (ReadTerm(s_LEtype,*prop,"LEtype")==false)
		m_LEtype = PARALLEL;
	else
	{
		int i_RLCtype = (int)(s_LEtype.GetValue());

		if (i_RLCtype == 0)
			m_LEtype = PARALLEL;
		else if (i_RLCtype == 1)
			m_LEtype = SERIES;
		else
			m_LEtype = INVALID;
	}

	return true;
}

void CSPropLumpedElement::ShowPropertyStatus(std::ostream& stream)
{
	CSProperties::ShowPropertyStatus(stream);
	stream << " --- Lumped Element Properties --- " << std::endl;
	stream << "  Direction: " << m_ny << std::endl;
	stream << "  Resistance: " << m_R.GetValueString() << std::endl;
	stream << "  Capacity: "   << m_C.GetValueString() << std::endl;
	stream << "  Inductance: " << m_L.GetValueString() << std::endl;

	std::string	s_type;
	switch (this->m_LEtype){
		case PARALLEL:
			s_type = "Parallel";
			break;
		case SERIES:
			s_type = "Series";
			break;
		case INVALID:
			s_type = "Invalid";
			break;
	}
	stream << "  Type: " << s_type << std::endl;
}


