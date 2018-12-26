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

#include "CSPropMaterial.h"

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

std::string CSPropMaterial::GetTerm(ParameterScalar *ps, int ny)
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

int CSPropMaterial::SetValue(std::string val, ParameterScalar *ps, int ny)
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
		std::cerr << "CSPropMaterial::GetWeight: Error evaluating the weighting function (ID: " << this->GetID() << "): " << PSErrorCode2Msg(EC) << std::endl;
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
	Density.SetValue(0);
	WeightDensity.SetValue(1.0);
	FillColor.a=EdgeColor.a=123;
	bVisisble=true;
}

bool CSPropMaterial::Update(std::string *ErrStr)
{
	bool bOK=true;
	int EC=0;
	for (int n=0;n<3;++n)
	{
		EC=Epsilon[n].Evaluate();
		if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::PS_NO_ERROR) && (ErrStr!=NULL))
		{
			std::stringstream stream;
			stream << std::endl << "Error in Material-Property Epsilon-Value (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=Mue[n].Evaluate();
		if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::PS_NO_ERROR) && (ErrStr!=NULL))
		{
			std::stringstream stream;
			stream << std::endl << "Error in Material-Property Mue-Value (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=Kappa[n].Evaluate();
		if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::PS_NO_ERROR) && (ErrStr!=NULL))
		{
			std::stringstream stream;
			stream << std::endl << "Error in Material-Property Kappa-Value (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=Sigma[n].Evaluate();
		if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::PS_NO_ERROR) && (ErrStr!=NULL))
		{
			std::stringstream stream;
			stream << std::endl << "Error in Material-Property Sigma-Value (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=WeightEpsilon[n].Evaluate();
		if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::PS_NO_ERROR) && (ErrStr!=NULL))
		{
			std::stringstream stream;
			stream << std::endl << "Error in Material-Property Epsilon-Value weighting function (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=WeightMue[n].Evaluate();
		if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::PS_NO_ERROR) && (ErrStr!=NULL))
		{
			std::stringstream stream;
			stream << std::endl << "Error in Material-Property Mue-Value weighting function  (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=WeightKappa[n].Evaluate();
		if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::PS_NO_ERROR) && (ErrStr!=NULL))
		{
			std::stringstream stream;
			stream << std::endl << "Error in Material-Property Kappa-Value weighting function  (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
		EC=WeightSigma[n].Evaluate();
		if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::PS_NO_ERROR) && (ErrStr!=NULL))
		{
			std::stringstream stream;
			stream << std::endl << "Error in Material-Property Sigma-Value weighting function  (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}

	EC=Density.Evaluate();
	if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::PS_NO_ERROR) && (ErrStr!=NULL))
	{
		std::stringstream stream;
		stream << std::endl << "Error in Material-Property Density-Value (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}
	EC=WeightDensity.Evaluate();
	if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::PS_NO_ERROR) && (ErrStr!=NULL))
	{
		std::stringstream stream;
		stream << std::endl << "Error in Material-Property Density weighting function (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

	return bOK;
}

bool CSPropMaterial::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSProperties::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	prop->SetAttribute("Isotropy",bIsotropy);

	/***************   3D - Properties *****************/
	TiXmlElement value("Property");
	WriteVectorTerm(Epsilon,value,"Epsilon",parameterised);
	WriteVectorTerm(Mue,value,"Mue",parameterised);
	WriteVectorTerm(Kappa,value,"Kappa",parameterised);
	WriteVectorTerm(Sigma,value,"Sigma",parameterised);
	/***************   1D - Properties *****************/
	WriteTerm(Density,value,"Density",parameterised);
	prop->InsertEndChild(value);

	/**********   3D - Properties  Weight **************/
	TiXmlElement Weight("Weight");
	WriteVectorTerm(WeightEpsilon,Weight,"Epsilon",parameterised);
	WriteVectorTerm(WeightMue,Weight,"Mue",parameterised);
	WriteVectorTerm(WeightKappa,Weight,"Kappa",parameterised);
	WriteVectorTerm(WeightSigma,Weight,"Sigma",parameterised);
	/**********   1D - Properties  Weight **************/
	WriteTerm(WeightDensity,Weight,"Density",parameterised);
	prop->InsertEndChild(Weight);

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

	/***************   3D - Properties *****************/
	TiXmlElement* matProp=prop->FirstChildElement("Property");
	if (matProp!=NULL)
	{
		ReadVectorTerm(Epsilon,*matProp,"Epsilon",1.0);
		ReadVectorTerm(Mue,*matProp,"Mue",1.0);
		ReadVectorTerm(Kappa,*matProp,"Kappa");
		ReadVectorTerm(Sigma,*matProp,"Sigma");
		ReadTerm(Density,*matProp,"Density",0.0);
	}

	/**********   3D - Properties  Weight **************/
	TiXmlElement *weight = prop->FirstChildElement("Weight");
	if (weight!=NULL)
	{
		ReadVectorTerm(WeightEpsilon,*weight,"Epsilon",1.0);
		ReadVectorTerm(WeightMue,*weight,"Mue",1.0);
		ReadVectorTerm(WeightKappa,*weight,"Kappa",1.0);
		ReadVectorTerm(WeightSigma,*weight,"Sigma",1.0);
		ReadTerm(WeightDensity,*weight,"Density",1.0);
	}

	return true;
}

void CSPropMaterial::ShowPropertyStatus(std::ostream& stream)
{
	CSProperties::ShowPropertyStatus(stream);
	stream << " --- " << GetTypeString() << " --- " << std::endl;
	stream << "  Isotropy\t: " << bIsotropy << std::endl;
	stream << "  Epsilon_R\t: " << Epsilon[0].GetValueString() << ", "  << Epsilon[1].GetValueString() << ", "  << Epsilon[2].GetValueString()  << std::endl;
	stream << "  Kappa\t\t: " << Kappa[0].GetValueString() << ", "  << Kappa[1].GetValueString() << ", "  << Kappa[2].GetValueString()  << std::endl;
	stream << "  Mue_R\t\t: " << Mue[0].GetValueString() << ", "  << Mue[1].GetValueString() << ", "  << Mue[2].GetValueString()  << std::endl;
	stream << "  Sigma\t\t: " << Sigma[0].GetValueString() << ", "  << Sigma[1].GetValueString() << ", "  << Sigma[2].GetValueString()  << std::endl;
	stream << "  Density\t: " << Density.GetValueString() << std::endl;

}
