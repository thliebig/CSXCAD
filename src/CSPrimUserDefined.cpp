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

#include <sstream>
#include <iostream>
#include <limits>
#include "tinyxml.h"
#include "stdint.h"

#include "CSPrimUserDefined.h"
#include "CSProperties.h"
#include "CSFunctionParser.h"
#include "CSUseful.h"

CSPrimUserDefined::CSPrimUserDefined(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=USERDEFINED;
	fParse = new CSFunctionParser();
	stFunction = std::string();
	CoordSystem=CARESIAN_SYSTEM;
	for (int i=0;i<3;++i) {dPosShift[i].SetParameterSet(paraSet);}
	PrimTypeName = std::string("User-Defined");
}

CSPrimUserDefined::CSPrimUserDefined(CSPrimUserDefined* primUDef, CSProperties *prop) : CSPrimitives(primUDef,prop)
{
	Type=USERDEFINED;
	fParse = new CSFunctionParser(*primUDef->fParse);
	stFunction = std::string(primUDef->stFunction);
	CoordSystem = primUDef->CoordSystem;
	for (int i=0;i<3;++i)
		dPosShift[i].Copy(&primUDef->dPosShift[i]);
	PrimTypeName = std::string("User-Defined");
}

CSPrimUserDefined::CSPrimUserDefined(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=USERDEFINED;
	fParse = new CSFunctionParser();
	stFunction = std::string();
	CoordSystem=CARESIAN_SYSTEM;
	for (int i=0;i<3;++i)
		dPosShift[i].SetParameterSet(paraSet);
	PrimTypeName = std::string("User-Defined");
}


CSPrimUserDefined::~CSPrimUserDefined()
{
	delete fParse;fParse=NULL;
}

void CSPrimUserDefined::SetCoordSystem(UserDefinedCoordSystem newSystem)
{
	CoordSystem=newSystem;
}

void CSPrimUserDefined::SetFunction(const char* func)
{
	if (func==NULL) return;
	stFunction = std::string(func);
}

bool CSPrimUserDefined::GetBoundBox(double dBoundBox[6], bool PreserveOrientation)
{
	UNUSED(PreserveOrientation); //has no orientation or preserved anyways
	//this type has no simple bound box
	double max=std::numeric_limits<double>::max();
	dBoundBox[0]=dBoundBox[2]=dBoundBox[4]=-max;
	dBoundBox[1]=dBoundBox[3]=dBoundBox[5]=max;
	bool accurate=false;
	return accurate;
}

bool CSPrimUserDefined::IsInside(const double* Coord, double /*tol*/)
{
	if (Coord==NULL) return false;

	int NrPara=clParaSet->GetQtyParameter();
	if (NrPara!=iQtyParameter) return false;
	double *vars = new double[NrPara+6];

	vars=clParaSet->GetValueArray(vars);

	double inCoord[3] = {Coord[0],Coord[1],Coord[2]};
	//transform incoming coordinates into cartesian coords
	TransformCoordSystem(Coord,inCoord,m_MeshType,CARTESIAN);
	if (m_Transform)
		m_Transform->InvertTransform(inCoord,inCoord);

	double x=inCoord[0]-dPosShift[0].GetValue();
	double y=inCoord[1]-dPosShift[1].GetValue();
	double z=inCoord[2]-dPosShift[2].GetValue();
	double rxy=sqrt(x*x+y*y);
	vars[NrPara]=x;
	vars[NrPara+1]=y;
	vars[NrPara+2]=z;

	switch (CoordSystem)
	{
	case CARESIAN_SYSTEM:  //uses x,y,z
		vars[NrPara+3]=0;
		vars[NrPara+4]=0;
		vars[NrPara+5]=0;		break;
	case CYLINDER_SYSTEM: //uses x,y,z,r,a,0
		vars[NrPara+3]=rxy;
		vars[NrPara+4]=atan2(y,x);
		vars[NrPara+5]=0;
		break;
	case SPHERE_SYSTEM:   //uses x,y,z,r,a,t
		vars[NrPara+3]=sqrt(x*x+y*y+z*z);
		vars[NrPara+4]=atan2(y,x);
		vars[NrPara+5]=asin(1)-atan(z/rxy);
		//cout << "x::" << x << "y::" << y << "z::" << z << "r::" << vars[NrPara] << "a::" << vars[NrPara+1] << "t::" << vars[NrPara+2] << std::endl;
		break;
	default:
		//unknown System
		return false;
		break;
	}
	double dValue=0;

	if (fParse->GetParseErrorType()==FunctionParser::FP_NO_ERROR) dValue=fParse->Eval(vars);
	else dValue=0;
	delete[] vars;vars=NULL;

	if (dValue==1)
		return true;
	return false;
}


