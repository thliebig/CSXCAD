#include "CSProperties.h"
#include "CSPrimitives.h"
#include "ParameterObjects.h"
//#include <iostream>
#include <sstream>
#include "tinyxml.h"

/*********************CSProperties********************************************************************/
CSProperties::CSProperties(CSProperties* prop)
{
	uiID=prop->uiID;
	clParaSet=prop->clParaSet;
	FillColor=prop->FillColor;
	EdgeColor=prop->EdgeColor;
	sName=string(prop->sName);
	for (size_t i=0;i<prop->vPrimitives.size();++i)
	{
		vPrimitives.push_back(prop->vPrimitives.at(i));
	}
}

CSProperties::CSProperties(ParameterSet* paraSet)
{
	uiID=0;
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
}
CSProperties::CSProperties(unsigned int ID, ParameterSet* paraSet)
{
	uiID=ID;
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
}


CSProperties::~CSProperties()
{
	for (size_t i=0;i<vPrimitives.size();++i)
	{
		vPrimitives.at(i)->SetProperty(NULL);
	}
}

int CSProperties::GetType() {return Type;}

unsigned int CSProperties::GetID() {return uiID;}
void CSProperties::SetID(unsigned int ID) {uiID=ID;}

unsigned int CSProperties::GetUniqueID() {return UniqueID;}
void CSProperties::SetUniqueID(unsigned int uID) {UniqueID=uID;}

void CSProperties::SetName(const string name) {sName=string(name);}
const string CSProperties::GetName() {return sName;}

void CSProperties::AddPrimitive(CSPrimitives *prim) {vPrimitives.push_back(prim);}

size_t CSProperties::GetQtyPrimitives() {return vPrimitives.size();}
CSPrimitives* CSProperties::GetPrimitive(size_t index) {if ((index>=0) && (index<vPrimitives.size())) return vPrimitives.at(index); else return NULL;}
void CSProperties::SetFillColor(RGBa color) {FillColor.R=color.R;FillColor.G=color.G;FillColor.B=color.B;FillColor.a=color.a;}
RGBa CSProperties::GetFillColor() {return FillColor;}

RGBa CSProperties::GetEdgeColor() {return EdgeColor;}
void CSProperties::SetEdgeColor(RGBa color) {EdgeColor.R=color.R;EdgeColor.G=color.G;EdgeColor.B=color.B;EdgeColor.a=color.a;}

bool CSProperties::GetVisibility() {return bVisisble;}
void CSProperties::SetVisibility(bool val) {bVisisble=val;}

CSPropUnknown* CSProperties::ToUnknown() { return ( this && Type == UNKNOWN ) ? (CSPropUnknown*) this : 0; }
CSPropMaterial* CSProperties::ToMaterial() { return ( this && Type == MATERIAL ) ? (CSPropMaterial*) this : 0; }
CSPropMetal* CSProperties::ToMetal() { return ( this && Type == METAL ) ? (CSPropMetal*) this : 0; }
CSPropElectrode* CSProperties::ToElectrode() { return ( this && Type == ELECTRODE ) ? (CSPropElectrode*) this : 0; }
CSPropChargeBox* CSProperties::ToChargeBox() { return ( this && Type == CHARGEBOX ) ? (CSPropChargeBox*) this : 0; }
CSPropResBox* CSProperties::ToResBox() { return ( this && Type == RESBOX ) ? (CSPropResBox*) this : 0; }
CSPropDumpBox* CSProperties::ToDumpBox() { return ( this && Type == DUMPBOX ) ? (CSPropDumpBox*) this : 0; }

bool CSProperties::Update(string *ErrStr) {return true;}

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
		case CSProperties::CHARGEBOX:
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
	return sType.c_str();
}

bool CSProperties::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement* prop=root.ToElement();
	prop->SetAttribute("ID",uiID);
	prop->SetAttribute("Name",sName.c_str());
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

bool CSProperties::CheckCoordInPrimitive(double *coord, int &priority, double tol)
{
	priority=0;
	bool found=false;
	for (size_t i=0; i<vPrimitives.size();++i)
	{
		if (vPrimitives.at(i)->IsInside(coord,tol)==true)
		{
			if (found==false) priority=vPrimitives.at(i)->GetPriority()-1;
			found=true;
			if (vPrimitives.at(i)->GetPriority()>priority) priority=vPrimitives.at(i)->GetPriority();
		}
	}
	return found;
}

