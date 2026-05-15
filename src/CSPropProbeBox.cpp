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

#include "CSPropProbeBox.h"

CSPropProbeBox::CSPropProbeBox(ParameterSet* paraSet) : CSProperties(paraSet)
{
	Type=PROBEBOX; uiNumber=0; m_NormDir=-1; ProbeType=0; m_weight=1; bVisible=false; startTime=0; stopTime=0;
	m_ModeOrigin[0] = m_ModeOrigin[1] = m_ModeOrigin[2] = 0.0;
}
CSPropProbeBox::CSPropProbeBox(CSPropProbeBox* prop, bool copyPrim) : CSProperties(prop, copyPrim)
{
	Type=PROBEBOX;
	uiNumber=prop->uiNumber;
	m_NormDir=prop->m_NormDir;
	ProbeType=prop->ProbeType;
	m_weight=prop->m_weight;
	startTime=prop->startTime;
	stopTime=prop->stopTime;
	m_FD_Samples=prop->m_FD_Samples;
	m_ModeFile=prop->m_ModeFile;
	for (int n=0;n<3;++n) m_ModeFunction[n]=prop->m_ModeFunction[n];
	for (int n=0;n<3;++n) m_ModeOrigin[n]=prop->m_ModeOrigin[n];
}
CSPropProbeBox::CSPropProbeBox(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet)
{
	Type=PROBEBOX; uiNumber=0; m_NormDir=-1; ProbeType=0; m_weight=1; bVisible=false; startTime=0; stopTime=0;
	m_ModeOrigin[0] = m_ModeOrigin[1] = m_ModeOrigin[2] = 0.0;
}
CSPropProbeBox::~CSPropProbeBox() {}

void CSPropProbeBox::SetNumber(unsigned int val) {uiNumber=val;}
unsigned int CSPropProbeBox::GetNumber() {return uiNumber;}

void CSPropProbeBox::AddFDSample(std::vector<double> *freqs)
{
	for (size_t n=0;n<freqs->size();++n)
		AddFDSample(freqs->at(n));
}

void CSPropProbeBox::AddFDSample(std::string freqs)
{
	std::vector<double> v_freqs = SplitString2Double(freqs, ',');
	AddFDSample(&v_freqs);
}

bool CSPropProbeBox::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSProperties::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	prop->SetAttribute("Number",(int)uiNumber);
	prop->SetAttribute("Type",ProbeType);
	prop->SetDoubleAttribute("Weight",m_weight);
	prop->SetAttribute("NormDir",m_NormDir);
	prop->SetDoubleAttribute("StartTime",startTime);
	prop->SetDoubleAttribute("StopTime" ,stopTime );

	if (m_FD_Samples.size())
	{
		std::string fdSamples = CombineVector2String(m_FD_Samples,',');

		TiXmlElement FDS_Elem("FD_Samples");
		TiXmlText FDS_Text(fdSamples.c_str());
		FDS_Elem.InsertEndChild(FDS_Text);
		prop->InsertEndChild(FDS_Elem);
	}

	if (!m_ModeFile.empty())
		prop->SetAttribute("ModeFile", m_ModeFile.c_str());

	bool hasFunc = false;
	for (int n=0; n<3; ++n) if (!m_ModeFunction[n].empty()) hasFunc=true;
	if (hasFunc)
	{
		TiXmlElement MF("ModeFunction");
		const char* xyz[3] = {"X","Y","Z"};
		for (int n=0; n<3; ++n)
			MF.SetAttribute(xyz[n], m_ModeFunction[n].c_str());
		prop->InsertEndChild(MF);
	}

	if (m_ModeOrigin[0]!=0.0 || m_ModeOrigin[1]!=0.0 || m_ModeOrigin[2]!=0.0)
	{
		TiXmlElement MO("ModeOrigin");
		MO.SetDoubleAttribute("X", m_ModeOrigin[0]);
		MO.SetDoubleAttribute("Y", m_ModeOrigin[1]);
		MO.SetDoubleAttribute("Z", m_ModeOrigin[2]);
		prop->InsertEndChild(MO);
	}

	return true;
}

bool CSPropProbeBox::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;

	TiXmlElement *prop = root.ToElement();
	if (prop==NULL) return false;

	int iHelp;
	if (prop->QueryIntAttribute("Number",&iHelp)!=TIXML_SUCCESS) uiNumber=0;
	else uiNumber=(unsigned int)iHelp;

	if (prop->QueryIntAttribute("NormDir",&m_NormDir)!=TIXML_SUCCESS) m_NormDir=-1;

	if (prop->QueryIntAttribute("Type",&ProbeType)!=TIXML_SUCCESS) ProbeType=0;

	if (prop->QueryDoubleAttribute("Weight",&m_weight)!=TIXML_SUCCESS) m_weight=1;

	if (prop->QueryDoubleAttribute("StartTime",&startTime)!=TIXML_SUCCESS) startTime=0;
	if (prop->QueryDoubleAttribute("StopTime" ,&stopTime )!=TIXML_SUCCESS) stopTime =0;

	TiXmlElement* FDSamples = prop->FirstChildElement("FD_Samples");
	if (FDSamples!=NULL)
	{
		TiXmlNode* node = FDSamples->FirstChild();
		if (node)
		{
			TiXmlText* text = node->ToText();
			if (text)
				this->AddFDSample(text->Value());
		}
	}

	m_ModeFile.clear();
	prop->QueryStringAttribute("ModeFile", &m_ModeFile);

	m_ModeFunction[0] = m_ModeFunction[1] = m_ModeFunction[2] = "";
	TiXmlElement* mf = prop->FirstChildElement("ModeFunction");
	if (mf)
	{
		const char* v;
		if ((v = mf->Attribute("X"))) m_ModeFunction[0] = v;
		if ((v = mf->Attribute("Y"))) m_ModeFunction[1] = v;
		if ((v = mf->Attribute("Z"))) m_ModeFunction[2] = v;
	}
	else
	{
		// Compat: old Matlab stored mode functions in the generic <Attributes> element
		const char* dirs[3] = {"ModeFunctionX","ModeFunctionY","ModeFunctionZ"};
		for (int n=0; n<3; ++n)
		{
			if (ExistAttribute(dirs[n]))
			{
				m_ModeFunction[n] = GetAttributeValue(dirs[n]);
				RemoveAttribute(dirs[n]);
			}
		}
	}

	m_ModeOrigin[0] = m_ModeOrigin[1] = m_ModeOrigin[2] = 0.0;
	TiXmlElement* mo = prop->FirstChildElement("ModeOrigin");
	if (mo)
	{
		mo->QueryDoubleAttribute("X", &m_ModeOrigin[0]);
		mo->QueryDoubleAttribute("Y", &m_ModeOrigin[1]);
		mo->QueryDoubleAttribute("Z", &m_ModeOrigin[2]);
	}

	return true;
}
