#include "CSProperties.h"
#include "CSPrimitives.h"
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
	coordInputType=0;
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
	coordInputType=0;
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

void CSProperties::AddPrimitive(CSPrimitives *prim) {vPrimitives.push_back(prim);}

size_t CSProperties::GetQtyPrimitives() {return vPrimitives.size();}
CSPrimitives* CSProperties::GetPrimitive(size_t index) {if (index<vPrimitives.size()) return vPrimitives.at(index); else return NULL;}
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
CSPropProbeBox* CSProperties::ToProbeBox() { return ( this && Type == PROBEBOX ) ? (CSPropProbeBox*) this : 0; }
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

void CSPropMaterial::SetEpsilon(double val, int direction)
{
    if ((direction>2) || (direction<0)) return;
    Epsilon[direction].SetValue(val);
}
void CSPropMaterial::SetEpsilon(const string val, int direction)
{
    if ((direction>2) || (direction<0)) return;
    Epsilon[direction].SetValue(val);
}
double CSPropMaterial::GetEpsilon(int direction)
{
    if (bIsotropy) direction=0;
    if ((direction>2) || (direction<0)) direction=0;
    return Epsilon[direction].GetValue();
}
const string CSPropMaterial::GetEpsilonTerm(int direction)
{
    if (bIsotropy) direction=0;
    if ((direction>2) || (direction<0)) direction=0;
    return Epsilon[direction].GetString();
}

double CSPropMaterial::GetWeight(ParameterScalar &ps, double* coords)
{
//	cerr << "CSPropElectrode::GetWeightedExcitation: methode not yet supported!! Falling back to CSPropElectrode::GetExcitation" << endl;
	coordPara[0]->SetValue(coords[0]);
	coordPara[1]->SetValue(coords[1]);
	coordPara[2]->SetValue(coords[2]);
	double rho = sqrt(pow(coords[0],2)+pow(coords[1],2));
	coordPara[3]->SetValue(rho); //rho
	coordPara[4]->SetValue(sqrt(pow(coords[0],2)+pow(coords[1],2)+pow(coords[2],2))); //r
	coordPara[5]->SetValue(atan2(coords[1],coords[0]));  //alpha
	coordPara[6]->SetValue(asin(1)-atan(coords[2]/rho)); //theta
	ps.Evaluate();
	return ps.GetValue();
}

int CSPropMaterial::SetEpsilonWeightFunction(const string fct, int ny) {if ((ny>=0) && (ny<3)) return WeightEpsilon[ny].SetValue(fct);}
const string CSPropMaterial::GetEpsilonWeightFunction(int ny) {if ((ny>=0) && (ny<3)) {return WeightEpsilon[ny].GetString();} else return string();}
double CSPropMaterial::GetEpsilonWeighted(int ny, double* coords)
{
	if ((ny<0) || (ny>=3)) return 0;
	return GetWeight(WeightEpsilon[ny],coords)*GetEpsilon(ny);
}

void CSPropMaterial::SetMue(double val, int direction)
{
    if ((direction>2) || (direction<0)) return;
    Mue[direction].SetValue(val);
}
void CSPropMaterial::SetMue(const string val, int direction)
{
    if ((direction>2) || (direction<0)) return;
    Mue[direction].SetValue(val);
}
double CSPropMaterial::GetMue(int direction)
{
    if (bIsotropy) direction=0;
    if ((direction>2) || (direction<0)) direction=0;
    return Mue[direction].GetValue();
}
const string CSPropMaterial::GetMueTerm(int direction)
{
    if (bIsotropy) direction=0;
    if ((direction>2) || (direction<0)) direction=0;
    return Mue[direction].GetString();
}

int CSPropMaterial::SetMueWeightFunction(const string fct, int ny) {if ((ny>=0) && (ny<3)) return WeightMue[ny].SetValue(fct);}
const string CSPropMaterial::GetMueWeightFunction(int ny) {if ((ny>=0) && (ny<3)) {return WeightMue[ny].GetString();} else return string();}
double CSPropMaterial::GetMueWeighted(int ny, double* coords)
{
	if ((ny<0) || (ny>=3)) return 0;
	return GetWeight(WeightMue[ny],coords)*GetMue(ny);
}


