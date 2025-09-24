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
	Type=PROBEBOX;
	uiNumber=0;
	m_NormDir=-1;
	ProbeType=0;
	m_weight=1;
	bVisisble=false;
	startTime=0;
	stopTime=0;

	for (uint dirIdx = 0 ; dirIdx < 3 ; dirIdx++)
	{
		Weights[dirIdx].clear();
		WeightCoords[dirIdx].clear();
	}
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

	for (uint dirIdx = 0 ; dirIdx < 3 ; dirIdx++)
	{
		Weights[dirIdx] = prop->Weights[dirIdx];
		WeightCoords[dirIdx] = prop->WeightCoords[dirIdx];
	}
}

CSPropProbeBox::CSPropProbeBox(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet)
{
	Type=PROBEBOX;
	uiNumber=0;
	m_NormDir=-1;
	ProbeType=0;
	m_weight=1;
	bVisisble=false;
	startTime=0;
	stopTime=0;

	for (uint dirIdx = 0 ; dirIdx < 3 ; dirIdx++)
	{
		Weights[dirIdx].clear();
		WeightCoords[dirIdx].clear();
	}
}

CSPropProbeBox::~CSPropProbeBox() {}

void CSPropProbeBox::SetNumber(unsigned int val) {uiNumber=val;}
unsigned int CSPropProbeBox::GetNumber() {return uiNumber;}

void CSPropProbeBox::SetManualWeights(float * wx, float * wy, float * wz, float * cx, float * cy, float * cz, uint listLength)
{
	for (uint dirIdx = 0 ; dirIdx < 3 ; dirIdx++)
	{
		Weights[dirIdx].clear();
		WeightCoords[dirIdx].clear();
	}

	Weights[0].insert(Weights[0].end(),wx,&wx[listLength]);
	Weights[1].insert(Weights[1].end(),wy,&wy[listLength]);
	Weights[2].insert(Weights[2].end(),wz,&wz[listLength]);

	WeightCoords[0].insert(WeightCoords[0].end(),cx,&cx[listLength]);
	WeightCoords[1].insert(WeightCoords[1].end(),cy,&cy[listLength]);
	WeightCoords[2].insert(WeightCoords[2].end(),cz,&cz[listLength]);

}

std::vector<float> CSPropProbeBox::GetManualWeights(uint dir)
{
	if (dir < 3)
		return Weights[dir];
	else
	{
		std::stringstream stream;
		stream << std::endl << "Error in obtaining manual weights - Wrong direction ";

		std::vector<float> tempVect;
		tempVect.clear();
		return tempVect;
	}
}

std::vector<float> CSPropProbeBox::GetManualWeightCoords(uint dir)
{
	if (dir < 3)
		return WeightCoords[dir];
	else
	{
		std::stringstream stream;
		stream << std::endl << "Error in obtaining manual weights - Wrong direction ";

		std::vector<float> tempVect;
		tempVect.clear();
		return tempVect;
	}
}

void CSPropProbeBox::ClearManualWeights()
{
	for (uint dirIdx = 0 ; dirIdx < 3 ; dirIdx++)
	{
		Weights[dirIdx].clear();
		WeightCoords[dirIdx].clear();
	}
}

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
	if ((m_NormDir>0) && (m_NormDir<3))
			prop->SetAttribute("NormDir",(int)uiNumber);
	prop->SetAttribute("Type",ProbeType);
	prop->SetAttribute("Weight",m_weight);
	prop->SetAttribute("NormDir",m_NormDir);
	prop->SetAttribute("StartTime",startTime);
	prop->SetAttribute("StopTime" ,stopTime );

	if (m_FD_Samples.size())
	{
		std::string fdSamples = CombineVector2String(m_FD_Samples,',');

		TiXmlElement FDS_Elem("FD_Samples");
		TiXmlText FDS_Text(fdSamples.c_str());
		FDS_Elem.InsertEndChild(FDS_Text);
		prop->InsertEndChild(FDS_Elem);
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

	return true;
}
