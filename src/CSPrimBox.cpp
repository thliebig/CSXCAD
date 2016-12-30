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

#include "CSPrimBox.h"
#include "CSProperties.h"
#include "CSUseful.h"

CSPrimBox::CSPrimBox(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=BOX;
	m_Coords[0].SetParameterSet(paraSet);
	m_Coords[1].SetParameterSet(paraSet);
	PrimTypeName = std::string("Box");
}

CSPrimBox::CSPrimBox(CSPrimBox* primBox, CSProperties *prop) : CSPrimitives(primBox,prop)
{
	Type=BOX;
	m_Coords[0].Copy(&primBox->m_Coords[0]);
	m_Coords[1].Copy(&primBox->m_Coords[1]);
	PrimTypeName = std::string("Box");
}

CSPrimBox::CSPrimBox(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=BOX;
	m_Coords[0].SetParameterSet(paraSet);
	m_Coords[1].SetParameterSet(paraSet);
	PrimTypeName = std::string("Box");
}


CSPrimBox::~CSPrimBox()
{
}

bool CSPrimBox::GetBoundBox(double dBoundBox[6], bool PreserveOrientation)
{
	const double* start = m_Coords[0].GetCoords(m_MeshType);
	const double* stop = m_Coords[1].GetCoords(m_MeshType);

	m_BoundBox_CoordSys = m_MeshType;
	m_Dimension=0;
	for (int i=0;i<3;++i)
	{
		dBoundBox[2*i]  = start[i];
		dBoundBox[2*i+1]= stop[i];

		if (start[i]!=stop[i])
			++m_Dimension;
	}
	if (PreserveOrientation)
		return true;
	for (int i=0;i<3;++i)
		if (dBoundBox[2*i]>dBoundBox[2*i+1])
		{
			double help=dBoundBox[2*i];
			dBoundBox[2*i]=dBoundBox[2*i+1];
			dBoundBox[2*i+1]=help;
		}
	if ( (m_MeshType!=m_PrimCoordSystem) &&  (m_PrimCoordSystem!=UNDEFINED_CS))
		// if the box is defined in a coordinate system other than the expected one, this BB is invalid
		return false;
	return true;
}

bool CSPrimBox::IsInside(const double* Coord, double /*tol*/)
{
	if (Coord==NULL) return false;

	const double* start = m_Coords[0].GetCoords(m_PrimCoordSystem);
	const double* stop  = m_Coords[1].GetCoords(m_PrimCoordSystem);
	double pos[3] = {Coord[0],Coord[1],Coord[2]};

	TransformCoords(pos, true, m_MeshType);
	//transform incoming coordinates into the coorindate system of the primitive
	TransformCoordSystem(pos,pos,m_MeshType,m_PrimCoordSystem);

	if (m_PrimCoordSystem!=UNDEFINED_CS)
		return CoordInRange(pos, start, stop, m_PrimCoordSystem);
	else
		return CoordInRange(pos, start, stop, m_MeshType);
}


bool CSPrimBox::Update(std::string *ErrStr)
{
	bool bOK=m_Coords[0].Evaluate(ErrStr) && m_Coords[1].Evaluate(ErrStr);
	if (bOK==false)
	{
		std::stringstream stream;
		stream << std::endl << "Error in Box (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
	}
	m_Coords[0].SetCoordinateSystem(m_PrimCoordSystem, m_MeshType);
	m_Coords[1].SetCoordinateSystem(m_PrimCoordSystem, m_MeshType);
	//update local bounding box
	m_BoundBoxValid = GetBoundBox(m_BoundBox);
	return bOK;
}

bool CSPrimBox::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimitives::Write2XML(elem,parameterised);

	TiXmlElement P1("P1");
	m_Coords[0].Write2XML(&P1,parameterised);
	elem.InsertEndChild(P1);

	TiXmlElement P2("P2");
	m_Coords[1].Write2XML(&P2,parameterised);
	elem.InsertEndChild(P2);
	return true;
}

bool CSPrimBox::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;
	if (m_Coords[0].ReadFromXML(root.FirstChildElement("P1")) == false)	return false;
	if (m_Coords[1].ReadFromXML(root.FirstChildElement("P2")) == false)	return false;
	return true;
}

void CSPrimBox::ShowPrimitiveStatus(std::ostream& stream)
{
	CSPrimitives::ShowPrimitiveStatus(stream);
	stream << "  Start: " << m_Coords[0].GetValueString(0) << "," << m_Coords[0].GetValueString(1) << "," << m_Coords[0].GetValueString(2) << std::endl;
	stream << "  Stop : " << m_Coords[1].GetValueString(0) << "," << m_Coords[1].GetValueString(1) << "," << m_Coords[1].GetValueString(2) << std::endl;
}