void CSPropMaterial::SetKappa(double val, int direction)
{
    if ((direction>2) || (direction<0)) return;
    Kappa[direction].SetValue(val);
}
void CSPropMaterial::SetKappa(const string val, int direction)
{
    if ((direction>2) || (direction<0)) return;
    Kappa[direction].SetValue(val);
}
double CSPropMaterial::GetKappa(int direction)
{
    if (bIsotropy) direction=0;
    if ((direction>2) || (direction<0)) direction=0;
    return Kappa[direction].GetValue();
}
const string CSPropMaterial::GetKappaTerm(int direction)
{
    if (bIsotropy) direction=0;
    if ((direction>2) || (direction<0)) direction=0;
    return Kappa[direction].GetString();
}

int CSPropMaterial::SetKappaWeightFunction(const string fct, int ny) {if ((ny>=0) && (ny<3)) return WeightKappa[ny].SetValue(fct);}
const string CSPropMaterial::GetKappaWeightFunction(int ny) {if ((ny>=0) && (ny<3)) {return WeightKappa[ny].GetString();} else return string();}
double CSPropMaterial::GetKappaWeighted(int ny, double* coords)
{
	if ((ny<0) || (ny>=3)) return 0;
	return GetWeight(WeightKappa[ny],coords)*GetKappa(ny);
}

void CSPropMaterial::SetSigma(double val, int direction)
{
    if ((direction>2) || (direction<0)) return;
    Sigma[direction].SetValue(val);
}
void CSPropMaterial::SetSigma(const string val, int direction)
{
    if ((direction>2) || (direction<0)) return; Sigma[direction].SetValue(val);
}
double CSPropMaterial::GetSigma(int direction)
{
    if (bIsotropy) direction=0;
    if ((direction>2) || (direction<0)) direction=0;
    return Sigma[direction].GetValue();
}
const string CSPropMaterial::GetSigmaTerm(int direction)
{
    if (bIsotropy) direction=0;
    if ((direction>2) || (direction<0)) direction=0;
    return Sigma[direction].GetString();
}

int CSPropMaterial::SetSigmaWeightFunction(const string fct, int ny) {if ((ny>=0) && (ny<3)) return WeightSigma[ny].SetValue(fct);}
const string CSPropMaterial::GetSigmaWeightFunction(int ny) {if ((ny>=0) && (ny<3)) {return WeightSigma[ny].GetString();} else return string();}
double CSPropMaterial::GetSigmaWeighted(int ny, double* coords)
{
	if ((ny<0) || (ny>=3)) return 0;
	return GetWeight(WeightSigma[ny],coords)*GetSigma(ny);
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
            if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
            {
                    stringstream stream;
                    stream << endl << "Error in Material-Property Epsilon-Value (ID: " << uiID << "): ";
                    ErrStr->append(stream.str());
                    PSErrorCode2Msg(EC,ErrStr);
            }
            EC=Mue[n].Evaluate();
            if (EC!=ParameterScalar::NO_ERROR) bOK=false;
            if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
            {
                    stringstream stream;
                    stream << endl << "Error in Material-Property Mue-Value (ID: " << uiID << "): ";
                    ErrStr->append(stream.str());
                    PSErrorCode2Msg(EC,ErrStr);
            }
            EC=Kappa[n].Evaluate();
            if (EC!=ParameterScalar::NO_ERROR) bOK=false;
            if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
            {
                    stringstream stream;
                    stream << endl << "Error in Material-Property Kappa-Value (ID: " << uiID << "): ";
                    ErrStr->append(stream.str());
                    PSErrorCode2Msg(EC,ErrStr);
            }
            EC=Sigma[n].Evaluate();
            if (EC!=ParameterScalar::NO_ERROR) bOK=false;
            if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
            {
                    stringstream stream;
                    stream << endl << "Error in Material-Property Sigma-Value (ID: " << uiID << "): ";
                    ErrStr->append(stream.str());
                    PSErrorCode2Msg(EC,ErrStr);
            }
        }
	return bOK;
}

