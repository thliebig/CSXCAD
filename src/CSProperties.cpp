/*
*	Copyright (C) 2008,2009,2010 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#include "CSProperties.h"
#include "CSPropUnknown.h"
#include "CSPropMaterial.h"
#include "CSPropDispersiveMaterial.h"
#include "CSPropLorentzMaterial.h"
#include "CSPropDebyeMaterial.h"
#include "CSPropDiscMaterial.h"
#include "CSPropLumpedElement.h"
#include "CSPropMetal.h"
#include "CSPropConductingSheet.h"
#include "CSPropExcitation.h"
#include "CSPropProbeBox.h"
#include "CSPropDumpBox.h"
#include "CSPropResBox.h"

#include "CSPrimitives.h"
#include <iostream>
#include <sstream>
#include "tinyxml.h"

/*********************CSProperties********************************************************************/
CSProperties::CSProperties(CSProperties* prop)
{
	uiID=prop->uiID;
	bMaterial=prop->bMaterial;
	coordInputType=prop->coordInputType;
	clParaSet=prop->clParaSet;
	FillColor=prop->FillColor;
	EdgeColor=prop->EdgeColor;
	bVisisble=prop->bVisisble;
	sName=std::string(prop->sName);
	for (size_t i=0;i<prop->vPrimitives.size();++i)
	{
		vPrimitives.push_back(prop->vPrimitives.at(i));
	}
	InitCoordParameter();
}

CSProperties::CSProperties(ParameterSet* paraSet)
{
	uiID=0;
	bMaterial=false;
	coordInputType=CARTESIAN;
	clParaSet=paraSet;
	FillColor.R=(rand()%256);
	FillColor.G=(rand()%256);
	FillColor.B=(rand()%256);
	EdgeColor.R=FillColor.R;
	EdgeColor.G=FillColor.G;
	EdgeColor.B=FillColor.B;
	FillColor.a=EdgeColor.a=255;
	bVisisble=true;
	Type=ANY;
	InitCoordParameter();
}

CSProperties::CSProperties(unsigned int ID, ParameterSet* paraSet)
{
	uiID=ID;
	bMaterial=false;
	coordInputType=CARTESIAN;
	clParaSet=paraSet;
	FillColor.R=(rand()%256);
	FillColor.G=(rand()%256);
	FillColor.B=(rand()%256);
	EdgeColor.R=FillColor.R;
	EdgeColor.G=FillColor.G;
	EdgeColor.B=FillColor.B;
	FillColor.a=EdgeColor.a=255;
	bVisisble=true;
	Type=ANY;
	InitCoordParameter();
}


CSProperties::~CSProperties()
{
	while (vPrimitives.size()>0)
		DeletePrimitive(vPrimitives.back());
	delete coordParaSet;
	coordParaSet=NULL;
}

void CSProperties::SetCoordInputType(CoordinateSystem type, bool CopyToPrimitives)
{
	coordInputType = type;
	if (CopyToPrimitives==false)
		return;
	for (size_t i=0;i<vPrimitives.size();++i)
		vPrimitives.at(i)->SetCoordInputType(type);
}

void CSProperties::InitCoordParameter()
{
	coordParaSet = new ParameterSet();

	coordPara[0]=new Parameter("x",0);
	coordPara[1]=new Parameter("y",0);
	coordPara[2]=new Parameter("z",0);
	coordPara[3]=new Parameter("rho",0);
	coordPara[4]=new Parameter("r",0);
	coordPara[5]=new Parameter("a",0);
	coordPara[6]=new Parameter("t",0);

	for (int i=0;i<7;++i)
		coordParaSet->LinkParameter(coordPara[i]); //the Paraset will take care of deletion...
}

int CSProperties::GetType() {return Type;}

unsigned int CSProperties::GetID() {return uiID;}
void CSProperties::SetID(unsigned int ID) {uiID=ID;}

unsigned int CSProperties::GetUniqueID() {return UniqueID;}
void CSProperties::SetUniqueID(unsigned int uID) {UniqueID=uID;}

void CSProperties::SetName(const std::string name) {sName=std::string(name);}
const std::string CSProperties::GetName() {return sName;}

bool CSProperties::ExistAttribute(std::string name)
{
	for (size_t n=0;n<m_Attribute_Name.size();++n)
	{
		if (m_Attribute_Name.at(n) == name)
			return true;
	}
	return false;
}

std::string CSProperties::GetAttributeValue(std::string name)
{
	for (size_t n=0;n<m_Attribute_Name.size();++n)
	{
		if (m_Attribute_Name.at(n) == name)
			return m_Attribute_Value.at(n);
	}
	return std::string();
}

void CSProperties::AddAttribute(std::string name, std::string value)
{
	if (name.empty()) return;
	m_Attribute_Name.push_back(name);
	m_Attribute_Value.push_back(value);
}

