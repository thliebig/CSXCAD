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

#include "CSPropExcitation.h"

CSPropExcitation::CSPropExcitation(ParameterSet* paraSet,unsigned int number) : CSProperties(paraSet) {Type=EXCITATION;Init();uiNumber=number;}
CSPropExcitation::CSPropExcitation(CSProperties* prop) : CSProperties(prop) {Type=EXCITATION;Init();}
CSPropExcitation::CSPropExcitation(unsigned int ID, ParameterSet* paraSet) : CSProperties(ID,paraSet) {Type=EXCITATION;Init();}
CSPropExcitation::~CSPropExcitation() {}

void CSPropExcitation::SetNumber(unsigned int val) {uiNumber=val;}
unsigned int CSPropExcitation::GetNumber() {return uiNumber;}

void CSPropExcitation::SetExcitType(int val) {iExcitType=val;}
int CSPropExcitation::GetExcitType() {return iExcitType;}

void CSPropExcitation::SetExcitation(double val, int Component)
{
	if ((Component<0) || (Component>=3)) return;
	Excitation[Component].SetValue(val);
}

void CSPropExcitation::SetExcitation(const std::string val, int Component)
{
	if ((Component<0) || (Component>=3)) return;
	Excitation[Component].SetValue(val);
}

double CSPropExcitation::GetExcitation(int Component)
{
	if ((Component<0) || (Component>=3)) return 0;
	return Excitation[Component].GetValue();
}

const std::string CSPropExcitation::GetExcitationString(int Comp)
{
	if ((Comp<0) || (Comp>=3)) return NULL;
	return Excitation[Comp].GetString();
}

void CSPropExcitation::SetActiveDir(bool active, int Component)
{
	if ((Component<0) || (Component>=3)) return;
	ActiveDir[Component]=active;
}

bool CSPropExcitation::GetActiveDir(int Component)
{
	if ((Component<0) || (Component>=3)) return false;
	return ActiveDir[Component];
}

int CSPropExcitation::SetWeightFunction(const std::string fct, int ny)
{
	if ((ny>=0) && (ny<3))
		return WeightFct[ny].SetValue(fct);
	return 0;
}

const std::string CSPropExcitation::GetWeightFunction(int ny) {if ((ny>=0) && (ny<3)) {return WeightFct[ny].GetString();} else return std::string();}

double CSPropExcitation::GetWeightedExcitation(int ny, const double* coords)
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
		std::cerr << "CSPropExcitation::GetWeightedExcitation: Error evaluating the weighting function (ID: " << this->GetID() << ", n=" << ny << "): " << PSErrorCode2Msg(EC) << std::endl;
	}

	return WeightFct[ny].GetValue()*GetExcitation(ny);
}

void CSPropExcitation::SetDelay(double val)	{Delay.SetValue(val);}

void CSPropExcitation::SetDelay(const std::string val) {Delay.SetValue(val);}

double CSPropExcitation::GetDelay(){return Delay.GetValue();}

const std::string CSPropExcitation::GetDelayString(){return Delay.GetString();}