bool CSPrimUserDefined::Update(std::string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	std::string vars;
	switch (CoordSystem)
	{
	case CARESIAN_SYSTEM:
		vars=std::string("x,y,z");
		break;
	case CYLINDER_SYSTEM:
		vars=std::string("x,y,z,r,a");
		break;
	case SPHERE_SYSTEM:
		vars=std::string("x,y,z,r,a,t");
		break;
	default:
		//unknown System
		return false;
		break;
	}
	iQtyParameter=clParaSet->GetQtyParameter();
	if (iQtyParameter>0)
	{
		fParameter=std::string(clParaSet->GetParameterString());
		vars = fParameter + "," + vars;
	}

	fParse->Parse(stFunction,vars);

	EC=fParse->GetParseErrorType();
	//cout << fParse.ErrorMsg();

	if (EC!=FunctionParser::FP_NO_ERROR) bOK=false;

	if ((EC!=FunctionParser::FP_NO_ERROR)  && (ErrStr!=NULL))
	{
		std::ostringstream oss;
		oss << "\nError in User Defined Primitive Function (ID: " << uiID << "): " << fParse->ErrorMsg();
		ErrStr->append(oss.str());
		bOK=false;
	}

	for (int i=0;i<3;++i)
	{
		EC=dPosShift[i].Evaluate();
		if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
		{
			bOK=false;
			std::ostringstream oss;
			oss << "\nError in User Defined Primitive Coord (ID: " << uiID << "): ";
			ErrStr->append(oss.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}

	//update local bounding box
	m_BoundBoxValid = GetBoundBox(m_BoundBox);

	return bOK;
}

bool CSPrimUserDefined::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimitives::Write2XML(elem,parameterised);

	elem.SetAttribute("CoordSystem",CoordSystem);

	TiXmlElement P1("CoordShift");
	WriteTerm(dPosShift[0],P1,"X",parameterised);
	WriteTerm(dPosShift[1],P1,"Y",parameterised);
	WriteTerm(dPosShift[2],P1,"Z",parameterised);
	elem.InsertEndChild(P1);

	TiXmlElement FuncElem("Function");
	TiXmlText FuncText(GetFunction());
	FuncElem.InsertEndChild(FuncText);

	elem.InsertEndChild(FuncElem);
	return true;
}

bool CSPrimUserDefined::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;

	int value;
	TiXmlElement* elem=root.ToElement();
	if (elem==NULL) return false;
	if (elem->QueryIntAttribute("CoordSystem",&value)!=TIXML_SUCCESS) return false;
	SetCoordSystem((UserDefinedCoordSystem)value);

	//P1
	TiXmlElement* P1=root.FirstChildElement("CoordShift");
	if (P1==NULL) return false;
	if (ReadTerm(dPosShift[0],*P1,"X")==false) return false;
	if (ReadTerm(dPosShift[1],*P1,"Y")==false) return false;
	if (ReadTerm(dPosShift[2],*P1,"Z")==false) return false;

	TiXmlElement* FuncElem=root.FirstChildElement("Function");
	if (FuncElem==NULL) return false;
	SetFunction(FuncElem->GetText());

	return true;
}
