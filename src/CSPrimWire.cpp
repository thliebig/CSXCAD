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

#include "CSPrimWire.h"
#include "CSProperties.h"
#include "CSUseful.h"

CSPrimWire::CSPrimWire(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimCurve(ID,paraSet,prop)
{
	Type=WIRE;
	PrimTypeName = std::string("Wire");
	wireRadius.SetParameterSet(paraSet);
}

CSPrimWire::CSPrimWire(CSPrimWire* primCurve, CSProperties *prop) : CSPrimCurve(primCurve,prop)
{
	Type=WIRE;
	PrimTypeName = std::string("Wire");
	wireRadius.Copy(&primCurve->wireRadius);
}

CSPrimWire::CSPrimWire(ParameterSet* paraSet, CSProperties* prop) : CSPrimCurve(paraSet,prop)
{
	Type=WIRE;
	PrimTypeName = std::string("Wire");
	wireRadius.SetParameterSet(paraSet);
}


CSPrimWire::~CSPrimWire()
{
}


bool CSPrimWire::GetBoundBox(double dBoundBox[6], bool PreserveOrientation)
{
	bool accurate;
	accurate = CSPrimCurve::GetBoundBox(dBoundBox,PreserveOrientation);
	double rad = wireRadius.GetValue();
	for (int n=0;n<3;++n)
	{
		dBoundBox[2*n]-=rad;
		dBoundBox[2*n+1]+=rad;
	}
	if (rad>0)
		m_Dimension+=2;
	return accurate;
}

bool CSPrimWire::IsInside(const double* Coord, double /*tol*/)
{
	if (Coord==NULL) return false;
	double rad = wireRadius.GetValue();
	const double* p0;
	const double* p1;
	double pos[3];
	//transform incoming coordinates into cartesian coords
	TransformCoordSystem(Coord,pos,m_MeshType,CARTESIAN);
	if (m_Transform)
		m_Transform->InvertTransform(pos,pos);

	for (unsigned int n=0;n<3;++n)
	{
		if ((m_BoundBox[2*n]>pos[n]) || (m_BoundBox[2*n+1]<pos[n])) return false;
	}

	double foot,dist,distPP;
	for (size_t i=0;i<GetNumberOfPoints();++i)
	{
		p0 = points.at(i)->GetCartesianCoords();

		dist = sqrt(pow(pos[0]-p0[0],2)+pow(pos[1]-p0[1],2)+pow(pos[2]-p0[2],2));
		if (dist<rad)
			return true;

		if (i<GetNumberOfPoints()-1)
		{
			p1 = points.at(i+1)->GetCartesianCoords();
			distPP = sqrt(pow(p1[0]-p0[0],2)+pow(p1[1]-p0[1],2)+pow(p1[2]-p0[2],2))+rad;
			if (dist<distPP)
			{
				Point_Line_Distance(pos ,p0 ,p1 ,foot ,dist);
				if ((foot>0) && (foot<1) && (dist<rad))
					return true;
			}
		}
	}

	return false;
}

bool CSPrimWire::Update(std::string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	CSPrimCurve::Update(ErrStr);

	EC=wireRadius.Evaluate();
	if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		std::stringstream stream;
		stream << std::endl << "Error in " << PrimTypeName << " (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}
	//update local bounding box used to speedup IsInside()
	m_BoundBoxValid = GetBoundBox(m_BoundBox);
	return bOK;
}

bool CSPrimWire::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimCurve::Write2XML(elem,parameterised);

	WriteTerm(wireRadius,elem,"WireRadius",parameterised);
	return true;
}

bool CSPrimWire::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimCurve::ReadFromXML(root)==false) return false;

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;
	if (ReadTerm(wireRadius,*elem,"WireRadius")==false) return false;
	return true;
}
