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

#include "CSPropDumpBox.h"

CSPropDumpBox::CSPropDumpBox(ParameterSet* paraSet) : CSPropProbeBox(paraSet) {Type=DUMPBOX;Init();}
CSPropDumpBox::CSPropDumpBox(CSProperties* prop) : CSPropProbeBox(prop) {Type=DUMPBOX;Init();}
CSPropDumpBox::CSPropDumpBox(unsigned int ID, ParameterSet* paraSet) : CSPropProbeBox(ID,paraSet) {Type=DUMPBOX;Init();}
CSPropDumpBox::~CSPropDumpBox() {}

void CSPropDumpBox::Init()
{
	DumpType = 0;
	DumpMode = 0;
	FileType = 0;
	MultiGridLevel = 0;
	m_SubSampling=false;
	SubSampling[0]=1;
	SubSampling[1]=1;
	SubSampling[2]=1;
	m_OptResolution=false;
	OptResolution[0]=1;
	OptResolution[1]=1;
	OptResolution[2]=1;
}

void CSPropDumpBox::SetSubSampling(int ny, unsigned int val)
{
	if ((ny<0) || (ny>2)) return;
	if (val<1) return;
	m_SubSampling=true;
	SubSampling[ny] = val;
}

void CSPropDumpBox::SetSubSampling(unsigned int val[])
{
	for (int ny=0;ny<3;++ny)
		SetSubSampling(ny,val[ny]);
}

void CSPropDumpBox::SetSubSampling(const char* vals)
{
	if (vals==NULL) return;
	std::vector<int> values = SplitString2Int(std::string(vals),',');
	for (int ny=0;ny<3 && ny<(int)values.size();++ny)
		SetSubSampling(ny,values.at(ny));
}

unsigned int CSPropDumpBox::GetSubSampling(int ny)
{
	if ((ny<0) || (ny>2)) return 1;
	return SubSampling[ny];
}

void CSPropDumpBox::SetOptResolution(int ny, double val)
{
	if ((ny<0) || (ny>2)) return;
	if (val<0) return;
	m_OptResolution=true;
	OptResolution[ny] = val;
}

void CSPropDumpBox::SetOptResolution(double val[])
{
	for (int ny=0;ny<3;++ny)
		SetOptResolution(ny,val[ny]);
}

void CSPropDumpBox::SetOptResolution(const char* vals)
{
	if (vals==NULL) return;
	std::vector<double> values = SplitString2Double(std::string(vals),',');
	if (values.size()==1) //allow one resolution for all directions
	{
		for (int ny=0;ny<3;++ny)
			SetOptResolution(ny,values.at(0));
		return;
	}
	for (int ny=0;ny<3 && ny<(int)values.size();++ny)
		SetOptResolution(ny,values.at(ny));
}

double CSPropDumpBox::GetOptResolution(int ny)
{
	if ((ny<0) || (ny>2)) return 1;
	return OptResolution[ny];
}

bool CSPropDumpBox::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSPropProbeBox::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	prop->SetAttribute("DumpType",DumpType);
	prop->SetAttribute("DumpMode",DumpMode);
	prop->SetAttribute("FileType",FileType);
	prop->SetAttribute("MultiGridLevel",MultiGridLevel);

	if (m_SubSampling)
	{
		std::stringstream ss;
		ss << GetSubSampling(0) << "," << GetSubSampling(1) << "," << GetSubSampling(2) ;
		prop->SetAttribute("SubSampling",ss.str().c_str());
	}
	if (m_OptResolution)
	{
		std::stringstream ss;
		ss << GetOptResolution(0) << "," << GetOptResolution(1) << "," << GetOptResolution(2) ;
		prop->SetAttribute("OptResolution",ss.str().c_str());
	}

	return true;
}

bool CSPropDumpBox::ReadFromXML(TiXmlNode &root)
{
	if (CSPropProbeBox::ReadFromXML(root)==false) return false;

	TiXmlElement *prop = root.ToElement();
	if (prop==NULL) return false;

	if (prop->QueryIntAttribute("DumpType",&DumpType)!=TIXML_SUCCESS) DumpType=0;
	if (prop->QueryIntAttribute("DumpMode",&DumpMode)!=TIXML_SUCCESS) DumpMode=0;
	if (prop->QueryIntAttribute("FileType",&FileType)!=TIXML_SUCCESS) FileType=0;
	if (prop->QueryIntAttribute("MultiGridLevel",&MultiGridLevel)!=TIXML_SUCCESS) MultiGridLevel=0;

	SetSubSampling(prop->Attribute("SubSampling"));
	SetOptResolution(prop->Attribute("OptResolution"));

	return true;
}

void CSPropDumpBox::ShowPropertyStatus(std::ostream& stream)
{
	//skip output of prarent CSPropProbeBox
	CSProperties::ShowPropertyStatus(stream);
	stream << " --- Dump Properties --- " << std::endl;
	stream << "  DumpType: " << DumpType << "  DumpMode: " << DumpMode << " FileType: " << FileType << " MultiGridLevel: " << MultiGridLevel << std::endl;
	if (m_FD_Samples.size()>0)
		stream << "  Dump Frequencies: " << CombineVector2String(m_FD_Samples,',') << std::endl;
}
