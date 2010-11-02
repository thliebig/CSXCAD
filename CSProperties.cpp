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
#include "CSPrimitives.h"
#include "CSUseful.h"
#include "ParameterObjects.h"
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
	sName=string(prop->sName);
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
	for (size_t i=0;i<vPrimitives.size();++i)
	{
		vPrimitives.at(i)->SetProperty(NULL);
	}
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

void CSProperties::SetName(const string name) {sName=string(name);}
const string CSProperties::GetName() {return sName;}

bool CSProperties::ExistAttribute(string name)
{
	for (size_t n=0;n<m_Attribute_Name.size();++n)
	{
		if (m_Attribute_Name.at(n) == name)
			return true;
	}
	return false;
}

string CSProperties::GetAttributeValue(string name)
{
	for (size_t n=0;n<m_Attribute_Name.size();++n)
	{
		if (m_Attribute_Name.at(n) == name)
			return m_Attribute_Value.at(n);
	}
	return string();
}

void CSProperties::AddAttribute(string name, string value)
{
	if (name.empty()) return;
	m_Attribute_Name.push_back(name);
	m_Attribute_Value.push_back(value);
}

void CSProperties::AddPrimitive(CSPrimitives *prim) {vPrimitives.push_back(prim);}

size_t CSProperties::GetQtyPrimitives() {return vPrimitives.size();}
CSPrimitives* CSProperties::GetPrimitive(size_t index) {if (index<vPrimitives.size()) return vPrimitives.at(index); else return NULL;}
void CSProperties::SetFillColor(RGBa color) {FillColor.R=color.R;FillColor.G=color.G;FillColor.B=color.B;FillColor.a=color.a;}
RGBa CSProperties::GetFillColor() {return FillColor;}

RGBa CSProperties::GetEdgeColor() {return EdgeColor;}
void CSProperties::SetEdgeColor(RGBa color) {EdgeColor.R=color.R;EdgeColor.G=color.G;EdgeColor.B=color.B;EdgeColor.a=color.a;}

bool CSProperties::GetVisibility() {return bVisisble;}
void CSProperties::SetVisibility(bool val) {bVisisble=val;}

CSPropUnknown* CSProperties::ToUnknown() { return dynamic_cast<CSPropUnknown*>(this); }
CSPropMaterial* CSProperties::ToMaterial() { return dynamic_cast<CSPropMaterial*>(this); }
CSPropLorentzMaterial* CSProperties::ToLorentzMaterial() { return dynamic_cast<CSPropLorentzMaterial*>(this); }
CSPropMetal* CSProperties::ToMetal() { return dynamic_cast<CSPropMetal*>(this); }
CSPropElectrode* CSProperties::ToElectrode() { return dynamic_cast<CSPropElectrode*>(this); }
CSPropProbeBox* CSProperties::ToProbeBox() { return dynamic_cast<CSPropProbeBox*>(this); }
CSPropResBox* CSProperties::ToResBox() { return dynamic_cast<CSPropResBox*>(this); }
CSPropDumpBox* CSProperties::ToDumpBox() { return dynamic_cast<CSPropDumpBox*>(this); }

bool CSProperties::Update(string */*ErrStr*/) {return true;}

const string CSProperties::GetTypeString()
{
	switch (Type)
	{
		case CSProperties::UNKNOWN:
			sType=string("Unknown");
			break;
		case CSProperties::MATERIAL:
			sType=string("Material");
			break;
		case CSProperties::METAL:
			sType=string("Metal");
			break;
		case CSProperties::ELECTRODE:
			sType=string("Electrode");
			break;
		case CSProperties::PROBEBOX:
			sType=string("Probe-Box");
			break;
		case CSProperties::RESBOX:
			sType=string("Res-Box");
			break;
		case CSProperties::DUMPBOX:
			sType=string("Dump-Box");
			break;
		case CSProperties::ANY:
			sType=string("Any");
			break;
		default:
			sType=string("Invalid Type");
			break;
	};
	return sType;
}

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


void CSProperties::RemovePrimitive(CSPrimitives *prim)
{
	for (size_t i=0; i<vPrimitives.size();++i)
	{
		if (vPrimitives.at(i)==prim)
		{
				vector<CSPrimitives*>::iterator iter=vPrimitives.begin()+i;
				vPrimitives.erase(iter);
		}
	}
}

