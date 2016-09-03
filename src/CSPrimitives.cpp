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

#include <sstream>
#include <iostream>
#include <limits>
#include "tinyxml.h"
#include "stdint.h"

#include "CSPrimitives.h"

#include "CSProperties.h"
#include "CSFunctionParser.h"
#include "CSUseful.h"

#include <math.h>

#define PI acos(-1)

int g_PrimUniqueIDCounter=0;

void Point_Line_Distance(const double P[], const double start[], const double stop[], double &foot, double &dist, CoordinateSystem c_system)
{
	double l_P[3],l_start[3],l_stop[3];
	TransformCoordSystem(P,l_P,c_system,CARTESIAN);
	TransformCoordSystem(start,l_start,c_system,CARTESIAN);
	TransformCoordSystem(stop,l_stop,c_system,CARTESIAN);

	double dir[] = {l_stop[0]-l_start[0],l_stop[1]-l_start[1],l_stop[2]-l_start[2]};

	double LL = pow(dir[0],2)+pow(dir[1],2)+pow(dir[2],2); //Line length^2
	foot = (l_P[0]-l_start[0])*dir[0] +  (l_P[1]-l_start[1])*dir[1] + (l_P[2]-l_start[2])*dir[2];
	foot /= LL;

	double footP[] = {l_start[0] + foot*dir[0], l_start[1] + foot*dir[1], l_start[2] + foot*dir[2]};

	dist = sqrt(pow(l_P[0]-footP[0],2)+pow(l_P[1]-footP[1],2)+pow(l_P[2]-footP[2],2));
}

bool CSXCAD_EXPORT CoordInRange(const double* coord, const double* start, const double* stop, CoordinateSystem cs_in)
{
	double p[] = {coord[0],coord[1],coord[2]};
	switch (cs_in)
	{
	case CYLINDRICAL:
		if (p[1]<std::min(start[1],stop[1]))
			while (p[1]<std::min(start[1],stop[1]))
				p[1]+=2*PI;
		else if (p[1]>std::max(start[1],stop[1]))
			while (p[1]>std::max(start[1],stop[1]))
				p[1]-=2*PI;
	case CARTESIAN:
	default:
		for (int n=0;n<3;++n)
			if ((p[n]<std::min(start[n],stop[n])) || (p[n]>std::max(start[n],stop[n])))
				return false;
		return true;
		break;
	}
	return true;
}

/*********************CSPrimitives********************************************************************/
CSPrimitives::CSPrimitives(unsigned int ID, ParameterSet* paraSet, CSProperties* prop)
{
	this->Init();
	SetProperty(prop);
	uiID=ID;
	clParaSet=paraSet;
}

CSPrimitives::CSPrimitives(CSPrimitives* prim, CSProperties *prop)
{
	this->Init();
	if (prop==NULL)
		SetProperty(prim->clProperty);
	else
		SetProperty(prop);
	clParaSet=prim->clParaSet;
	m_Transform=CSTransform::New(prim->m_Transform);
	iPriority=prim->iPriority;
	m_MeshType = prim->m_MeshType;
	m_PrimCoordSystem = prim->m_PrimCoordSystem;
	m_Dimension = prim->m_Dimension;
}

CSPrimitives::CSPrimitives(ParameterSet* paraSet, CSProperties* prop)
{
	this->Init();
	SetProperty(prop);
	clParaSet=paraSet;
}

void CSPrimitives::Init()
{
	clProperty=NULL;
	clParaSet=NULL;
	m_Transform=NULL;
	uiID=g_PrimUniqueIDCounter++;
	iPriority=0;
	PrimTypeName = std::string("Base Type");
	m_Primtive_Used = false;
	m_MeshType = CARTESIAN;
	m_PrimCoordSystem = UNDEFINED_CS;
	m_BoundBox_CoordSys = UNDEFINED_CS;
	m_Dimension = 0;
	for (int n=0;n<6;++n)
		m_BoundBox[n]=0;
	m_BoundBoxValid = false;
}

CSTransform* CSPrimitives::GetTransform()
{
	if (m_Transform==NULL)
		m_Transform = new CSTransform(clParaSet);
	return m_Transform;
}

