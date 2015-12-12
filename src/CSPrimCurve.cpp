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

#include "CSPrimCurve.h"
#include "CSProperties.h"
#include "CSUseful.h"

CSPrimCurve::CSPrimCurve(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=CURVE;
	PrimTypeName = std::string("Curve");
}

CSPrimCurve::CSPrimCurve(CSPrimCurve* primCurve, CSProperties *prop) : CSPrimitives(primCurve,prop)
{
	Type=CURVE;
	for (size_t i=0;i<primCurve->points.size();++i)
		points.push_back(new ParameterCoord(primCurve->points.at(i)));
	PrimTypeName = std::string("Curve");
}

CSPrimCurve::CSPrimCurve(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=CURVE;
	PrimTypeName = std::string("Curve");
}


CSPrimCurve::~CSPrimCurve()
{
	points.clear();
}

size_t CSPrimCurve::AddPoint(double coords[])
{
	points.push_back(new ParameterCoord(clParaSet,coords));
	return points.size();
}

void CSPrimCurve::SetCoord(size_t point_index, int nu, double val)
{
	if (point_index>=GetNumberOfPoints()) return;
	if ((nu<0) || (nu>2)) return;
	points.at(point_index)->SetValue(nu,val);
}

void CSPrimCurve::SetCoord(size_t point_index, int nu, std::string val)
{
	if (point_index>=GetNumberOfPoints()) return;
	if ((nu<0) || (nu>2)) return;
	points.at(point_index)->SetValue(nu,val);
}

bool CSPrimCurve::GetPoint(size_t point_index, double point[3])
{
	if (point_index>=GetNumberOfPoints()) return false;
	point[0] = points.at(point_index)->GetValue(0);
	point[1] = points.at(point_index)->GetValue(1);
	point[2] = points.at(point_index)->GetValue(2);
	return true;
}

bool CSPrimCurve::GetPoint(size_t point_index, double* point, CoordinateSystem c_system, bool transform)
{
	if (point_index>=GetNumberOfPoints()) return false;
	point[0] = points.at(point_index)->GetCoordValue(0,c_system);
	point[1] = points.at(point_index)->GetCoordValue(1,c_system);
	point[2] = points.at(point_index)->GetCoordValue(2,c_system);
	if (transform)
		TransformCoords(point, false, c_system);
	return true;
}

void CSPrimCurve::ClearPoints()
{
	points.clear();
}

bool CSPrimCurve::GetBoundBox(double dBoundBox[6], bool /*PreserveOrientation*/)
{
//	cerr << "CSPrimCurve::GetBoundBox: Warning: The bounding box for this object is not calculated properly... " << std::endl;
	bool accurate=false;
	m_BoundBox_CoordSys = CARTESIAN;
	for (int n=0;n<6;++n) dBoundBox[n] = 0;
	for (size_t i=0;i<points.size();++i)
	{
		if (i==0)
		{
			for (int n=0;n<3;++n)
			{
				dBoundBox[2*n]=points.at(0)->GetCoordValue(n,CARTESIAN);
				dBoundBox[2*n+1]=dBoundBox[2*n];
			}
		}
		for (int n=0;n<3;++n)
		{
			if (points.at(i)->GetValue(n)<dBoundBox[2*n])
				dBoundBox[2*n]=points.at(i)->GetCoordValue(n,CARTESIAN);
			else if (points.at(i)->GetValue(n)>dBoundBox[2*n+1])
				dBoundBox[2*n+1]=points.at(i)->GetCoordValue(n,CARTESIAN);
		}
	}
	if (points.size()<=1)
		m_Dimension=0;
	else
		m_Dimension=1;
	return accurate;
}

bool CSPrimCurve::IsInside(const double* /*Coord*/, double /*tol*/)
{
	//this is a 1D-object, you can never be inside...
	return false;
}


bool CSPrimCurve::Update(std::string *ErrStr)
{
	bool bOK=true;
	for (size_t i=0;i<GetNumberOfPoints();++i)
	{
		bool isOK = points.at(i)->Evaluate(ErrStr);
		if (isOK==false)
		{
			std::stringstream stream;
			stream << std::endl << "Error in " << PrimTypeName << " (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
		}
		points.at(i)->SetCoordinateSystem(m_PrimCoordSystem, m_MeshType);
		bOK &= isOK;
	}

	//update local bounding box
	m_BoundBoxValid = GetBoundBox(m_BoundBox);

	return bOK;
}

bool CSPrimCurve::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimitives::Write2XML(elem,parameterised);

	for (size_t i=0;i<points.size();++i)
	{
		TiXmlElement VT("Vertex");
		points.at(i)->Write2XML(&VT,parameterised);
		elem.InsertEndChild(VT);
	}
	return true;
}

bool CSPrimCurve::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;

	TiXmlElement *VT=root.FirstChildElement("Vertex");
	if (points.size()!=0) return false;
	ParameterCoord *newPoint;
	while (VT)
	{
		newPoint = new ParameterCoord(clParaSet);
		if (newPoint->ReadFromXML(VT))
			points.push_back(newPoint);
		VT=VT->NextSiblingElement("Vertex");
	};

	return true;
}