bool CSProperties::ReadFromXML(TiXmlNode &root)
{
	TiXmlElement* prop = root.ToElement();
	if (prop==NULL) return false;

	int help;
	if (prop->QueryIntAttribute("ID",&help)!=TIXML_SUCCESS) return false;
	uiID=help;

	const char* cHelp=prop->Attribute("Name");
	if (cHelp!=NULL) sName=string(cHelp);
	else sName.clear();

	TiXmlElement* FC = root.FirstChildElement("FillColor");
	if (FC==NULL) return false;

	if (FC->QueryIntAttribute("R",&help)!=TIXML_SUCCESS) return false;
	FillColor.R=(unsigned char) help;
	if (FC->QueryIntAttribute("G",&help)!=TIXML_SUCCESS) return false;
	FillColor.G=(unsigned char) help;
	if (FC->QueryIntAttribute("B",&help)!=TIXML_SUCCESS) return false;
	FillColor.B=(unsigned char) help;
	if (FC->QueryIntAttribute("a",&help)!=TIXML_SUCCESS) return false;
	FillColor.a=(unsigned char) help;

	FillColor.a=255; //for the time being lock 2 this! take out later!

	TiXmlElement* EC = root.FirstChildElement("EdgeColor");
	if (EC==NULL) return false;
	if (EC->QueryIntAttribute("R",&help)!=TIXML_SUCCESS) return false;
	EdgeColor.R=(unsigned char) help;
	if (EC->QueryIntAttribute("G",&help)!=TIXML_SUCCESS) return false;
	EdgeColor.G=(unsigned char) help;
	if (EC->QueryIntAttribute("B",&help)!=TIXML_SUCCESS) return false;
	EdgeColor.B=(unsigned char) help;
	if (EC->QueryIntAttribute("a",&help)!=TIXML_SUCCESS) return false;
	EdgeColor.a=(unsigned char) help;

	return true;
}


/*********************CSPropUnknown********************************************************************/
CSPropUnknown::CSPropUnknown(ParameterSet* paraSet) : CSProperties(paraSet) {Type=UNKNOWN;}
CSPropUnknown::CSPropUnknown(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=UNKNOWN;}
CSPropUnknown::CSPropUnknown(CSProperties* prop) : CSProperties(prop) {Type=UNKNOWN;}
CSPropUnknown::~CSPropUnknown() {}

void CSPropUnknown::SetProperty(const string val) {sUnknownProperty=string(val);}
const string CSPropUnknown::GetProperty() {return sUnknownProperty;}


bool CSPropUnknown::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement prop("Unknown");
	CSProperties::Write2XML(prop,parameterised);
	prop.SetAttribute("Property",sUnknownProperty.c_str());

	root.InsertEndChild(prop);
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

void CSPropMaterial::SetEpsilon(double val) {Epsilon.SetValue(val);}
void CSPropMaterial::SetEpsilon(const string val) {Epsilon.SetValue(val);}
double CSPropMaterial::GetEpsilon() {return Epsilon.GetValue();}
const string CSPropMaterial::GetEpsilonTerm() {return Epsilon.GetString();}

void CSPropMaterial::SetMue(double val) {Mue.SetValue(val);}
void CSPropMaterial::SetMue(const string val) {Mue.SetValue(val);}
double CSPropMaterial::GetMue() {return Mue.GetValue();}
const string CSPropMaterial::GetMueTerm() {return Mue.GetString();}

void CSPropMaterial::SetKappa(double val) {Kappa.SetValue(val);}
void CSPropMaterial::SetKappa(const string val) {Kappa.SetValue(val);}
double CSPropMaterial::GetKappa() {return Kappa.GetValue();}
const string CSPropMaterial::GetKappaTerm() {return Kappa.GetString();}

void CSPropMaterial::Init()
{
	Epsilon.SetValue(1);
	Epsilon.SetParameterSet(clParaSet);
	Mue.SetValue(1);
	Mue.SetParameterSet(clParaSet);
	Kappa.SetValue(0.0);
	Kappa.SetParameterSet(clParaSet);
}