void CSPrimitives::SetProperty(CSProperties *prop)
{
	if ((clProperty!=NULL) && (clProperty!=prop))
		clProperty->RemovePrimitive(this);
	clProperty=prop;
	if ((prop!=NULL) && (!prop->HasPrimitive(this)))
		prop->AddPrimitive(this);
}

CSPrimitives::~CSPrimitives()
{
	if (clProperty!=NULL)
		clProperty->RemovePrimitive(this);
	delete m_Transform;
	m_Transform=NULL;
}

int CSPrimitives::IsInsideBox(const double *boundbox)
{
	if (m_BoundBoxValid==false)
		return 0;  // unable to decide with an invalid bounding box
	if ((this->GetBoundBoxCoordSystem()!=UNDEFINED_CS) && (this->GetBoundBoxCoordSystem()!=this->GetCoordInputType()))
		return 0;  // unable to decide if coordinate system do not match
	if (m_Transform!=NULL)
		if (m_Transform->HasTransform())
			return 0;  // unable to decide if a transformation is used

	for (int i=0;i<3;++i)
	{
		int d_l = 2*i;
		int d_u = d_l+1;
		if ((boundbox[d_l]<m_BoundBox[d_l]) && (boundbox[d_l]<m_BoundBox[d_u]) && (boundbox[d_u]<m_BoundBox[d_l]) && (boundbox[d_u]<m_BoundBox[d_u]))
			return -1;
		if ((boundbox[d_l]>m_BoundBox[d_l]) && (boundbox[d_l]>m_BoundBox[d_u]) && (boundbox[d_u]>m_BoundBox[d_l]) && (boundbox[d_u]>m_BoundBox[d_u]))
			return -1;
	}
	return 1;
}

bool CSPrimitives::Write2XML(TiXmlElement &elem, bool /*parameterised*/)
{
	elem.SetAttribute("Priority",iPriority);

	if (m_PrimCoordSystem!=UNDEFINED_CS)
		elem.SetAttribute("CoordSystem",(int)m_PrimCoordSystem);

	if (m_Transform)
		m_Transform->Write2XML(&elem);

	return true;
}

bool CSPrimitives::ReadFromXML(TiXmlNode &root)
{
	int help;
	TiXmlElement* elem=root.ToElement();
	if (elem==NULL) return false;
	if (elem->QueryIntAttribute("Priority",&iPriority)!=TIXML_SUCCESS) return false;

	if (elem->QueryIntAttribute("CoordSystem",&help)==TIXML_SUCCESS)
		m_PrimCoordSystem = (CoordinateSystem)help;

	delete m_Transform;
	m_Transform = CSTransform::New(elem, clParaSet);

	return true;
}

void CSPrimitives::ShowPrimitiveStatus(std::ostream& stream)
{
	stream << "  Primitive #" << GetID() << " Type: \"" << GetTypeName() << "\" Priority: " << GetPriority() << std::endl;
	stream << "  Primary Coord-System: " << m_PrimCoordSystem << " Mesh Coord-System: " << m_MeshType << " Bound-Box Coord-System: " << m_BoundBox_CoordSys << std::endl;
	stream << "  Bounding Box (Valid: " << m_BoundBoxValid << "): P1: (" << m_BoundBox[0] << "," << m_BoundBox[2] << "," << m_BoundBox[4] << ") P2: (" << m_BoundBox[1] << "," << m_BoundBox[3] << "," << m_BoundBox[5] << ")" << std::endl;
	if (m_Transform)
	{
		stream << "  Transform: " << std::endl;
		m_Transform->PrintTransformations(stream, "\t* ");
	}
	else
		stream << "  Transform: None" << std::endl;
}

void CSPrimitives::TransformCoords(double* Coord, bool invers, CoordinateSystem cs_in) const
{
	if (m_Transform==NULL)
		return;
	// transform to Cartesian for transformation
	TransformCoordSystem(Coord,Coord,cs_in,CARTESIAN);
	if (invers)
		m_Transform->InvertTransform(Coord,Coord);
	else
		m_Transform->Transform(Coord,Coord);
	// transform back from Cartesian to incoming coordinate system
	TransformCoordSystem(Coord,Coord,CARTESIAN,cs_in);
}
