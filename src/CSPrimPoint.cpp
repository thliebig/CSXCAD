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

#include "CSPrimPoint.h"
#include "CSProperties.h"
#include "CSUseful.h"

CSPrimPoint::CSPrimPoint(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type = POINT;
	m_Coords.SetParameterSet(paraSet);
	PrimTypeName = "Point";
}

CSPrimPoint::CSPrimPoint(CSPrimPoint* primPoint, CSProperties *prop) : CSPrimitives(primPoint,prop)
{
	Type = POINT;
	m_Coords.Copy(&primPoint->m_Coords);
	PrimTypeName = "Point";
}

CSPrimPoint::CSPrimPoint(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type = POINT;
	m_Coords.SetParameterSet(paraSet);
	PrimTypeName = "Point";
}

CSPrimPoint::~CSPrimPoint()
{
}

void CSPrimPoint::SetCoord(int index, double val)
{
	m_Coords.SetValue(index,val);
}

void CSPrimPoint::SetCoord(int index, const std::string val)
{
	m_Coords.SetValue(index,val);
}

double CSPrimPoint::GetCoord(int index)
{
	return m_Coords.GetCoordValue(index,m_MeshType);
}

ParameterScalar* CSPrimPoint::GetCoordPS(int index)
{
	return m_Coords.GetCoordPS(index);
}

bool CSPrimPoint::GetBoundBox(double dBoundBox[6], bool PreserveOrientation)
{
	UNUSED(PreserveOrientation); //has no orientation or preserved anyways
	const double *coord = m_Coords.GetCoords(m_MeshType);
	for (int i=0; i<3; i++)
	{
		dBoundBox[2*i]   = coord[i];
		dBoundBox[2*i+1] = coord[i];
	}
	m_Dimension=0;
	m_BoundBox_CoordSys = m_PrimCoordSystem;
	return true;
}

bool CSPrimPoint::IsInside(const double* /*Coord*/, double /*tol*/)
{
	// this is a 0D-object, you can never be inside...
	return false;
}


bool CSPrimPoint::Update(std::string *ErrStr)
{
	bool bOK=m_Coords.Evaluate(ErrStr);
	if (bOK==false)
	{
		std::stringstream stream;
		stream << std::endl << "Error in Point (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
	}
	m_Coords.SetCoordinateSystem(m_PrimCoordSystem, m_MeshType);
	//update local bounding box
	m_BoundBoxValid = GetBoundBox(m_BoundBox);
	return bOK;
}

bool CSPrimPoint::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimitives::Write2XML(elem,parameterised);
	return m_Coords.Write2XML(&elem,parameterised);
}

bool CSPrimPoint::ReadFromXML(TiXmlNode &root)
{
	if (!CSPrimitives::ReadFromXML(root)) return false;
	return m_Coords.ReadFromXML(dynamic_cast<TiXmlElement*>(&root));
}


void CSPrimPoint::ShowPrimitiveStatus(std::ostream& stream)
{
	CSPrimitives::ShowPrimitiveStatus(stream);
	stream << "  Coordinate: " << m_Coords.GetValueString(0) << "," << m_Coords.GetValueString(1) << "," << m_Coords.GetValueString(2) << std::endl;
}