bool CSPropMaterial::Update(string *ErrStr)
{
	bool bOK=true;
	int EC=0;
	EC=Epsilon.Evaluate();
	if (EC!=ParameterScalar::NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
	{
		stringstream stream;
		stream << endl << "Error in Material-Property Epsilon-Value (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}
	EC=Mue.Evaluate();
	if (EC!=ParameterScalar::NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
	{
		stringstream stream;
		stream << endl << "Error in Material-Property Mue-Value (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}
	EC=Kappa.Evaluate();
	if (EC!=ParameterScalar::NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
	{
		stringstream stream;
		stream << endl << "Error in Material-Property Kappa-Value (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}
	return bOK;
}

bool CSPropMaterial::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement prop("Material");
	CSProperties::Write2XML(prop,parameterised);

	TiXmlElement value("Property");
	WriteTerm(Epsilon,value,"Epsilon",parameterised);
	WriteTerm(Mue,value,"Mue",parameterised);
	WriteTerm(Kappa,value,"Kappa",parameterised);
	prop.InsertEndChild(value);
	root.InsertEndChild(prop);
	return true;
}

bool CSPropMaterial::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;
	TiXmlElement* matProp=prop->FirstChildElement("Property");
	if (matProp==NULL) return false;
	if (ReadTerm(Epsilon,*matProp,"Epsilon")==false) return false;
	if (ReadTerm(Mue,*matProp,"Mue")==false) return false;
	if (ReadTerm(Kappa,*matProp,"Kappa")==false) return false;

	return true;
}

/*********************CSPropMetal********************************************************************/
CSPropMetal::CSPropMetal(ParameterSet* paraSet) : CSProperties(paraSet) {Type=METAL;}
CSPropMetal::CSPropMetal(CSProperties* prop) : CSProperties(prop) {Type=METAL;}
CSPropMetal::CSPropMetal(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=METAL;}
CSPropMetal::~CSPropMetal() {}

bool CSPropMetal::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement prop("Metal");
	CSProperties::Write2XML(prop,parameterised);

	root.InsertEndChild(prop);
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

void CSPropElectrode::SetWeightFct(const string fct, int ny) {if ((ny>=0) && (ny<3)) sWeightFct[ny]=string(fct);}
const string CSPropElectrode::GetWeightFct(int ny) {if ((ny>=0) && (ny<3)) {return sWeightFct[ny];} else return string();}

void CSPropElectrode::Init()
{
	uiNumber=0;
	iExcitType=1;
	for (unsigned int i=0;i<3;++i)
	{
		Excitation[i].SetValue(0.0);
		Excitation[i].SetParameterSet(clParaSet);
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
	return bOK;
}

bool CSPropElectrode::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement prop("Electrode");
	CSProperties::Write2XML(prop,parameterised);

	prop.SetAttribute("Number",(int)uiNumber);

	TiXmlElement Excit("Excitation");
	Excit.SetAttribute("Type",iExcitType);
	WriteTerm(Excitation[0],Excit,"Excit_X",parameterised);
	WriteTerm(Excitation[1],Excit,"Excit_Y",parameterised);
	WriteTerm(Excitation[2],Excit,"Excit_Z",parameterised);
	prop.InsertEndChild(Excit);

	if ((sWeightFct[0].empty()==false) ||(sWeightFct[1].empty()==false) ||(sWeightFct[2].empty()==false))
	{
		TiXmlElement Weight("Weight");
		Weight.SetAttribute("Function_X",sWeightFct[0].c_str());
//		Weight.SetAttribute("Vars_X",sWeightVars[0].c_str());
		Weight.SetAttribute("Function_Y",sWeightFct[1].c_str());
//		Weight.SetAttribute("Vars_Y",sWeightVars[1].c_str());
		Weight.SetAttribute("Function_Z",sWeightFct[2].c_str());
//		Weight.SetAttribute("Vars_Z",sWeightVars[2].c_str());
		prop.InsertEndChild(Weight);
	}

	root.InsertEndChild(prop);
	return true;
}

bool CSPropElectrode::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;

	TiXmlElement *prop = root.ToElement();
	if (prop==NULL) return false;

	int iHelp;
	if (prop->QueryIntAttribute("Number",&iHelp)!=TIXML_SUCCESS) return false;
	else uiNumber=(unsigned int)iHelp;

	TiXmlElement *excit = prop->FirstChildElement("Excitation");
	if (excit==NULL) return false;
	if (excit->QueryIntAttribute("Type",&iExcitType)!=TIXML_SUCCESS) return false;
	if (ReadTerm(Excitation[0],*excit,"Excit_X")==false) return false;
	if (ReadTerm(Excitation[1],*excit,"Excit_Y")==false) return false;
	if (ReadTerm(Excitation[2],*excit,"Excit_Z")==false) return false;

	TiXmlElement *weight = prop->FirstChildElement("Weight");
	const char* cHelp=NULL;
	if (weight!=NULL)
	{
		cHelp=weight->Attribute("Function_X");
		if (cHelp!=NULL) sWeightFct[0]=string(cHelp);
		else sWeightFct[0].clear();
//		cHelp=weight->Attribute("Vars_X");
//		if (cHelp!=NULL) sWeightVars[0]=string(cHelp);
//		else sWeightVars[0].clear();

		cHelp=weight->Attribute("Function_Y");
		if (cHelp!=NULL) sWeightFct[1]=string(cHelp);
		else sWeightFct[1].clear();
//		cHelp=weight->Attribute("Vars_Y");
//		if (cHelp!=NULL) sWeightVars[1]=string(cHelp);
//		else sWeightVars[1].clear();

		cHelp=weight->Attribute("Function_Z");
		if (cHelp!=NULL) sWeightFct[2]=string(cHelp);
		else sWeightFct[2].clear();
//		cHelp=weight->Attribute("Vars_Z");
//		if (cHelp!=NULL) sWeightVars[2]=string(cHelp);
//		else sWeightVars[2].clear();
	}

	return true;
}

/*********************CSPropChargeBox********************************************************************/
CSPropChargeBox::CSPropChargeBox(ParameterSet* paraSet) : CSProperties(paraSet) {Type=CHARGEBOX;uiNumber=0;}
CSPropChargeBox::CSPropChargeBox(CSProperties* prop) : CSProperties(prop) {Type=CHARGEBOX;uiNumber=0;}
CSPropChargeBox::CSPropChargeBox(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=CHARGEBOX;uiNumber=0;}
CSPropChargeBox::~CSPropChargeBox() {}

void CSPropChargeBox::SetNumber(unsigned int val) {uiNumber=val;}
unsigned int CSPropChargeBox::GetNumber() {return uiNumber;}

bool CSPropChargeBox::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement prop("ProbeBox");
	CSProperties::Write2XML(prop,parameterised);

	prop.SetAttribute("Number",(int)uiNumber);

	root.InsertEndChild(prop);
	return true;
}

bool CSPropChargeBox::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;

	TiXmlElement *prop = root.ToElement();
	if (prop==NULL) return false;

	int iHelp;
	if (prop->QueryIntAttribute("Number",&iHelp)!=TIXML_SUCCESS) return false;
	else uiNumber=(unsigned int)iHelp;

	return true;
}

/*********************CSPropResBox********************************************************************/
CSPropResBox::CSPropResBox(ParameterSet* paraSet) : CSProperties(paraSet) {Type=RESBOX;uiFactor=1;} ;
CSPropResBox::CSPropResBox(CSProperties* prop) : CSProperties(prop) {Type=RESBOX;uiFactor=1;} ;
CSPropResBox::CSPropResBox(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=RESBOX;uiFactor=1;} ;
CSPropResBox::~CSPropResBox() {};

void CSPropResBox::SetResFactor(unsigned int val)  {uiFactor=val;}
unsigned int CSPropResBox::GetResFactor()  {return uiFactor;}

bool CSPropResBox::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement prop("ResBox");
	CSProperties::Write2XML(prop,parameterised);

	prop.SetAttribute("Factor",(int)uiFactor);

	root.InsertEndChild(prop);
	return true;
}

