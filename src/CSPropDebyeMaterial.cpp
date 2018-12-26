/*
*	Copyright (C) 2013 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#include "CSPropDebyeMaterial.h"

CSPropDebyeMaterial::CSPropDebyeMaterial(ParameterSet* paraSet) : CSPropDispersiveMaterial(paraSet) {Type=(CSProperties::PropertyType)(DEBYEMATERIAL | DISPERSIVEMATERIAL | MATERIAL);Init();}
CSPropDebyeMaterial::CSPropDebyeMaterial(CSProperties* prop) : CSPropDispersiveMaterial(prop) {Type=(CSProperties::PropertyType)(DEBYEMATERIAL | DISPERSIVEMATERIAL | MATERIAL);Init();}
CSPropDebyeMaterial::CSPropDebyeMaterial(unsigned int ID, ParameterSet* paraSet) : CSPropDispersiveMaterial(ID,paraSet) {Type=(CSProperties::PropertyType)(DEBYEMATERIAL | DISPERSIVEMATERIAL | MATERIAL);Init();}

CSPropDebyeMaterial::~CSPropDebyeMaterial()
{
	DeleteValues();
	m_Order = 0;
}

void CSPropDebyeMaterial::Init()
{
	m_Order = 0;
	EpsDelta=NULL;
	WeightEpsDelta=NULL;
	EpsRelaxTime=NULL;
	WeightEpsRelaxTime=NULL;
	InitValues();
	CSPropDispersiveMaterial::Init();
}

void CSPropDebyeMaterial::DeleteValues()
{
	for (int o=0;o<m_Order;++o)
	{
		delete[] EpsDelta[o];
		delete[] WeightEpsDelta[o];
		delete[] EpsRelaxTime[o];
		delete[] WeightEpsRelaxTime[o];
	}
	delete[] EpsDelta;
	delete[] WeightEpsDelta;
	delete[] EpsRelaxTime;
	delete[] WeightEpsRelaxTime;

	EpsDelta=NULL;
	WeightEpsDelta=NULL;
	EpsRelaxTime=NULL;
	WeightEpsRelaxTime=NULL;
}

void CSPropDebyeMaterial::InitValues()
{
//	DeleteValues();
	EpsDelta=new ParameterScalar*[m_Order];
	WeightEpsDelta=new ParameterScalar*[m_Order];
	EpsRelaxTime=new ParameterScalar*[m_Order];
	WeightEpsRelaxTime=new ParameterScalar*[m_Order];

	for (int o=0;o<m_Order;++o)
	{
		EpsDelta[o] = new ParameterScalar[3];
		WeightEpsDelta[o] = new ParameterScalar[3];
		EpsRelaxTime[o] = new ParameterScalar[3];
		WeightEpsRelaxTime[o] = new ParameterScalar[3];

		for (int n=0;n<3;++n)
		{
			EpsDelta[o][n].SetValue(0);
			EpsDelta[o][n].SetParameterSet(clParaSet);
			WeightEpsDelta[o][n].SetValue(1);
			WeightEpsDelta[o][n].SetParameterSet(coordParaSet);
			EpsRelaxTime[o][n].SetValue(0);
			EpsRelaxTime[o][n].SetParameterSet(clParaSet);
			WeightEpsRelaxTime[o][n].SetValue(1);
			WeightEpsRelaxTime[o][n].SetParameterSet(coordParaSet);
		}
	}
}


bool CSPropDebyeMaterial::Update(std::string *ErrStr)
{
	bool bOK=true;
	int EC=0;
	for (int o=0;o<m_Order;++o)
	{
		for (int n=0;n<3;++n)
		{
			EC=EpsDelta[o][n].Evaluate();
			if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
			if ((EC!=ParameterScalar::PS_NO_ERROR) && (ErrStr!=NULL))
			{
				std::stringstream stream;
				stream << std::endl << "Error in Debye Material-Property epsilon Delta frequency value (ID: " << uiID << "): ";
				ErrStr->append(stream.str());
				PSErrorCode2Msg(EC,ErrStr);
			}

			EC=WeightEpsDelta[o][n].Evaluate();
			if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
			if ((EC!=ParameterScalar::PS_NO_ERROR) && (ErrStr!=NULL))
			{
				std::stringstream stream;
				stream << std::endl << "Error in Debye Material-Property epsilon Delta frequency weighting function (ID: " << uiID << "): ";
				ErrStr->append(stream.str());
				PSErrorCode2Msg(EC,ErrStr);
			}

			EC=EpsRelaxTime[o][n].Evaluate();
			if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
			if ((EC!=ParameterScalar::PS_NO_ERROR) && (ErrStr!=NULL))
			{
				std::stringstream stream;
				stream << std::endl << "Error in Debye Material-Property epsilon relaxation time value (ID: " << uiID << "): ";
				ErrStr->append(stream.str());
				PSErrorCode2Msg(EC,ErrStr);
			}

			EC=WeightEpsRelaxTime[o][n].Evaluate();
			if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
			if ((EC!=ParameterScalar::PS_NO_ERROR) && (ErrStr!=NULL))
			{
				std::stringstream stream;
				stream << std::endl << "Error in Debye Material-Property epsilon relaxation time weighting function (ID: " << uiID << "): ";
				ErrStr->append(stream.str());
				PSErrorCode2Msg(EC,ErrStr);
			}
		}
	}
	return bOK & CSPropDispersiveMaterial::Update(ErrStr);
}

bool CSPropDebyeMaterial::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSPropDispersiveMaterial::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	std::string suffix;
	for (int o=0;o<m_Order;++o)
	{
		suffix = ConvertInt(o+1);
		TiXmlElement* value=prop->FirstChildElement("Property");
		if (value==NULL)
			return false;
		WriteVectorTerm(EpsDelta[o],*value,"EpsilonDelta_"+suffix,parameterised);
		WriteVectorTerm(EpsRelaxTime[o],*value,"EpsilonRelaxTime_"+suffix,parameterised);

		TiXmlElement* weight=prop->FirstChildElement("Weight");
		if (weight==NULL)
			return false;
		WriteVectorTerm(WeightEpsDelta[o],*weight,"EpsilonDelta_"+suffix,parameterised);
		WriteVectorTerm(WeightEpsRelaxTime[o],*weight,"EpsilonRelaxTime_"+suffix,parameterised);
	}
	return true;
}

bool CSPropDebyeMaterial::ReadFromXML(TiXmlNode &root)
{
	if (CSPropDispersiveMaterial::ReadFromXML(root)==false) return false;
	TiXmlElement* prop=root.ToElement();

	if (prop==NULL) return false;

	// count m_Order
	TiXmlElement* matProp=prop->FirstChildElement("Property");
	if (matProp!=NULL)
	{
		m_Order=1;
		while (1)
		{
			if (matProp->Attribute("EpsilonDelta_"+ConvertInt(m_Order+1)))
				++m_Order;
			else if (matProp->Attribute("MueDelta_"+ConvertInt(m_Order+1)))
				++m_Order;
			else
				break;
		}
	}
	else
		return false;

	InitValues();

	if (ReadVectorTerm(EpsDelta[0],*matProp,"EpsilonDelta_1",0.0)==false)
		ReadVectorTerm(EpsDelta[0],*matProp,"EpsilonDelta",0.0);

	if (ReadVectorTerm(EpsRelaxTime[0],*matProp,"EpsilonRelaxTime_1",0.0)==false)
		ReadVectorTerm(EpsRelaxTime[0],*matProp,"EpsilonRelaxTime",0.0);

	TiXmlElement* weightProp=prop->FirstChildElement("Weight");
	if (weightProp)
	{
		if (ReadVectorTerm(WeightEpsDelta[0],*weightProp,"EpsilonDelta_1",1.0)==false)
			ReadVectorTerm(WeightEpsDelta[0],*weightProp,"EpsilonDelta",1.0);

		if (ReadVectorTerm(WeightEpsRelaxTime[0],*weightProp,"EpsilonRelaxTime_1",1.0)==false)
			ReadVectorTerm(WeightEpsRelaxTime[0],*weightProp,"EpsilonRelaxTime",1.0);
	}

	for (int o=1;o<m_Order;++o)
	{
		ReadVectorTerm(EpsDelta[o],*matProp,"EpsilonDelta_"+ConvertInt(o+1),0.0);

		ReadVectorTerm(EpsRelaxTime[o],*matProp,"EpsilonRelaxTime_"+ConvertInt(o+1),0.0);

		if (weightProp)
		{
			ReadVectorTerm(WeightEpsDelta[o],*weightProp,"EpsilonDelta_"+ConvertInt(o+1),1.0);

			ReadVectorTerm(WeightEpsRelaxTime[o],*weightProp,"EpsilonRelaxTime_"+ConvertInt(o+1),1.0);
		}
	}
	return true;
}

void CSPropDebyeMaterial::ShowPropertyStatus(std::ostream& stream)
{
	CSPropDispersiveMaterial::ShowPropertyStatus(stream);
	stream << "  Debye model order:\t" << m_Order << std::endl;
	for (int o=0;o<m_Order;++o)
	{
		stream << " Epsilon Delta #" << o << ":\t" << GetEpsDelta(o,0) << "," << GetEpsDelta(o,1) << "," << GetEpsDelta(o,2) << std::endl;
		stream << " Epsilon Relax Time #" << o << ":\t" << GetEpsRelaxTime(o,0) << "," << GetEpsRelaxTime(o,1) << "," << GetEpsRelaxTime(o,2) << std::endl;
	}
}