size_t CSProperties::GetQtyPrimitives() {return vPrimitives.size();}
CSPrimitives* CSProperties::GetPrimitive(size_t index) {if (index<vPrimitives.size()) return vPrimitives.at(index); else return NULL;}
void CSProperties::SetFillColor(RGBa color) {FillColor.R=color.R;FillColor.G=color.G;FillColor.B=color.B;FillColor.a=color.a;}
void CSProperties::SetFillColor(unsigned char R, unsigned char G, unsigned char B, unsigned char a) {FillColor.R=R;FillColor.G=G;FillColor.B=B;FillColor.a=a;}
RGBa CSProperties::GetFillColor() {return FillColor;}

RGBa CSProperties::GetEdgeColor() {return EdgeColor;}
void CSProperties::SetEdgeColor(RGBa color) {EdgeColor.R=color.R;EdgeColor.G=color.G;EdgeColor.B=color.B;EdgeColor.a=color.a;}
void CSProperties::SetEdgeColor(unsigned char R, unsigned char G, unsigned char B, unsigned char a) {EdgeColor.R=R;EdgeColor.G=G;EdgeColor.B=B;EdgeColor.a=a;}

bool CSProperties::GetVisibility() {return bVisisble;}
void CSProperties::SetVisibility(bool val) {bVisisble=val;}

CSPropUnknown* CSProperties::ToUnknown() { return dynamic_cast<CSPropUnknown*>(this); }
CSPropMaterial* CSProperties::ToMaterial() { return dynamic_cast<CSPropMaterial*>(this); }
CSPropLorentzMaterial* CSProperties::ToLorentzMaterial() { return dynamic_cast<CSPropLorentzMaterial*>(this); }
CSPropDebyeMaterial* CSProperties::ToDebyeMaterial() { return dynamic_cast<CSPropDebyeMaterial*>(this); }
CSPropDiscMaterial* CSProperties::ToDiscMaterial() { return dynamic_cast<CSPropDiscMaterial*>(this); }
CSPropMetal* CSProperties::ToMetal() { return dynamic_cast<CSPropMetal*>(this); }
CSPropConductingSheet* CSProperties::ToConductingSheet() { return dynamic_cast<CSPropConductingSheet*>(this); }
CSPropExcitation* CSProperties::ToExcitation() { return dynamic_cast<CSPropExcitation*>(this); }
CSPropProbeBox* CSProperties::ToProbeBox() { return dynamic_cast<CSPropProbeBox*>(this); }
CSPropResBox* CSProperties::ToResBox() { return dynamic_cast<CSPropResBox*>(this); }
CSPropDumpBox* CSProperties::ToDumpBox() { return dynamic_cast<CSPropDumpBox*>(this); }

bool CSProperties::Update(std::string */*ErrStr*/) {return true;}

bool CSProperties::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	prop->SetAttribute("ID",uiID);
	prop->SetAttribute("Name",sName.c_str());

	if (!sparse)
	{
		TiXmlElement FC("FillColor");
		FC.SetAttribute("R",FillColor.R);
		FC.SetAttribute("G",FillColor.G);
		FC.SetAttribute("B",FillColor.B);
		FC.SetAttribute("a",FillColor.a);
		prop->InsertEndChild(FC);
		TiXmlElement EC("EdgeColor");
		EC.SetAttribute("R",EdgeColor.R);
		EC.SetAttribute("G",EdgeColor.G);
		EC.SetAttribute("B",EdgeColor.B);
		EC.SetAttribute("a",EdgeColor.a);
		prop->InsertEndChild(EC);
	}

	if (m_Attribute_Name.size())
	{
		TiXmlElement Attributes("Attributes");
		for (size_t n=0;n<m_Attribute_Name.size();++n)
		{
			Attributes.SetAttribute(m_Attribute_Name.at(n).c_str(),m_Attribute_Value.at(n).c_str());
		}
		prop->InsertEndChild(Attributes);
	}

	TiXmlElement Primitives("Primitives");
	for (size_t i=0;i<vPrimitives.size();++i)
	{
		TiXmlElement PrimElem(vPrimitives.at(i)->GetTypeName().c_str());
		vPrimitives.at(i)->Write2XML(PrimElem,parameterised);
		Primitives.InsertEndChild(PrimElem);
	}
	prop->InsertEndChild(Primitives);

	return true;
}

void CSProperties::AddPrimitive(CSPrimitives *prim)
{
	if (HasPrimitive(prim)==true)
	{
		std::cerr << __func__ << ": Error, primitive is already owned by this property!" << std::endl;
		return;
	}
	vPrimitives.push_back(prim);
	prim->SetProperty(this);
}

bool CSProperties::HasPrimitive(CSPrimitives *prim)
{
	if (prim==NULL)
		return false;
	for (size_t i=0; i<vPrimitives.size();++i)
		if (vPrimitives.at(i)==prim)
			return true;
	return false;
}

void CSProperties::RemovePrimitive(CSPrimitives *prim)
{
	for (size_t i=0; i<vPrimitives.size();++i)
	{
		if (vPrimitives.at(i)==prim)
		{
			std::vector<CSPrimitives*>::iterator iter=vPrimitives.begin()+i;
			vPrimitives.erase(iter);
			prim->SetProperty(NULL);
			return;
		}
	}
}

