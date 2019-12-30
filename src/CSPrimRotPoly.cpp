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
#define _USE_MATH_DEFINES
#include <math.h>

#include "CSPrimRotPoly.h"
#include "CSProperties.h"
#include "CSUseful.h"

CSPrimRotPoly::CSPrimRotPoly(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimPolygon(ID,paraSet,prop)
{
	Type=ROTPOLY;
	m_RotAxisDir=0;
	PrimTypeName = std::string("RotPoly");
}

CSPrimRotPoly::CSPrimRotPoly(CSPrimRotPoly* primRotPoly, CSProperties *prop) : CSPrimPolygon(primRotPoly,prop)
{
	Type=ROTPOLY;
	m_RotAxisDir=primRotPoly->m_RotAxisDir;
	PrimTypeName = std::string("RotPoly");
}

CSPrimRotPoly::CSPrimRotPoly(ParameterSet* paraSet, CSProperties* prop) : CSPrimPolygon(paraSet,prop)
{
	Type=ROTPOLY;
	m_RotAxisDir=0;
	PrimTypeName = std::string("RotPoly");
}

CSPrimRotPoly::~CSPrimRotPoly()
{
}

bool CSPrimRotPoly::IsInside(const double* inCoord, double /*tol*/)
{
	if (inCoord==NULL) return false;

	double Coord[3];
	//transform incoming coordinates into cartesian coords
	TransformCoordSystem(inCoord,Coord,m_MeshType,CARTESIAN);
	if (m_Transform && Type==ROTPOLY)
		TransformCoords(Coord,true, CARTESIAN);

	double origin[3]={0,0,0};
	double dir[3]={0,0,0};
	dir[m_RotAxisDir] = 1;
	double foot;
	double dist;
	Point_Line_Distance(Coord, origin, dir, foot, dist);

	int raP = (m_RotAxisDir+1)%3;
	int raPP = (m_RotAxisDir+2)%3;
	double alpha = atan2(Coord[raPP],Coord[raP]);
	if (raP == m_NormDir)
		alpha=alpha-M_PI/2;
	if (alpha<0)
		alpha+=2*M_PI;

	origin[0] = dist;origin[1] = dist;origin[2] = dist;
	origin[m_NormDir] = 0;
	origin[m_RotAxisDir] = foot;

	if (alpha<m_StartStopAng[0])
		alpha+=2*M_PI;

	if ((CSPrimPolygon::IsInside(origin)) && (alpha<m_StartStopAng[1]))
		return true;

	dist*=-1;
	alpha=alpha+M_PI;
	if (alpha>2*M_PI)
		alpha-=2*M_PI;

	if (alpha<m_StartStopAng[0])
		alpha+=2*M_PI;

	if (alpha>m_StartStopAng[1])
		return false;

	origin[0] = dist;origin[1] = dist;origin[2] = dist;
	origin[m_NormDir] = 0;
	origin[m_RotAxisDir] = foot;
	return CSPrimPolygon::IsInside(origin);
}


bool CSPrimRotPoly::Update(std::string *ErrStr)
{
	int EC=0;
	bool bOK = CSPrimPolygon::Update(ErrStr);

	EC=StartStopAngle[0].Evaluate();
	if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		std::stringstream stream;
		stream << std::endl << "Error in RotPoly Start Angle (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

	EC=StartStopAngle[1].Evaluate();
	if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		std::stringstream stream;
		stream << std::endl << "Error in RotPoly Stop Angle (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

	// make angle range to 0..2*M_PI & stop > start!
	m_StartStopAng[0]=StartStopAngle[0].GetValue();
	m_StartStopAng[1]=StartStopAngle[1].GetValue();
	if (m_StartStopAng[0]>m_StartStopAng[1])
		m_StartStopAng[1]+=2*M_PI;
	if (m_StartStopAng[0]>2*M_PI)
	{
		m_StartStopAng[0]-=2*M_PI;
		m_StartStopAng[1]-=2*M_PI;
	}
	if (m_StartStopAng[0]<0)
	{
		m_StartStopAng[0]+=2*M_PI;
		m_StartStopAng[1]+=2*M_PI;
	}

	//update local bounding box
	m_BoundBoxValid = GetBoundBox(m_BoundBox);

	return bOK;
}

bool CSPrimRotPoly::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimPolygon::Write2XML(elem,parameterised);

	elem.SetAttribute("RotAxisDir",m_RotAxisDir);

	TiXmlElement Ang("Angles");
	WriteTerm(StartStopAngle[0],Ang,"Start",parameterised);
	WriteTerm(StartStopAngle[1],Ang,"Stop",parameterised);
	elem.InsertEndChild(Ang);
	return true;
}

bool CSPrimRotPoly::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimPolygon::ReadFromXML(root)==false) return false;

	if (Elevation.GetValue()!=0)
		std::cerr << __func__ << ": Warning: An elevation for a rotational poly is not supported! Skipping!" << std::endl;
	Elevation.SetValue(0);

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;

	int help;
	if (elem->QueryIntAttribute("RotAxisDir",&help)!=TIXML_SUCCESS)
		 return false;
	m_RotAxisDir=help;

	TiXmlElement *NV=elem->FirstChildElement("Angles");
	if (NV==NULL) return false;
	if (ReadTerm(StartStopAngle[0],*NV,"Start")==false) return false;
	if (ReadTerm(StartStopAngle[1],*NV,"Stop")==false) return false;

	return true;
}