CSPrimitives* CSProperties::TakePrimitive(size_t index)
{
	if (index>=vPrimitives.size()) return NULL;
	CSPrimitives* prim=vPrimitives.at(index);
	vector<CSPrimitives*>::iterator iter=vPrimitives.begin()+index;
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

void CSProperties::WarnUnusedPrimitves(ostream& stream)
{
	if (vPrimitives.size()==0)
	{
		stream << "Warning: No primitives found in property: " << GetName() << "!" << endl;
		return;
	}
	for (size_t i=0; i<vPrimitives.size();++i)
	{
		if (vPrimitives.at(i)->GetPrimitiveUsed()==false)
		{
			stream << "Warning: Unused primitive (type: " << vPrimitives.at(i)->GetTypeName() << ") detected in property: " << GetName() << "!" << endl;
		}
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
	if (cHelp!=NULL) sName=string(cHelp);
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

	FillColor.a=255; //for the time being lock 2 this! take out later!

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


/*********************CSPropUnknown********************************************************************/
CSPropUnknown::CSPropUnknown(ParameterSet* paraSet) : CSProperties(paraSet) {Type=UNKNOWN;}
CSPropUnknown::CSPropUnknown(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=UNKNOWN;}
CSPropUnknown::CSPropUnknown(CSProperties* prop) : CSProperties(prop) {Type=UNKNOWN;}
CSPropUnknown::~CSPropUnknown() {}

void CSPropUnknown::SetProperty(const string val) {sUnknownProperty=string(val);}
const string CSPropUnknown::GetProperty() {return sUnknownProperty;}


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
		sUnknownProperty=string("unknown");
	else sUnknownProperty=string(chProp);
	return true;
}

/*********************CSPropMaterial********************************************************************/
CSPropMaterial::CSPropMaterial(ParameterSet* paraSet) : CSProperties(paraSet) {Type=MATERIAL;Init();}
CSPropMaterial::CSPropMaterial(CSProperties* prop) : CSProperties(prop) {Type=MATERIAL;Init();}
CSPropMaterial::CSPropMaterial(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=MATERIAL;Init();}
CSPropMaterial::~CSPropMaterial() {}

double CSPropMaterial::GetValue(ParameterScalar *ps, int ny)
{
	if (bIsotropy) ny=0;
	if ((ny>2) || (ny<0)) return 0;
	return ps[ny].GetValue();
}

string CSPropMaterial::GetTerm(ParameterScalar *ps, int ny)
{
	if (bIsotropy) ny=0;
	if ((ny>2) || (ny<0)) return 0;
	return ps[ny].GetString();
}

void CSPropMaterial::SetValue(double val, ParameterScalar *ps, int ny)
{
	if ((ny>2) || (ny<0)) return;
	ps[ny].SetValue(val);
}

int CSPropMaterial::SetValue(string val, ParameterScalar *ps, int ny)
{
	if ((ny>2) || (ny<0)) return 0;
	return ps[ny].SetValue(val);
}

double CSPropMaterial::GetWeight(ParameterScalar *ps, int ny, const double* coords)
{
	if (bIsotropy) ny=0;
	if ((ny>2) || (ny<0)) return 0;
	return GetWeight(ps[ny],coords);
}

double CSPropMaterial::GetWeight(ParameterScalar &ps, const double* coords)
{
	double paraVal[7];
	if (coordInputType==1)
	{
		double rho = coords[0];
		double alpha=coords[1];
		paraVal[0] = rho*cos(alpha);
		paraVal[1] = rho*sin(alpha);
		paraVal[2] = coords[2]; //z
		paraVal[3] = rho;
		paraVal[4] = sqrt(pow(rho,2)+pow(coords[2],2)); // r
		paraVal[5] = alpha; //alpha
		paraVal[6] = asin(1)-atan(coords[2]/rho); //theta
	}
	else
	{
		paraVal[0] = coords[0]; //x
		paraVal[1] = coords[1]; //y
		paraVal[2] = coords[2]; //z
		paraVal[3] = sqrt(pow(coords[0],2)+pow(coords[1],2));		//rho
		paraVal[4] = sqrt(pow(coords[0],2)+pow(coords[1],2)+pow(coords[2],2)); // r
		paraVal[5] = atan2(coords[1],coords[0]); //alpha
		paraVal[6] = asin(1)-atan(coords[2]/paraVal[3]); //theta
	}

	int EC=0;
	double value = ps.GetEvaluated(paraVal,EC);
	if (EC)
	{
		cerr << "CSPropMaterial::GetWeight: Error evaluating the weighting function (ID: " << this->GetID() << "): " << PSErrorCode2Msg(EC) << endl;
	}
	return value;
}

void CSPropMaterial::Init()
{
    bIsotropy = true;
	bMaterial=true;
    for (int n=0;n<3;++n)
    {
        Epsilon[n].SetValue(1);
        Epsilon[n].SetParameterSet(clParaSet);
        Mue[n].SetValue(1);
        Mue[n].SetParameterSet(clParaSet);
        Kappa[n].SetValue(0.0);
        Kappa[n].SetParameterSet(clParaSet);
        Sigma[n].SetValue(0.0);
        Sigma[n].SetParameterSet(clParaSet);
		WeightEpsilon[n].SetValue(1);
		WeightEpsilon[n].SetParameterSet(coordParaSet);
		WeightMue[n].SetValue(1);
		WeightMue[n].SetParameterSet(coordParaSet);
		WeightKappa[n].SetValue(1.0);
		WeightKappa[n].SetParameterSet(coordParaSet);
		WeightSigma[n].SetValue(1.0);
		WeightSigma[n].SetParameterSet(coordParaSet);
	}
}

bool CSPropMaterial::Update(string *ErrStr)
{
	bool bOK=true;
	int EC=0;
	for (int n=0;n<3;++n)
	{
		EC=Epsilon[n].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR) && (ErrStr!=NULL))
		{
			stringstream stream;
			stream << endl << "Error in Material-Property Epsilon-Value (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=Mue[n].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR) && (ErrStr!=NULL))
		{
			stringstream stream;
			stream << endl << "Error in Material-Property Mue-Value (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=Kappa[n].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR) && (ErrStr!=NULL))
		{
			stringstream stream;
			stream << endl << "Error in Material-Property Kappa-Value (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=Sigma[n].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR) && (ErrStr!=NULL))
		{
			stringstream stream;
			stream << endl << "Error in Material-Property Sigma-Value (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=WeightEpsilon[n].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR) && (ErrStr!=NULL))
		{
			stringstream stream;
			stream << endl << "Error in Material-Property Epsilon-Value weighting function (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=WeightMue[n].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR) && (ErrStr!=NULL))
		{
			stringstream stream;
			stream << endl << "Error in Material-Property Mue-Value weighting function  (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=WeightKappa[n].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR) && (ErrStr!=NULL))
		{
			stringstream stream;
			stream << endl << "Error in Material-Property Kappa-Value weighting function  (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=WeightSigma[n].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR) && (ErrStr!=NULL))
		{
			stringstream stream;
			stream << endl << "Error in Material-Property Sigma-Value weighting function  (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}
	return bOK;
}

bool CSPropMaterial::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSProperties::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	prop->SetAttribute("Isotropy",bIsotropy);

	TiXmlElement value("PropertyX");
	WriteTerm(Epsilon[0],value,"Epsilon",parameterised);
	WriteTerm(Mue[0],value,"Mue",parameterised);
	WriteTerm(Kappa[0],value,"Kappa",parameterised);
	WriteTerm(Sigma[0],value,"Sigma",parameterised);
	prop->InsertEndChild(value);

	value = TiXmlElement("PropertyY");
	WriteTerm(Epsilon[1],value,"Epsilon",parameterised);
	WriteTerm(Mue[1],value,"Mue",parameterised);
	WriteTerm(Kappa[1],value,"Kappa",parameterised);
	WriteTerm(Sigma[1],value,"Sigma",parameterised);
	prop->InsertEndChild(value);

	value = TiXmlElement("PropertyZ");
	WriteTerm(Epsilon[2],value,"Epsilon",parameterised);
	WriteTerm(Mue[2],value,"Mue",parameterised);
	WriteTerm(Kappa[2],value,"Kappa",parameterised);
	WriteTerm(Sigma[2],value,"Sigma",parameterised);
	prop->InsertEndChild(value);

	TiXmlElement WeightX("WeightX");
	WriteTerm(WeightEpsilon[0],WeightX,"Epsilon",parameterised);
	WriteTerm(WeightMue[0],WeightX,"Mue",parameterised);
	WriteTerm(WeightKappa[0],WeightX,"Kappa",parameterised);
	WriteTerm(WeightSigma[0],WeightX,"Sigma",parameterised);
	prop->InsertEndChild(WeightX);

	TiXmlElement WeightY("WeightY");
	WriteTerm(WeightEpsilon[1],WeightY,"Epsilon",parameterised);
	WriteTerm(WeightMue[1],WeightY,"Mue",parameterised);
	WriteTerm(WeightKappa[1],WeightY,"Kappa",parameterised);
	WriteTerm(WeightSigma[1],WeightY,"Sigma",parameterised);
	prop->InsertEndChild(WeightY);

	TiXmlElement WeightZ("WeightZ");
	WriteTerm(WeightEpsilon[2],WeightZ,"Epsilon",parameterised);
	WriteTerm(WeightMue[2],WeightZ,"Mue",parameterised);
	WriteTerm(WeightKappa[2],WeightZ,"Kappa",parameterised);
	WriteTerm(WeightSigma[2],WeightZ,"Sigma",parameterised);
	prop->InsertEndChild(WeightZ);

	return true;
}

bool CSPropMaterial::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;
	TiXmlElement* prop=root.ToElement();

	if (prop==NULL) return false;

	int attr=1;
	prop->QueryIntAttribute("Isotropy",&attr);
	bIsotropy = attr>0;

	TiXmlElement* matProp=prop->FirstChildElement("PropertyX");
	if (matProp==NULL) //if 0 try also the old style...
		matProp=prop->FirstChildElement("Property");
	if (matProp!=NULL)
	{
		ReadTerm(Epsilon[0],*matProp,"Epsilon",1.0);
		ReadTerm(Mue[0],*matProp,"Mue",1.0);
		ReadTerm(Kappa[0],*matProp,"Kappa");
		ReadTerm(Sigma[0],*matProp,"Sigma");
	}

	matProp=prop->FirstChildElement("PropertyY");
	if (matProp!=NULL) //always accept do to legacy support
	{
		ReadTerm(Epsilon[1],*matProp,"Epsilon",1.0);
		ReadTerm(Mue[1],*matProp,"Mue",1.0);
		ReadTerm(Kappa[1],*matProp,"Kappa");
		ReadTerm(Sigma[1],*matProp,"Sigma");
	}
	matProp=prop->FirstChildElement("PropertyZ");
	if (matProp!=NULL) //always accept do to legacy support
	{
		ReadTerm(Epsilon[2],*matProp,"Epsilon",1.0);
		ReadTerm(Mue[2],*matProp,"Mue",1.0);
		ReadTerm(Kappa[2],*matProp,"Kappa");
		ReadTerm(Sigma[2],*matProp,"Sigma");
	}

	TiXmlElement *weight = prop->FirstChildElement("WeightX");
	if (weight!=NULL)
	{
		ReadTerm(WeightEpsilon[0],*weight,"Epsilon",1.0);
		ReadTerm(WeightMue[0],*weight,"Mue",1.0);
		ReadTerm(WeightKappa[0],*weight,"Kappa",1.0);
		ReadTerm(WeightSigma[0],*weight,"Sigma",1.0);
	}
	weight = prop->FirstChildElement("WeightY");
	if (weight!=NULL)
	{
		ReadTerm(WeightEpsilon[1],*weight,"Epsilon",1.0);
		ReadTerm(WeightMue[1],*weight,"Mue",1.0);
		ReadTerm(WeightKappa[1],*weight,"Kappa",1.0);
		ReadTerm(WeightSigma[1],*weight,"Sigma",1.0);
	}
	weight = prop->FirstChildElement("WeightZ");
	if (weight!=NULL)
	{
		ReadTerm(WeightEpsilon[2],*weight,"Epsilon",1.0);
		ReadTerm(WeightMue[2],*weight,"Mue",1.0);
		ReadTerm(WeightKappa[2],*weight,"Kappa",1.0);
		ReadTerm(WeightSigma[2],*weight,"Sigma",1.0);
	}

	return true;
}

/*********************CSPropDispersiveMaterial********************************************************/
CSPropDispersiveMaterial::CSPropDispersiveMaterial(ParameterSet* paraSet) : CSPropMaterial(paraSet) {Type=(CSProperties::PropertyType)(DISPERSIVEMATERIAL | MATERIAL);}
CSPropDispersiveMaterial::CSPropDispersiveMaterial(CSProperties* prop) : CSPropMaterial(prop) {Type=(CSProperties::PropertyType)(DISPERSIVEMATERIAL | MATERIAL);}
CSPropDispersiveMaterial::CSPropDispersiveMaterial(unsigned int ID, ParameterSet* paraSet) : CSPropMaterial(ID,paraSet) {Type=(CSProperties::PropertyType)(DISPERSIVEMATERIAL | MATERIAL);}
CSPropDispersiveMaterial::~CSPropDispersiveMaterial() {}

bool CSPropDispersiveMaterial::Update(string *ErrStr)
{
	return CSPropMaterial::Update(ErrStr);
}

bool CSPropDispersiveMaterial::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	return CSPropMaterial::Write2XML(root,parameterised,sparse);
}

bool CSPropDispersiveMaterial::ReadFromXML(TiXmlNode &root)
{
	return CSPropMaterial::ReadFromXML(root);
}

/*********************CSPropLorentzMaterial********************************************************/
CSPropLorentzMaterial::CSPropLorentzMaterial(ParameterSet* paraSet) : CSPropDispersiveMaterial(paraSet) {Type=(CSProperties::PropertyType)(LORENTZMATERIAL | DISPERSIVEMATERIAL | MATERIAL);Init();}
CSPropLorentzMaterial::CSPropLorentzMaterial(CSProperties* prop) : CSPropDispersiveMaterial(prop) {Type=(CSProperties::PropertyType)(LORENTZMATERIAL | DISPERSIVEMATERIAL | MATERIAL);Init();}
CSPropLorentzMaterial::CSPropLorentzMaterial(unsigned int ID, ParameterSet* paraSet) : CSPropDispersiveMaterial(ID,paraSet) {Type=(CSProperties::PropertyType)(LORENTZMATERIAL | DISPERSIVEMATERIAL | MATERIAL);Init();}
CSPropLorentzMaterial::~CSPropLorentzMaterial() {}

void CSPropLorentzMaterial::Init()
{
	for (int n=0;n<3;++n)
	{
		EpsPlasma[n].SetValue(0);
		EpsPlasma[n].SetParameterSet(clParaSet);
		MuePlasma[n].SetValue(0);
		MuePlasma[n].SetParameterSet(clParaSet);
		WeightEpsPlasma[n].SetValue(1);
		WeightEpsPlasma[n].SetParameterSet(coordParaSet);
		WeightMuePlasma[n].SetValue(1);
		WeightMuePlasma[n].SetParameterSet(coordParaSet);
	}
	CSPropDispersiveMaterial::Init();
}


bool CSPropLorentzMaterial::Update(string *ErrStr)
{
	bool bOK=true;
	int EC=0;
	for (int n=0;n<3;++n)
	{
		EC=EpsPlasma[n].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR) && (ErrStr!=NULL))
		{
			stringstream stream;
			stream << endl << "Error in Lorentz Material-Property epsilon plasma frequency value (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=MuePlasma[n].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR) && (ErrStr!=NULL))
		{
			stringstream stream;
			stream << endl << "Error in Lorentz Material-Property mue plasma frequency value (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}

		EC=WeightEpsPlasma[n].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR) && (ErrStr!=NULL))
		{
			stringstream stream;
			stream << endl << "Error in Lorentz Material-Property epsilon plasma frequency weighting function (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=WeightMuePlasma[n].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR) && (ErrStr!=NULL))
		{
			stringstream stream;
			stream << endl << "Error in Lorentz Material-Property mue plasma frequency value weighting function (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}
	return CSPropDispersiveMaterial::Update(ErrStr);
}

bool CSPropLorentzMaterial::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSPropDispersiveMaterial::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	string dirName[3] = {"X","Y","Z"};

	for (int n=0;n<3;++n)
	{
		string name = "PlasmaFrequency" + dirName[n];
		TiXmlElement value(name.c_str());
		WriteTerm(EpsPlasma[n],value,"Epsilon",parameterised);
		WriteTerm(MuePlasma[n],value,"Mue",parameterised);
		WriteTerm(WeightEpsPlasma[n],value,"WeightEpsilon",parameterised);
		WriteTerm(WeightMuePlasma[n],value,"WeighMue",parameterised);
		prop->InsertEndChild(value);
	}
	return true;
}

bool CSPropLorentzMaterial::ReadFromXML(TiXmlNode &root)
{
	if (CSPropDispersiveMaterial::ReadFromXML(root)==false) return false;
	TiXmlElement* prop=root.ToElement();

	if (prop==NULL) return false;

	string dirName[3] = {"X","Y","Z"};

	for (int n=0;n<3;++n)
	{
		string name = "PlasmaFrequency" + dirName[n];
		TiXmlElement* matProp=prop->FirstChildElement(name.c_str());
		if (matProp!=NULL)
		{
			ReadTerm(EpsPlasma[n],*matProp,"Epsilon",1.0);
			ReadTerm(MuePlasma[n],*matProp,"Mue",1.0);
			ReadTerm(WeightEpsPlasma[n],*matProp,"WeightEpsilon");
			ReadTerm(WeightMuePlasma[n],*matProp,"WeightMue");
		}
	}
	return true;
}

/*********************CSPropMetal********************************************************************/
CSPropMetal::CSPropMetal(ParameterSet* paraSet) : CSProperties(paraSet) {Type=METAL;bMaterial=true;}
CSPropMetal::CSPropMetal(CSProperties* prop) : CSProperties(prop) {Type=METAL;bMaterial=true;}
CSPropMetal::CSPropMetal(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=METAL;bMaterial=true;}
CSPropMetal::~CSPropMetal() {}

bool CSPropMetal::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSProperties::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	return true;
}

bool CSPropMetal::ReadFromXML(TiXmlNode &root)
{
	return CSProperties::ReadFromXML(root);
}

/*********************CSPropElectrode********************************************************************/
CSPropElectrode::CSPropElectrode(ParameterSet* paraSet,unsigned int number) : CSProperties(paraSet) {Type=ELECTRODE;Init();uiNumber=number;}
CSPropElectrode::CSPropElectrode(CSProperties* prop) : CSProperties(prop) {Type=ELECTRODE;Init();}
CSPropElectrode::CSPropElectrode(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=ELECTRODE;Init();}
CSPropElectrode::~CSPropElectrode() {}

void CSPropElectrode::SetNumber(unsigned int val) {uiNumber=val;}
unsigned int CSPropElectrode::GetNumber() {return uiNumber;}

void CSPropElectrode::SetExcitType(int val) {iExcitType=val;}
int CSPropElectrode::GetExcitType() {return iExcitType;}

void CSPropElectrode::SetExcitation(double val, int Component)
{
	if ((Component<0) || (Component>=3)) return;
	Excitation[Component].SetValue(val);
}

void CSPropElectrode::SetExcitation(const string val, int Component)
{
	if ((Component<0) || (Component>=3)) return;
	Excitation[Component].SetValue(val);
}

double CSPropElectrode::GetExcitation(int Component)
{
	if ((Component<0) || (Component>=3)) return 0;
	return Excitation[Component].GetValue();
}

const string CSPropElectrode::GetExcitationString(int Comp)
{
	if ((Comp<0) || (Comp>=3)) return NULL;
	return Excitation[Comp].GetString();
}

void CSPropElectrode::SetActiveDir(bool active, int Component)
{
	if ((Component<0) || (Component>=3)) return;
	ActiveDir[Component]=active;
}

bool CSPropElectrode::GetActiveDir(int Component)
{
	if ((Component<0) || (Component>=3)) return false;
	return ActiveDir[Component];
}

int CSPropElectrode::SetWeightFunction(const string fct, int ny)
{
	if ((ny>=0) && (ny<3))
		return WeightFct[ny].SetValue(fct);
	return 0;
}

const string CSPropElectrode::GetWeightFunction(int ny) {if ((ny>=0) && (ny<3)) {return WeightFct[ny].GetString();} else return string();}

double CSPropElectrode::GetWeightedExcitation(int ny, const double* coords)
{
	if ((ny<0) || (ny>=3)) return 0;
	//Warning: this is not reentrant....!!!!
	double loc_coords[3] = {coords[0],coords[1],coords[2]};
	double r,rho,alpha,theta;
	if (coordInputType==1)
	{
		loc_coords[0] = coords[0]*cos(coords[1]);
		loc_coords[1] = coords[0]*sin(coords[1]);
		rho = coords[0];
		alpha=coords[1];
		r = sqrt(pow(coords[0],2)+pow(coords[2],2));
		theta=asin(1)-atan(coords[2]/rho);
	}
	else
	{
		alpha=atan2(coords[1],coords[0]);
		rho = sqrt(pow(coords[0],2)+pow(coords[1],2));
		r = sqrt(pow(coords[0],2)+pow(coords[1],2)+pow(coords[2],2));
		theta=asin(1)-atan(coords[2]/rho);
	}
	coordPara[0]->SetValue(loc_coords[0]);
	coordPara[1]->SetValue(loc_coords[1]);
	coordPara[2]->SetValue(loc_coords[2]);
	coordPara[3]->SetValue(rho); //rho
	coordPara[4]->SetValue(r); //r
	coordPara[5]->SetValue(alpha);
	coordPara[6]->SetValue(theta); //theta
	int EC = WeightFct[ny].Evaluate();
	if (EC)
	{
		cerr << "CSPropElectrode::GetWeightedExcitation: Error evaluating the weighting function (ID: " << this->GetID() << ", n=" << ny << "): " << PSErrorCode2Msg(EC) << endl;
	}

	return WeightFct[ny].GetValue()*GetExcitation(ny);
}

void CSPropElectrode::SetDelay(double val)	{Delay.SetValue(val);}

void CSPropElectrode::SetDelay(const string val) {Delay.SetValue(val);}

double CSPropElectrode::GetDelay(){return Delay.GetValue();}

const string CSPropElectrode::GetDelayString(){return Delay.GetString();}

void CSPropElectrode::Init()
{
	uiNumber=0;
	iExcitType=1;
	coordInputType=UNDEFINED_CS;
	for (unsigned int i=0;i<3;++i)
	{
		ActiveDir[i]=true;
		Excitation[i].SetValue(0.0);
		Excitation[i].SetParameterSet(clParaSet);
		WeightFct[i].SetValue(1.0);
		WeightFct[i].SetParameterSet(coordParaSet);
		Delay.SetValue(0.0);
		Delay.SetParameterSet(clParaSet);
	}
}

bool CSPropElectrode::Update(string *ErrStr)
{
	bool bOK=true;
	int EC=0;
	for (unsigned int i=0;i<3;++i)
	{
		EC=Excitation[i].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
		{
			stringstream stream;
			stream << endl << "Error in Electrode-Property Excitaion-Value (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
			//cout << EC << endl;
		}
	}
	EC=Delay.Evaluate();
	if (EC!=ParameterScalar::NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
	{
		stringstream stream;
		stream << endl << "Error in Electrode-Property Delay-Value";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
		//cout << EC << endl;
	}
	return bOK;
}

bool CSPropElectrode::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSProperties::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	prop->SetAttribute("Number",(int)uiNumber);
	WriteTerm(Delay,*prop,"Delay",parameterised);

	TiXmlElement Excit("Excitation");
	Excit.SetAttribute("Type",iExcitType);
	WriteTerm(Excitation[0],Excit,"Excit_X",parameterised);
	WriteTerm(Excitation[1],Excit,"Excit_Y",parameterised);
	WriteTerm(Excitation[2],Excit,"Excit_Z",parameterised);
	prop->InsertEndChild(Excit);

	TiXmlElement Weight("Weight");
	WriteTerm(WeightFct[0],Weight,"X",parameterised);
	WriteTerm(WeightFct[1],Weight,"Y",parameterised);
	WriteTerm(WeightFct[2],Weight,"Z",parameterised);
	prop->InsertEndChild(Weight);

	return true;
}

bool CSPropElectrode::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;

	TiXmlElement *prop = root.ToElement();
	if (prop==NULL) return false;

	int iHelp;
	if (prop->QueryIntAttribute("Number",&iHelp)!=TIXML_SUCCESS) uiNumber=0;
	else uiNumber=(unsigned int)iHelp;

	ReadTerm(Delay,*prop,"Delay");

	TiXmlElement *excit = prop->FirstChildElement("Excitation");
	if (excit==NULL) return false;
	if (excit->QueryIntAttribute("Type",&iExcitType)!=TIXML_SUCCESS) return false;
	if (ReadTerm(Excitation[0],*excit,"Excit_X")==false) return false;
	if (ReadTerm(Excitation[1],*excit,"Excit_Y")==false) return false;
	if (ReadTerm(Excitation[2],*excit,"Excit_Z")==false) return false;

	TiXmlElement *weight = prop->FirstChildElement("Weight");
	if (weight!=NULL)
	{
		ReadTerm(WeightFct[0],*weight,"X");
		ReadTerm(WeightFct[1],*weight,"Y");
		ReadTerm(WeightFct[2],*weight,"Z");
	}

	return true;
}

/*********************CSPropProbeBox********************************************************************/
CSPropProbeBox::CSPropProbeBox(ParameterSet* paraSet) : CSProperties(paraSet) {Type=PROBEBOX;uiNumber=0;ProbeType=0;m_weight=1;}
CSPropProbeBox::CSPropProbeBox(CSProperties* prop) : CSProperties(prop) {Type=PROBEBOX;uiNumber=0;ProbeType=0;m_weight=1;}
CSPropProbeBox::CSPropProbeBox(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=PROBEBOX;uiNumber=0;ProbeType=0;m_weight=1;}
CSPropProbeBox::~CSPropProbeBox() {}

void CSPropProbeBox::SetNumber(unsigned int val) {uiNumber=val;}
unsigned int CSPropProbeBox::GetNumber() {return uiNumber;}

void CSPropProbeBox::AddFDSample(vector<double> *freqs)
{
	for (size_t n=0;n<freqs->size();++n)
		AddFDSample(freqs->at(n));
}

void CSPropProbeBox::AddFDSample(string freqs)
{
	vector<double> v_freqs = SplitString2Double(freqs, ',');
	AddFDSample(&v_freqs);
}

bool CSPropProbeBox::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSProperties::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	prop->SetAttribute("Number",(int)uiNumber);
	prop->SetAttribute("Type",ProbeType);
	prop->SetAttribute("Weight",m_weight);

	if (m_FD_Samples.size())
	{
		string fdSamples = CombineVector2String(m_FD_Samples,',');

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

	if (prop->QueryIntAttribute("Type",&ProbeType)!=TIXML_SUCCESS) ProbeType=0;

	if (prop->QueryDoubleAttribute("Weight",&m_weight)!=TIXML_SUCCESS) m_weight=1;

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

/*********************CSPropResBox********************************************************************/
CSPropResBox::CSPropResBox(ParameterSet* paraSet) : CSProperties(paraSet) {Type=RESBOX;uiFactor=1;} ;
CSPropResBox::CSPropResBox(CSProperties* prop) : CSProperties(prop) {Type=RESBOX;uiFactor=1;} ;
CSPropResBox::CSPropResBox(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=RESBOX;uiFactor=1;} ;
CSPropResBox::~CSPropResBox() {};

void CSPropResBox::SetResFactor(unsigned int val)  {uiFactor=val;}
unsigned int CSPropResBox::GetResFactor()  {return uiFactor;}

bool CSPropResBox::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSProperties::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	prop->SetAttribute("Factor",(int)uiFactor);

	return true;
}

bool CSPropResBox::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;

	TiXmlElement *prop = root.ToElement();
	if (prop==NULL) return false;

	int iHelp;
	if (prop->QueryIntAttribute("Factor",&iHelp)!=TIXML_SUCCESS) uiFactor=1;
	else uiFactor=(unsigned int)iHelp;

	return true;
}

/*********************CSPropDumpBox********************************************************************/
CSPropDumpBox::CSPropDumpBox(ParameterSet* paraSet) : CSProperties(paraSet) {Type=DUMPBOX;Init();}
CSPropDumpBox::CSPropDumpBox(CSProperties* prop) : CSProperties(prop) {Type=DUMPBOX;Init();}
CSPropDumpBox::CSPropDumpBox(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=DUMPBOX;Init();}
CSPropDumpBox::~CSPropDumpBox() {}

bool CSPropDumpBox::GetGlobalSetting() {return GlobalSetting;}
bool CSPropDumpBox::GetPhiDump() {return PhiDump;}
bool CSPropDumpBox::GetDivEDump() {return divE;}
bool CSPropDumpBox::GetDivDDump() {return divD;}
bool CSPropDumpBox::GetDivPDump() {return divP;}
bool CSPropDumpBox::GetFieldWDump() {return FieldW;}
bool CSPropDumpBox::GetChargeWDump() {return ChargeW;}
bool CSPropDumpBox::GetEFieldDump() {return EField;}
bool CSPropDumpBox::GetDFieldDump() {return DField;}
bool CSPropDumpBox::GetPFieldDump() {return PField;}
bool CSPropDumpBox::GetSGDump() {return SGDump;}
bool CSPropDumpBox::GetSimpleDump() {return SimpleDump;}
int CSPropDumpBox::GetSGLevel() {return SGLevel;}

void CSPropDumpBox::SetGlobalSetting(bool val) {GlobalSetting=val;}
void CSPropDumpBox::SetPhiDump(bool val) {PhiDump=val;}
void CSPropDumpBox::SetDivEDump(bool val) {divE=val;}
void CSPropDumpBox::SetDivDDump(bool val) {divD=val;}
void CSPropDumpBox::SetDivPDump(bool val) {divP=val;}
void CSPropDumpBox::SetFieldWDump(bool val) {FieldW=val;}
void CSPropDumpBox::SetChargeWDump(bool val) {ChargeW=val;}
void CSPropDumpBox::SetEFieldDump(bool val) {EField=val;}
void CSPropDumpBox::SetDFieldDump(bool val) {DField=val;}
void CSPropDumpBox::SetPFieldDump(bool val) {PField=val;}
void CSPropDumpBox::SetSGDump(bool val) {SGDump=val;}
void CSPropDumpBox::SetSimpleDump(bool val) {SimpleDump=val;}
void CSPropDumpBox::SetSGLevel(int val) {SGLevel=val;}

void CSPropDumpBox::Init()
{
	GlobalSetting=true;
	PhiDump=true;
	divE=divD=divP=FieldW=ChargeW=false;
	EField=DField=PField=false;
	SGDump=SimpleDump=false;
	SGLevel=-1;
	DumpType = 0;
	DumpMode = 0;
	FileType = 0;
	SubSampling[0]=1;
	SubSampling[1]=1;
	SubSampling[2]=1;
}

void CSPropDumpBox::SetSubSampling(int ny, unsigned int val)
{
	if ((ny<0) || (ny>2)) return;
	if (val<1) return;
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
	vector<int> values = SplitString2Int(string(vals),',');
	for (int ny=0;ny<3 && ny<(int)values.size();++ny)
		SetSubSampling(ny,values.at(ny));
}

unsigned int CSPropDumpBox::GetSubSampling(int ny)
{
	if ((ny<0) || (ny>2)) return 1;
	return SubSampling[ny];
}

bool CSPropDumpBox::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSProperties::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	prop->SetAttribute("DumpType",DumpType);
	prop->SetAttribute("DumpMode",DumpMode);
	prop->SetAttribute("FileType",FileType);

	stringstream ss;
	ss << GetSubSampling(0) << "," << GetSubSampling(1) << "," << GetSubSampling(2) ;
	prop->SetAttribute("SubSampling",ss.str().c_str());

	if (!sparse)
	{
		prop->SetAttribute("GlobalSetting",(int)GlobalSetting);
		TiXmlElement ScalDump("ScalarDump");
		ScalDump.SetAttribute("DumpPhi",(int)PhiDump);
		ScalDump.SetAttribute("DumpDivE",(int)divE);
		ScalDump.SetAttribute("DumpDivD",(int)divD);
		ScalDump.SetAttribute("DumpDivP",(int)divP);
		ScalDump.SetAttribute("DumpFieldW",(int)FieldW);
		ScalDump.SetAttribute("DumpChargeW",(int)ChargeW);
		prop->InsertEndChild(ScalDump);

		TiXmlElement VecDump("VectorDump");
		VecDump.SetAttribute("DumpEField",(int)EField);
		VecDump.SetAttribute("DumpDField",(int)DField);
		VecDump.SetAttribute("DumpPField",(int)PField);
		prop->InsertEndChild(VecDump);

		TiXmlElement SubGDump("SubGridDump");
		SubGDump.SetAttribute("SubGridDump",(int)SGDump);
		SubGDump.SetAttribute("SimpleDump",(int)SimpleDump);
		SubGDump.SetAttribute("SubGridLevel",SGLevel);
		prop->InsertEndChild(SubGDump);
	}

	return true;
}

bool CSPropDumpBox::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;

	TiXmlElement *prop = root.ToElement();
	if (prop==NULL) return false;

	int iHelp;
	if (prop->QueryIntAttribute("GlobalSetting",&iHelp)!=TIXML_SUCCESS) GlobalSetting=0;
	else GlobalSetting=(bool)iHelp;

	if (prop->QueryIntAttribute("DumpType",&DumpType)!=TIXML_SUCCESS) DumpType=0;
	if (prop->QueryIntAttribute("DumpMode",&DumpMode)!=TIXML_SUCCESS) DumpMode=0;
	if (prop->QueryIntAttribute("FileType",&FileType)!=TIXML_SUCCESS) FileType=0;

	SetSubSampling(prop->Attribute("SubSampling"));

	TiXmlElement *ScalDump = prop->FirstChildElement("ScalarDump");
	if (ScalDump!=NULL)
	{
		if (ScalDump->QueryIntAttribute("DumpPhi",&iHelp)==TIXML_SUCCESS)
			PhiDump=(bool)iHelp;
		if (ScalDump->QueryIntAttribute("DumpDivE",&iHelp)==TIXML_SUCCESS)
			divE=(bool)iHelp;
		if (ScalDump->QueryIntAttribute("DumpDivD",&iHelp)==TIXML_SUCCESS)
			divD=(bool)iHelp;
		if (ScalDump->QueryIntAttribute("DumpDivP",&iHelp)==TIXML_SUCCESS)
			divP=(bool)iHelp;
		if (ScalDump->QueryIntAttribute("DumpFieldW",&iHelp)==TIXML_SUCCESS)
			FieldW=(bool)iHelp;
		if (ScalDump->QueryIntAttribute("DumpChargeW",&iHelp)==TIXML_SUCCESS)
			ChargeW=(bool)iHelp;
	}
	TiXmlElement *VecDump = prop->FirstChildElement("VectorDump");
	if (VecDump!=NULL)
	{

		if (VecDump->QueryIntAttribute("DumpEField",&iHelp)==TIXML_SUCCESS)
			EField=(bool)iHelp;
		if (VecDump->QueryIntAttribute("DumpDField",&iHelp)==TIXML_SUCCESS)
			DField=(bool)iHelp;
		if (VecDump->QueryIntAttribute("DumpPField",&iHelp)==TIXML_SUCCESS)
			PField=(bool)iHelp;
	}
	TiXmlElement *SubGDump = prop->FirstChildElement("SubGridDump");
	if (SubGDump!=NULL)
	{

		if (SubGDump->QueryIntAttribute("SubGridDump",&iHelp)==TIXML_SUCCESS)
			SGDump=(bool)iHelp;
		if (SubGDump->QueryIntAttribute("SimpleDump",&iHelp)==TIXML_SUCCESS)
			SimpleDump=(bool)iHelp;
		if (SubGDump->QueryIntAttribute("SubGridLevel",&iHelp)==TIXML_SUCCESS)
			SGLevel=iHelp;
	}
	return true;
}