void CSProperties::DeletePrimitive(CSPrimitives *prim)
{
	if (!HasPrimitive(prim))
	{
		std::cerr << __func__ << ": Error, primitive not found, can't delete it! Skipping." << std::endl;
		return;
	}
	RemovePrimitive(prim);
	delete prim;
}

CSPrimitives* CSProperties::TakePrimitive(size_t index)
{
	if (index>=vPrimitives.size()) return NULL;
	CSPrimitives* prim=vPrimitives.at(index);
	std::vector<CSPrimitives*>::iterator iter=vPrimitives.begin()+index;
	vPrimitives.erase(iter);
	return prim;
}

CSPrimitives* CSProperties::CheckCoordInPrimitive(const double *coord, int &priority, bool markFoundAsUsed, double tol)
{
	priority=0;
	CSPrimitives* found_CSPrim = NULL;
	bool found=false;
	for (size_t i=0; i<vPrimitives.size();++i)
	{
		if (vPrimitives.at(i)->IsInside(coord,tol)==true)
		{
			if (found==false)
			{
				priority=vPrimitives.at(i)->GetPriority()-1;
				found_CSPrim = vPrimitives.at(i);
			}
			found=true;
			if (vPrimitives.at(i)->GetPriority()>priority)
			{
				priority=vPrimitives.at(i)->GetPriority();
				found_CSPrim = vPrimitives.at(i);
			}
		}
	}
	if ((markFoundAsUsed) && (found_CSPrim))
		found_CSPrim->SetPrimitiveUsed(true);
	return found_CSPrim;
}

void CSProperties::WarnUnusedPrimitves(std::ostream& stream)
{
	if (vPrimitives.size()==0)
	{
		stream << "Warning: No primitives found in property: " << GetName() << "!" << std::endl;
		return;
	}
	for (size_t i=0; i<vPrimitives.size();++i)
	{
		if (vPrimitives.at(i)->GetPrimitiveUsed()==false)
		{
			stream << "Warning: Unused primitive (type: " << vPrimitives.at(i)->GetTypeName() << ") detected in property: " << GetName() << "!" << std::endl;
		}
	}
}

void CSProperties::ShowPropertyStatus(std::ostream& stream)
{
	stream << " Property #" << GetID() << " Type: \"" << GetTypeString() << "\" Name: \"" << GetName() << "\"" << std::endl;
	stream << " Primitive Count \t: " << vPrimitives.size() << std::endl;
	stream << " Coordinate System \t: " << coordInputType << std::endl;

	stream << "  -- Primitives: --" << std::endl;
	for (size_t i=0; i<vPrimitives.size();++i)
	{
		vPrimitives.at(i)->ShowPrimitiveStatus(stream);
		if (i<vPrimitives.size()-1)
			stream << " ---- " << std::endl;
	}
}

bool CSProperties::ReadFromXML(TiXmlNode &root)
{
	TiXmlElement* prop = root.ToElement();
	if (prop==NULL) return false;

	int help;
	if (prop->QueryIntAttribute("ID",&help)==TIXML_SUCCESS)
		uiID=help;

	const char* cHelp=prop->Attribute("Name");
	if (cHelp!=NULL) sName=std::string(cHelp);
	else sName.clear();

	TiXmlElement* FC = root.FirstChildElement("FillColor");
	if (FC!=NULL)
	{
		if (FC->QueryIntAttribute("R",&help)==TIXML_SUCCESS)
			FillColor.R=(unsigned char) help;
		if (FC->QueryIntAttribute("G",&help)==TIXML_SUCCESS)
			FillColor.G=(unsigned char) help;
		if (FC->QueryIntAttribute("B",&help)==TIXML_SUCCESS)
			FillColor.B=(unsigned char) help;
		if (FC->QueryIntAttribute("a",&help)==TIXML_SUCCESS)
			FillColor.a=(unsigned char) help;
	}

	TiXmlElement* EC = root.FirstChildElement("EdgeColor");
	if (EC!=NULL)
	{
		if (EC->QueryIntAttribute("R",&help)==TIXML_SUCCESS)
			EdgeColor.R=(unsigned char) help;
		if (EC->QueryIntAttribute("G",&help)==TIXML_SUCCESS)
			EdgeColor.G=(unsigned char) help;
		if (EC->QueryIntAttribute("B",&help)==TIXML_SUCCESS)
			EdgeColor.B=(unsigned char) help;
		if (EC->QueryIntAttribute("a",&help)==TIXML_SUCCESS)
			EdgeColor.a=(unsigned char) help;
	}

	TiXmlElement* att_root = root.FirstChildElement("Attributes");
	if (att_root)
	{
		TiXmlAttribute* att = att_root->FirstAttribute();
		while (att)
		{
			AddAttribute(att->Name(),att->Value());
			att = att->Next();
		}
	}

	return true;
}
