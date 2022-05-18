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

#include "ParameterObjects.h"
#include <sstream>
#include <iostream>
#include "tinyxml.h"
#include "CSFunctionParser.h"
#include "CSUseful.h"

bool ReadTerm(ParameterScalar &PS, TiXmlElement &elem, const char* attr, double val)
{
	double dHelp;

	PS.SetValue(val); // set default value
	const char* chHelp=elem.Attribute(attr);
	if (chHelp==NULL)
		return false;
	PS.SetValue(chHelp); // set string value if found
	return true;
}

void WriteTerm(ParameterScalar &PS, TiXmlElement &elem, const char* attr, bool mode, bool scientific)
{
	if (PS.GetMode() && mode)
		elem.SetAttribute(attr,PS.GetString().c_str());
	else
	{
		if (PS.GetValue()==NAN)
			return;
		if (scientific)
		{
			char doubleVal[50];
			sprintf(doubleVal,"%e",PS.GetValue());
			elem.SetAttribute(attr,doubleVal);
		}
		else
			elem.SetDoubleAttribute(attr,PS.GetValue());
	}
}

bool ReadVectorTerm(ParameterScalar PS[3], TiXmlElement &elem, std::string attr, double val, const char delimiter)
{
	return ReadVectorTerm(PS, elem, attr.c_str(), val, delimiter);
}

bool ReadVectorTerm(ParameterScalar PS[3], TiXmlElement &elem, const char* attr, double val, const char delimiter)
{
	//initialize with default value
	for (int n=0;n<3;++n)
		PS[n].SetValue(val);

	const char* values = elem.Attribute(attr);
	if (values==NULL)
		return false;
	std::vector<std::string> val_list = SplitString2Vector(values, delimiter);
	if (val_list.size()>3)
		return false;

	for (int n=0;n<3;++n)
		PS[n].SetValue(val);
	for (int n=0;n<(int)val_list.size();++n)
	{
		std::string sHelp=val_list.at(n);
		bool ok;
		double val = String2Double(sHelp, ok);
		if (ok)
			PS[n].SetValue(val);
		else
			PS[n].SetValue(sHelp.c_str());
	}
	return true;
}

void WriteVectorTerm(ParameterScalar PS[3], TiXmlElement &elem, std::string attr, bool mode, bool scientific, const char delimiter)
{
	WriteVectorTerm(PS, elem, attr.c_str(), mode, scientific, delimiter);
}

void WriteVectorTerm(ParameterScalar PS[3], TiXmlElement &elem, const char* attr, bool mode, bool sci, const char delimiter)
{
	std::stringstream ss;
	if (sci)
		ss << std::scientific;
	for (int i=0;i<3;++i)
	{
		if (PS[i].GetMode() && mode)
			ss << PS[i].GetString();
		else if (PS[i].GetValue()==NAN)
			ss << "NAN" << std::endl;
		else
			ss << PS[i].GetValue();
		if (i<2)
			ss << delimiter;
	}
	elem.SetAttribute(attr,ss.str().c_str());
}

Parameter::Parameter()
{
	bModified=true;
	dValue=0;
	Type=Const;
	bSweep=true;
//	Set=NULL;
}

Parameter::Parameter(const std::string Paraname, double val)
{
	sName=Paraname;
	SetValue(val);
	Type=Const;
	bSweep=true;
//	Set=NULL;
}

Parameter::~Parameter()
{
//	if (Set!=NULL) Set->RemoveParameter(this);
}


void Parameter::PrintSelf(FILE* /*out*/)
{
	fprintf(stderr," Parameter: %s  Value: %f  Type %d\n\n",sName.c_str(),dValue,Type);
}

bool Parameter::Write2XML(TiXmlNode& root)
{
	TiXmlElement *elem=NULL;
	if (Type==Const)
	{
		elem = new TiXmlElement("Parameter");
		elem->SetAttribute("Type","Const");
	}
	else elem = root.ToElement();
	if (elem==NULL) return false;
	elem->SetAttribute("name",sName.c_str());
	elem->SetAttribute("Sweep",bSweep);
	elem->SetDoubleAttribute("value",dValue);
	if (Type==Const) root.LinkEndChild(elem);
	return true;
}

bool Parameter::ReadFromXML(TiXmlNode &root)
{
	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;

	int iAtt=0;
	if (elem->QueryIntAttribute("Sweep",&iAtt)!=TIXML_SUCCESS) bSweep=true;
	else if (iAtt>0) bSweep=true;
	else bSweep=false;

	double val=0;
	if (elem->QueryDoubleAttribute("value",&val)!=TIXML_SUCCESS) return false;
	SetValue(val);

	const char* att=elem->Attribute("name");
	if (att==NULL) sName=std::string();
	else sName=std::string(att);

	return true;
}