bool CSPropMaterial::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement prop("Material");
	CSProperties::Write2XML(prop,parameterised);
        prop.SetAttribute("Isotropy",bIsotropy);

        TiXmlElement value("Property");
        WriteTerm(Epsilon[0],value,"Epsilon",parameterised);
        WriteTerm(Mue[0],value,"Mue",parameterised);
        WriteTerm(Kappa[0],value,"Kappa",parameterised);
        WriteTerm(Sigma[0],value,"Sigma",parameterised);
        prop.InsertEndChild(value);

        value = TiXmlElement("PropertyY");
        WriteTerm(Epsilon[1],value,"Epsilon",parameterised);
        WriteTerm(Mue[1],value,"Mue",parameterised);
        WriteTerm(Kappa[1],value,"Kappa",parameterised);
        WriteTerm(Sigma[1],value,"Sigma",parameterised);
        prop.InsertEndChild(value);

        value = TiXmlElement("PropertyZ");
        WriteTerm(Epsilon[2],value,"Epsilon",parameterised);
        WriteTerm(Mue[2],value,"Mue",parameterised);
        WriteTerm(Kappa[2],value,"Kappa",parameterised);
        WriteTerm(Sigma[2],value,"Sigma",parameterised);
        prop.InsertEndChild(value);

	root.InsertEndChild(prop);
	return true;
}

bool CSPropMaterial::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;
	TiXmlElement* prop=root.ToElement();

        int attr=1;
        prop->QueryIntAttribute("Isotropy",&attr);
        bIsotropy = attr>0;
	if (prop==NULL) return false;
	TiXmlElement* matProp=prop->FirstChildElement("Property");
	if (matProp==NULL) return false;
        if (ReadTerm(Epsilon[0],*matProp,"Epsilon")==false) return false;
        if (ReadTerm(Mue[0],*matProp,"Mue")==false) return false;
        if (ReadTerm(Kappa[0],*matProp,"Kappa")==false) return false;
        ReadTerm(Sigma[0],*matProp,"Sigma"); //always accept do to legacy support

        matProp=prop->FirstChildElement("PropertyY");
        if (matProp!=NULL) //always accept do to legacy support
        {
            if (ReadTerm(Epsilon[1],*matProp,"Epsilon")==false) return false;
            if (ReadTerm(Mue[1],*matProp,"Mue")==false) return false;
            if (ReadTerm(Kappa[1],*matProp,"Kappa")==false) return false;
            if (ReadTerm(Sigma[1],*matProp,"Sigma")==false) return false;
        }
        matProp=prop->FirstChildElement("PropertyZ");
        if (matProp!=NULL) //always accept do to legacy support
        {
            if (ReadTerm(Epsilon[2],*matProp,"Epsilon")==false) return false;
            if (ReadTerm(Mue[2],*matProp,"Mue")==false) return false;
            if (ReadTerm(Kappa[2],*matProp,"Kappa")==false) return false;
            if (ReadTerm(Sigma[2],*matProp,"Sigma")==false) return false;
        }

	return true;
}

/*********************CSPropMetal********************************************************************/
CSPropMetal::CSPropMetal(ParameterSet* paraSet) : CSProperties(paraSet) {Type=METAL;bMaterial=true;}
CSPropMetal::CSPropMetal(CSProperties* prop) : CSProperties(prop) {Type=METAL;bMaterial=true;}
CSPropMetal::CSPropMetal(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=METAL;bMaterial=true;}
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

int CSPropElectrode::SetWeightFunction(const string fct, int ny) {if ((ny>=0) && (ny<3)) return WeightFct[ny].SetValue(fct);}
const string CSPropElectrode::GetWeightFunction(int ny) {if ((ny>=0) && (ny<3)) {return WeightFct[ny].GetString();} else return string();}