void CSPropExcitation::Init()
{
	uiNumber=0;
	iExcitType=1;
	coordInputType=UNDEFINED_CS;
	m_Frequency.SetValue(0.0);
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

void CSPropExcitation::SetPropagationDir(double val, int Component)
{
	if ((Component<0) || (Component>=3)) return;
	PropagationDir[Component].SetValue(val);
}

void CSPropExcitation::SetPropagationDir(const std::string val, int Component)
{
	if ((Component<0) || (Component>=3)) return;
	PropagationDir[Component].SetValue(val);
}

double CSPropExcitation::GetPropagationDir(int Component)
{
	if ((Component<0) || (Component>=3)) return 0;
	return PropagationDir[Component].GetValue();
}

const std::string CSPropExcitation::GetPropagationDirString(int Comp)
{
	if ((Comp<0) || (Comp>=3)) return NULL;
	return PropagationDir[Comp].GetString();
}


bool CSPropExcitation::Update(std::string *ErrStr)
{
	bool bOK=true;
	int EC=0;
	for (unsigned int i=0;i<3;++i)
	{
		EC=Excitation[i].Evaluate();
		if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
		{
			std::stringstream stream;
			stream << std::endl << "Error in Excitation-Property Excitaion-Value (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=PropagationDir[i].Evaluate();
		if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
		{
			std::stringstream stream;
			stream << std::endl << "Error in Excitation-Property PropagationDir-Value (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}
	EC=m_Frequency.Evaluate();
	if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
	{
		std::stringstream stream;
		stream << std::endl << "Error in Excitation-Property Frequency-Value";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}
	EC=Delay.Evaluate();
	if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
	{
		std::stringstream stream;
		stream << std::endl << "Error in Excitation-Property Delay-Value";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}
	return bOK;
}

bool CSPropExcitation::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSProperties::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	prop->SetAttribute("Number",(int)uiNumber);
	WriteTerm(m_Frequency,*prop,"Frequency",parameterised);
	WriteTerm(Delay,*prop,"Delay",parameterised);

	prop->SetAttribute("Type",iExcitType);
	WriteVectorTerm(Excitation,*prop,"Excite",parameterised);

	TiXmlElement Weight("Weight");
	WriteTerm(WeightFct[0],Weight,"X",parameterised);
	WriteTerm(WeightFct[1],Weight,"Y",parameterised);
	WriteTerm(WeightFct[2],Weight,"Z",parameterised);
	prop->InsertEndChild(Weight);

	WriteVectorTerm(PropagationDir,*prop,"PropDir",parameterised);

	return true;
}

bool CSPropExcitation::ReadFromXML(TiXmlNode &root)
{
	if (CSProperties::ReadFromXML(root)==false) return false;

	TiXmlElement *prop = root.ToElement();
	if (prop==NULL) return false;

	int iHelp;
	if (prop->QueryIntAttribute("Number",&iHelp)!=TIXML_SUCCESS) uiNumber=0;
	else uiNumber=(unsigned int)iHelp;

	if (prop->QueryIntAttribute("Type",&iExcitType)!=TIXML_SUCCESS) return false;

	if (ReadVectorTerm(Excitation,*prop,"Excite",0.0)==false) return false;

	ReadTerm(m_Frequency,*prop,"Frequency");
	ReadTerm(Delay,*prop,"Delay");

	TiXmlElement *weight = prop->FirstChildElement("Weight");
	if (weight!=NULL)
	{
		ReadTerm(WeightFct[0],*weight,"X");
		ReadTerm(WeightFct[1],*weight,"Y");
		ReadTerm(WeightFct[2],*weight,"Z");
	}

	ReadVectorTerm(PropagationDir,*prop,"PropDir",0.0);

	return true;
}

void CSPropExcitation::ShowPropertyStatus(std::ostream& stream)
{
	CSProperties::ShowPropertyStatus(stream);
	stream << " --- Excitation Properties --- " << std::endl;
	stream << "  Type: " << iExcitType << std::endl;
	stream << "  Active directions: " << ActiveDir[0] << "," << ActiveDir[1] << "," << ActiveDir[2] << std::endl;
	stream << "  Excitation\t: " << Excitation[0].GetValueString() << ", "  << Excitation[1].GetValueString() << ", "  << Excitation[2].GetValueString()  << std::endl;
	stream << "  Weighting\t: " << WeightFct[0].GetValueString() << ", "  << WeightFct[1].GetValueString() << ", "  << WeightFct[2].GetValueString()  << std::endl;
	stream << "  Propagation Dir: " << PropagationDir[0].GetValueString() << ", "  << PropagationDir[1].GetValueString() << ", "  << PropagationDir[2].GetValueString()  << std::endl;
	stream << "  Delay\t\t: " << Delay.GetValueString() << std::endl;
}