Parameter* Parameter::GetParameterFromXML(TiXmlNode &/*root*/)
{
	return NULL;
}

LinearParameter::LinearParameter() : Parameter()
{
	Type=Linear;
	dMin=dMax=dStep=0;
}

LinearParameter::LinearParameter(const std::string Paraname, double val, double min, double max, double step) : Parameter(Paraname,val)
{
	//sName=string(Paraname);
	if (max<min) max=min;
	dMin=min;
	dMax=max;
	if (step<0) step=0;
	dStep=step;
	Type=Linear;

	SetValue(dValue);
}

LinearParameter::LinearParameter(const LinearParameter *parameter) : Parameter(parameter)
{
	if (parameter->Type==Linear)
	{
		dMin=parameter->dMin;
		dMax=parameter->dMax;
		dStep=parameter->dStep;
	}
	else
	{
		Type=Linear;
		dMin=dMax=dStep=0;
	}
	SetValue(dValue);
}

void LinearParameter::SetValue(double val)
{
	if (val>dMax) val=dMax;
	else if (val<dMin) val=dMin;
	if (dStep==0) dValue=val;
	else
	{
		dValue=dMin+dStep*floor((val-dMin)/dStep+0.5);
		if (dValue>dMax) dValue=dValue-dStep;
	}
	bModified=true;
}

bool LinearParameter::IncreaseStep()
{
	if (dValue+dStep>dMax) return false;
	SetValue(dValue+dStep);
	return true;
}

void LinearParameter::PrintSelf(FILE* /*out*/)
{
	fprintf(stderr," Parameter: %s  Value: %f\n from %f to %f; Stepsize: %f\n",sName.c_str(),dValue,dMin,dMax,dStep);
}

bool LinearParameter::Write2XML(TiXmlNode& root)
{
	TiXmlElement para("Parameter");
	para.SetAttribute("Type","Linear");
	Parameter::Write2XML(para);
	para.SetDoubleAttribute("min",dMin);
	para.SetDoubleAttribute("max",dMax);
	para.SetDoubleAttribute("step",dStep);
	root.InsertEndChild(para);
	return true;
}

bool LinearParameter::ReadFromXML(TiXmlNode &root)
{
	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;

	if (elem->QueryDoubleAttribute("min",&dMin)!=TIXML_SUCCESS) return false;
	if (elem->QueryDoubleAttribute("max",&dMax)!=TIXML_SUCCESS) return false;
	if (elem->QueryDoubleAttribute("step",&dStep)!=TIXML_SUCCESS) return false;

	if (Parameter::ReadFromXML(root)==false) return false;
//	const char* att=elem->Attribute("name");
//	if (att==NULL) return false;
//	sName=string(att);

//	if (elem->QueryDoubleAttribute("value",&dValue)!=TIXML_SUCCESS) return false;

	return true;
}


ParameterSet::ParameterSet(void)
{
	bModified=true;
}

ParameterSet::~ParameterSet(void)
{
	clear();
}

size_t ParameterSet::LinkParameter(Parameter* newPara)
{
	vParameter.push_back(newPara);
//	newPara->ParameterSet(this);
	return vParameter.size();
}

size_t ParameterSet::DeleteParameter(size_t index)
{
	if (index>=vParameter.size()) return vParameter.size();
	std::vector<Parameter*>::iterator pIter=vParameter.begin();
	vParameter.erase(pIter+index);

	return vParameter.size();
}

size_t ParameterSet::DeleteParameter(Parameter* para)
{
	std::vector<Parameter*>::iterator pIter=vParameter.begin();
	while (pIter!=vParameter.end())
	{
		if (*pIter==para)
		{
			vParameter.erase(pIter);
			return vParameter.size();
		}
		++pIter;
	}
	return vParameter.size();
}

void ParameterSet::clear()
{
	for (size_t i=0; i<vParameter.size();++i)
	{
		delete vParameter.at(i);
	}
	vParameter.clear();
//	ParameterString.clear();
//	ParameterValueString.clear();
}

bool ParameterSet::GetModified()
{
	if (bModified==true) return true;
	for (size_t i=0; i<vParameter.size();++i) if (vParameter.at(i)->GetModified()==true) return true;
	return false;
}

void ParameterSet::SetModified(bool mod)
{
	if (mod==true)
	{
		bModified=true;
		return;
	}
	bModified=false;
	for (size_t i=0; i<vParameter.size();++i) vParameter.at(i)->SetModified(false);
}

