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

#include "CSPrimSphere.h"
#include "CSProperties.h"
#include "CSUseful.h"

CSPrimSphere::CSPrimSphere(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=SPHERE;
	m_Center.SetParameterSet(paraSet);
	psRadius.SetParameterSet(paraSet);
	PrimTypeName = std::string("Sphere");
}

CSPrimSphere::CSPrimSphere(CSPrimSphere* sphere, CSProperties *prop) : CSPrimitives(sphere,prop)
{
	Type=SPHERE;
	m_Center.Copy(&sphere->m_Center);
	psRadius.Copy(&sphere->psRadius);
	PrimTypeName = std::string("Sphere");
}

CSPrimSphere::CSPrimSphere(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=SPHERE;
	m_Center.SetParameterSet(paraSet);
	psRadius.SetParameterSet(paraSet);
	PrimTypeName = std::string("Sphere");
}


CSPrimSphere::~CSPrimSphere()
{
}

void CSPrimSphere::SetCenter(double x1, double x2, double x3)
{
	SetCoord(0,x1);
	SetCoord(1,x2);
	SetCoord(2,x3);
}

void CSPrimSphere::SetCenter(double x[3])
{
	for (int n=0;n<3;++n)
		SetCoord(n, x[n]);
}

void CSPrimSphere::SetCenter(std::string x1, std::string x2, std::string x3)
{
	SetCoord(0,x1);
	SetCoord(1,x2);
	SetCoord(2,x3);
}

void CSPrimSphere::SetCenter(std::string x[3])
{
	for (int n=0;n<3;++n)
		SetCoord(n, x[n]);
}


bool CSPrimSphere::GetBoundBox(double dBoundBox[6], bool PreserveOrientation)
{
	UNUSED(PreserveOrientation); //has no orientation or preserved anyways
	const double* center = m_Center.GetCartesianCoords();
	m_BoundBox_CoordSys=CARTESIAN;
	double radius = psRadius.GetValue();
	for (unsigned int i=0;i<3;++i)
	{
		dBoundBox[2*i]=center[i]-radius;
		dBoundBox[2*i+1]=center[i]+radius;
	}
	if (radius>0)
		m_Dimension=3;
	else
		m_Dimension=0;
	return true;
}

bool CSPrimSphere::IsInside(const double* Coord, double /*tol*/)
{
	if (Coord==NULL) return false;
	double out[3];
	const double* center = m_Center.GetCartesianCoords();
	TransformCoordSystem(Coord,out,m_MeshType,CARTESIAN);
	if (m_Transform)
		m_Transform->InvertTransform(out,out);
	double dist=sqrt(pow(out[0]-center[0],2)+pow(out[1]-center[1],2)+pow(out[2]-center[2],2));
	if (dist<psRadius.GetValue())
		return true;
	return false;
}

bool CSPrimSphere::Update(std::string *ErrStr)
{
	int EC=0;
	bool bOK=m_Center.Evaluate(ErrStr);
	if (bOK==false)
	{
		std::stringstream stream;
		stream << std::endl << "Error in " << PrimTypeName << " Center Point (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
	}
	m_Center.SetCoordinateSystem(m_PrimCoordSystem, m_MeshType);

	EC=psRadius.Evaluate();
	if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		std::stringstream stream;
		stream << std::endl << "Error in " << PrimTypeName << " Radius (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

	//update local bounding box
	m_BoundBoxValid = GetBoundBox(m_BoundBox);

	return bOK;
}

bool CSPrimSphere::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimitives::Write2XML(elem,parameterised);

	WriteTerm(psRadius,elem,"Radius",parameterised);

	TiXmlElement Center("Center");
	m_Center.Write2XML(&Center,parameterised);
	elem.InsertEndChild(Center);
	return true;
}

bool CSPrimSphere::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;
	if (ReadTerm(psRadius,*elem,"Radius")==false) return false;

	TiXmlElement* Center=root.FirstChildElement("Center");
	if (m_Center.ReadFromXML(Center) == false)	return false;

	return true;
}

void CSPrimSphere::ShowPrimitiveStatus(std::ostream& stream)
{
	CSPrimitives::ShowPrimitiveStatus(stream);
	stream << "  Center: " << m_Center.GetValueString(0) << "," << m_Center.GetValueString(1) << "," << m_Center.GetValueString(2) << std::endl;
	stream << "  Radius: " << psRadius.GetValueString() << std::endl;
}