bool CSPropResBox::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;

	TiXmlElement *prop = root.ToElement();
	if (prop==NULL) return false;

	int iHelp;
	if (prop->QueryIntAttribute("Factor",&iHelp)!=TIXML_SUCCESS) return false;
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
}

bool CSPropDumpBox::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement prop("DumpBox");
	CSProperties::Write2XML(prop,parameterised);

	prop.SetAttribute("GlobalSetting",(int)GlobalSetting);

	TiXmlElement ScalDump("ScalarDump");
	ScalDump.SetAttribute("DumpPhi",(int)PhiDump);
	ScalDump.SetAttribute("DumpDivE",(int)divE);
	ScalDump.SetAttribute("DumpDivD",(int)divD);
	ScalDump.SetAttribute("DumpDivP",(int)divP);
	ScalDump.SetAttribute("DumpFieldW",(int)FieldW);
	ScalDump.SetAttribute("DumpChargeW",(int)ChargeW);
	prop.InsertEndChild(ScalDump);

	TiXmlElement VecDump("VectorDump");
	VecDump.SetAttribute("DumpEField",(int)EField);
	VecDump.SetAttribute("DumpDField",(int)DField);
	VecDump.SetAttribute("DumpPField",(int)PField);
	prop.InsertEndChild(VecDump);

	TiXmlElement SubGDump("SubGridDump");
	SubGDump.SetAttribute("SubGridDump",(int)SGDump);
	SubGDump.SetAttribute("SimpleDump",(int)SimpleDump);
	SubGDump.SetAttribute("SubGridLevel",SGLevel);
	prop.InsertEndChild(SubGDump);

	root.InsertEndChild(prop);

	return true;
}