double* ParameterSet::GetValueArray(double *array)
{
	if (array==NULL) return array;
	for (size_t i=0; i<vParameter.size();++i)
	{
		array[i]=vParameter.at(i)->GetValue();
	}
	return array;
}

int ParameterSet::CountSweepSteps(int SweepMode)
{
	int count=0;
	switch (SweepMode)
	{
		case 1:
			for (size_t i=0; i<vParameter.size();++i)
				if (vParameter.at(i)->GetSweep())
				{
					if (count==0) count=vParameter.at(i)->CountSteps();
					else count*=vParameter.at(i)->CountSteps();
				}
			return count;
			break;
		case 2:
			for (size_t i=0; i<vParameter.size();++i) if (vParameter.at(i)->GetSweep()) count+=vParameter.at(i)->CountSteps();
			return count;
			break;
	}
	return 0;
}

void ParameterSet::InitSweep()
{
	for (size_t i=0; i<vParameter.size();++i)
		if (vParameter.at(i)->GetSweep())
		{
			vParameter.at(i)->Save();
			vParameter.at(i)->InitSweep();
		}
	SweepPara=0;
}

void ParameterSet::EndSweep()
{
	for (size_t i=0; i<vParameter.size();++i)
		if (vParameter.at(i)->GetSweep())
		{
			vParameter.at(i)->Restore();
		}
}

bool ParameterSet::NextSweepPos(int SweepMode)
{
	switch (SweepMode)
	{
		case 1:
		{
			int level=vParameter.size()-1;
			while (level>=0)
			{
				if (vParameter.at(level)->GetSweep())
				{
					if (vParameter.at(level)->IncreaseStep()) return true;
					else
					{
						vParameter.at(level)->InitSweep();
						--level;
					}
				}
				else --level;
			}
			return false;
			break;
		}
		case 2:
			if ((unsigned int)SweepPara>=vParameter.size()) return false;
			if (vParameter.at(SweepPara)->GetSweep())
			{
				if (vParameter.at(SweepPara)->IncreaseStep()) return true;
				else ++SweepPara;
				return NextSweepPos(SweepMode);
			}
			else
			{
				++SweepPara;
				return NextSweepPos(SweepMode);
			}
			break;
	}
	return false;
}


const std::string ParameterSet::GetParameterString(const std::string spacer)
{
	std::string ParameterString;
	for (size_t i=0; i<vParameter.size();++i)
	{
		if (i>0) ParameterString+=spacer;
		ParameterString+=vParameter.at(i)->GetName();
	}
	return ParameterString;
}

const std::string ParameterSet::GetParameterValueString(const std::string spacer, bool ValuesOnly)
{
	std::string ParameterValueString;
	for (size_t i=0; i<vParameter.size();++i)
	{
		if (i>0) ParameterValueString+=spacer;
		if (!ValuesOnly)
			{
				ParameterValueString+=vParameter.at(i)->GetName();
				ParameterValueString+="=";
			}
		std::ostringstream os;
		os << vParameter.at(i)->GetValue();
		ParameterValueString+=os.str();
	}
	return ParameterValueString.c_str();
}

void ParameterSet::PrintSelf(FILE* out)
{
	fprintf(out," Parameter-Set Printout, Qty of Parameter: %d\n",(int)vParameter.size());
	for (size_t i=0; i<vParameter.size();++i)
	{
		fprintf(out,"----Nr. %d----\n",(int)i);
		vParameter.at(i)->PrintSelf(out);
	}
}

bool ParameterSet::Write2XML(TiXmlNode& root)
{
	TiXmlElement pSet("ParameterSet");
	//pSet.SetAttribute("QtyParameter",(int)vParameter.size());
	for (size_t i=0;i<vParameter.size();++i) vParameter.at(i)->Write2XML(pSet);

	root.InsertEndChild(pSet);
	return true;
}

bool ParameterSet::ReadFromXML(TiXmlNode &root)
{
	TiXmlNode* paraNode = root.FirstChild("Parameter");
	while (paraNode!=NULL)
	{
		TiXmlElement *elem = paraNode->ToElement();
		if (elem!=NULL)
		{
			const char* att=elem->Attribute("Type");
			if (att!=NULL)
			{
				Parameter* newPara=NULL;
				if (strcmp(att,"Const")==0) newPara = new Parameter();
				else if (strcmp(att,"Linear")==0) newPara = new LinearParameter();
				if (newPara!=NULL)
				{
					if (newPara->ReadFromXML(*elem)==true) this->InsertParameter(newPara);
				}
			}
		}
		paraNode=paraNode->NextSiblingElement("Parameter");
	};
	return true;
}


ParameterScalar::ParameterScalar()
{
	clParaSet=NULL;
	bModified=true;
	ParameterMode=false;
	sValue.clear();
	dValue=0;
}