double CSPropElectrode::GetWeightedExcitation(int ny, double* coords)
{
	if ((ny<0) || (ny>=3)) return 0;
	double r,rho,alpha,theta;
	if (coordInputType==1)
	{
		double orig[3] = {coords[0],coords[1],coords[2]};
		coords[0] = orig[0]*cos(orig[1]);
		coords[1] = orig[0]*sin(orig[1]);
		rho = orig[0];
		alpha=orig[1];
		r = sqrt(pow(orig[0],2)+pow(orig[2],2));
		theta=asin(1)-atan(coords[2]/rho);
	}
	else
	{
		alpha=atan2(coords[1],coords[0]);
		rho = sqrt(pow(coords[0],2)+pow(coords[1],2));
		r = sqrt(pow(coords[0],2)+pow(coords[1],2)+pow(coords[2],2));
		theta=asin(1)-atan(coords[2]/rho);
	}
	coordPara[0]->SetValue(coords[0]);
	coordPara[1]->SetValue(coords[1]);
	coordPara[2]->SetValue(coords[2]);
	coordPara[3]->SetValue(rho); //rho
	coordPara[4]->SetValue(r); //r
	coordPara[5]->SetValue(alpha);
	coordPara[6]->SetValue(theta); //theta
	WeightFct[ny].Evaluate();
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

bool CSPropElectrode::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement prop("Electrode");
	CSProperties::Write2XML(prop,parameterised);

	prop.SetAttribute("Number",(int)uiNumber);
	WriteTerm(Delay,prop,"Delay",parameterised);

	TiXmlElement Excit("Excitation");
	Excit.SetAttribute("Type",iExcitType);
	WriteTerm(Excitation[0],Excit,"Excit_X",parameterised);
	WriteTerm(Excitation[1],Excit,"Excit_Y",parameterised);
	WriteTerm(Excitation[2],Excit,"Excit_Z",parameterised);
	prop.InsertEndChild(Excit);

	TiXmlElement Weight("Weight");
	WriteTerm(WeightFct[0],Weight,"Function_X",parameterised);
	WriteTerm(WeightFct[1],Weight,"Function_Y",parameterised);
	WriteTerm(WeightFct[2],Weight,"Function_Z",parameterised);
	prop.InsertEndChild(Weight);

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
		ReadTerm(WeightFct[0],*weight,"Excit_X");
		ReadTerm(WeightFct[1],*weight,"Excit_Y");
		ReadTerm(WeightFct[2],*weight,"Excit_Z");
	}

	return true;
}

/*********************CSPropProbeBox********************************************************************/
CSPropProbeBox::CSPropProbeBox(ParameterSet* paraSet) : CSProperties(paraSet) {Type=PROBEBOX;uiNumber=0;ProbeType=0;}
CSPropProbeBox::CSPropProbeBox(CSProperties* prop) : CSProperties(prop) {Type=PROBEBOX;uiNumber=0;ProbeType=0;}
CSPropProbeBox::CSPropProbeBox(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=PROBEBOX;uiNumber=0;ProbeType=0;}
CSPropProbeBox::~CSPropProbeBox() {}

void CSPropProbeBox::SetNumber(unsigned int val) {uiNumber=val;}
unsigned int CSPropProbeBox::GetNumber() {return uiNumber;}

bool CSPropProbeBox::Write2XML(TiXmlNode& root, bool parameterised)
{
	TiXmlElement prop("ProbeBox");
	CSProperties::Write2XML(prop,parameterised);

	prop.SetAttribute("Number",(int)uiNumber);
	prop.SetAttribute("Type",ProbeType);

	root.InsertEndChild(prop);
	return true;
}

bool CSPropProbeBox::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;

	TiXmlElement *prop = root.ToElement();
	if (prop==NULL) return false;

	int iHelp;
	if (prop->QueryIntAttribute("Number",&iHelp)!=TIXML_SUCCESS) return false;
	else uiNumber=(unsigned int)iHelp;

	if (prop->QueryIntAttribute("Type",&ProbeType)!=TIXML_SUCCESS) ProbeType=0;

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