bool CSPropDumpBox::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;

	TiXmlElement *prop = root.ToElement();
	if (prop==NULL) return false;

	int iHelp;
	if (prop->QueryIntAttribute("GlobalSetting",&iHelp)!=TIXML_SUCCESS) return false;
	else GlobalSetting=(bool)iHelp;

	TiXmlElement *ScalDump = prop->FirstChildElement("ScalarDump");
	if (ScalDump!=NULL)
	{
		if (ScalDump->QueryIntAttribute("DumpPhi",&iHelp)!=TIXML_SUCCESS) return false;
		else PhiDump=(bool)iHelp;
		if (ScalDump->QueryIntAttribute("DumpDivE",&iHelp)!=TIXML_SUCCESS) return false;
		else divE=(bool)iHelp;
		if (ScalDump->QueryIntAttribute("DumpDivD",&iHelp)!=TIXML_SUCCESS) return false;
		else divD=(bool)iHelp;
		if (ScalDump->QueryIntAttribute("DumpDivP",&iHelp)!=TIXML_SUCCESS) return false;
		else divP=(bool)iHelp;
		if (ScalDump->QueryIntAttribute("DumpFieldW",&iHelp)!=TIXML_SUCCESS) return false;
		else FieldW=(bool)iHelp;
		if (ScalDump->QueryIntAttribute("DumpChargeW",&iHelp)!=TIXML_SUCCESS) return false;
		else ChargeW=(bool)iHelp;
	}
	TiXmlElement *VecDump = prop->FirstChildElement("VectorDump");
	if (VecDump!=NULL)
	{

		if (VecDump->QueryIntAttribute("DumpEField",&iHelp)!=TIXML_SUCCESS) return false;
		else EField=(bool)iHelp;
		if (VecDump->QueryIntAttribute("DumpDField",&iHelp)!=TIXML_SUCCESS) return false;
		else DField=(bool)iHelp;
		if (VecDump->QueryIntAttribute("DumpPField",&iHelp)!=TIXML_SUCCESS) return false;
		else PField=(bool)iHelp;
	}
	TiXmlElement *SubGDump = prop->FirstChildElement("SubGridDump");
	if (SubGDump!=NULL)
	{

		if (SubGDump->QueryIntAttribute("SubGridDump",&iHelp)!=TIXML_SUCCESS) return false;
		else SGDump=(bool)iHelp;
		if (SubGDump->QueryIntAttribute("SimpleDump",&iHelp)!=TIXML_SUCCESS) return false;
		else SimpleDump=(bool)iHelp;
		if (SubGDump->QueryIntAttribute("SubGridLevel",&iHelp)!=TIXML_SUCCESS) return false;
		else SGLevel=iHelp;
	}
	return true;
}