ParameterScalar::ParameterScalar(ParameterSet* ParaSet, const std::string value)
{
	SetParameterSet(ParaSet);
	SetValue(value);
}

ParameterScalar::ParameterScalar(ParameterSet* ParaSet, double value)
{
	SetParameterSet(ParaSet);
	bModified=true;
	SetValue(value);
}

ParameterScalar::ParameterScalar(ParameterScalar* ps)
{
	Copy(ps);
}

ParameterScalar::~ParameterScalar()
{
}

void ParameterScalar::SetParameterSet(ParameterSet *paraSet)
{
	clParaSet=paraSet;
}

int ParameterScalar::SetValue(const std::string value, bool Eval)
{
	if (value.empty()) return -1;

	//check if string is only a plain double
	char *pEnd;
	double val = strtod(value.c_str(),&pEnd);
	if (*pEnd == 0)
	{
		SetValue(val);
		return 0;
	}

	ParameterMode=true;
	bModified=true;
	sValue=value;

	if (Eval) return Evaluate();

	return 0;
}

void ParameterScalar::SetValue(double value)
{
	ParameterMode=false;
	dValue=value;
	sValue.clear();
}

double ParameterScalar::GetValue() const
{
	return dValue;
}

const std::string ParameterScalar::GetValueString() const
{
	if (ParameterMode)
		return sValue;
	std::stringstream numString;
	numString << dValue;
	return numString.str();
}

int ParameterScalar::Evaluate()
{
	if (ParameterMode==false) return 0;
	if (clParaSet!=NULL)
		bModified = bModified || clParaSet->GetModified();
	if (bModified==false)
		return 0;

	CSFunctionParser fParse;
	dValue=0;

	if (clParaSet!=NULL)
		fParse.Parse(sValue,clParaSet->GetParameterString());
	else
		fParse.Parse(sValue,"");

	if (fParse.GetParseErrorType()!=FunctionParser::FP_NO_ERROR) return fParse.GetParseErrorType()+100;
	bModified=false;

	if (clParaSet!=NULL)
	{
		double *vars = new double[clParaSet->GetQtyParameter()];
		vars=clParaSet->GetValueArray(vars);
		dValue=fParse.Eval(vars);
		delete[] vars;vars=NULL;
	}
	else
		dValue=fParse.Eval(NULL);
	return fParse.EvalError();
}

double ParameterScalar::GetEvaluated(double* ParaValues, int &EC)
{
	if (ParameterMode==false) return dValue;
	CSFunctionParser fParse;
	fParse.Parse(sValue,clParaSet->GetParameterString());
	if (fParse.GetParseErrorType()!=FunctionParser::FP_NO_ERROR)
	{
		EC = fParse.GetParseErrorType()+100;
		return 0;
	}
	double dvalue = fParse.Eval(ParaValues);
	EC = fParse.EvalError();
	return dvalue;
}

void ParameterScalar::Copy(ParameterScalar* ps)
{
	SetParameterSet(ps->clParaSet);
	bModified=ps->bModified;
	ParameterMode=ps->ParameterMode;
	sValue=std::string(ps->sValue);
	dValue=ps->dValue;
}

std::string PSErrorCode2Msg(int code)
{
	std::string msg;
	PSErrorCode2Msg(code,&msg);
	return msg;
}

void PSErrorCode2Msg(int code, std::string *msg)
{
	switch (code)
	{
	case -1:
		msg->append("Internal Error");
		break;
	case 0:
		msg->append("No Error");
		break;
	case 1:
		msg->append("Division by zero");
		break;
	case 2:
		msg->append("sqrt error (sqrt of a negative value)");
		break;
	case 3:
		msg->append("log error (logarithm of a negative value)");
		break;
	case 4:
		msg->append("trigonometric error (asin or acos of illegal value)");
		break;
	case 5:
		msg->append("Maximum recursion level reached");
		break;
	case 100:
		msg->append("Syntax error (check Parameter)");
		break;
	case 101:
		msg->append("Mismatched parenthesis");
		break;
	case 102:
		msg->append("Missing \')\'");
		break;
	case 103:
		msg->append("Empty parentheses");
		break;
	case 104:
		msg->append("Syntax error: Operator expected");
		break;
	case 105:
		msg->append("Not enough memory");
		break;
	case 106:
		msg->append("An unexpected error occurred");
		break;
	case 107:
		msg->append("Syntax error in parameters");
		break;
	case 108:
		msg->append("Illegal number of parameters to function");
		break;
	case 109:
		msg->append("Syntax error: Premature end of string");
		break;
	case 110:
		msg->append("Syntax error: Expecting ( after function");
		break;
	};
}
