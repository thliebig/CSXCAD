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

#include "CSPrimCylinder.h"
#include "CSProperties.h"
#include "CSUseful.h"

void NormalizeVector(double *vec)
{
	double mag = 0;
	for (unsigned int i=0; i<3; ++i)
	{
		mag += pow(vec[i], 2);
	}
	mag = sqrt(mag);

	for (unsigned int i=0; i<3; ++i)
	{
		vec[i] = vec[i] / mag;
	}
}

CSPrimCylinder::CSPrimCylinder(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=CYLINDER;
	m_AxisCoords[0].SetParameterSet(paraSet);
	m_AxisCoords[1].SetParameterSet(paraSet);
	psRadius.SetParameterSet(paraSet);
	PrimTypeName = std::string("Cylinder");
}

CSPrimCylinder::CSPrimCylinder(CSPrimCylinder* cylinder, CSProperties *prop) : CSPrimitives(cylinder,prop)
{
	Type=CYLINDER;
	m_AxisCoords[0].Copy(&cylinder->m_AxisCoords[0]);
	m_AxisCoords[1].Copy(&cylinder->m_AxisCoords[1]);
	psRadius.Copy(&cylinder->psRadius);
	PrimTypeName = std::string("Cylinder");
}

CSPrimCylinder::CSPrimCylinder(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=CYLINDER;
	m_AxisCoords[0].SetParameterSet(paraSet);
	m_AxisCoords[1].SetParameterSet(paraSet);
	psRadius.SetParameterSet(paraSet);
	PrimTypeName = std::string("Cylinder");
}


CSPrimCylinder::~CSPrimCylinder()
{
}

bool CSPrimCylinder::GetBoundBox(double dBoundBox[6], bool PreserveOrientation)
{
	UNUSED(PreserveOrientation); //has no orientation or preserved anyways
	bool accurate=true;
	int Direction=0;
	const double* start=m_AxisCoords[0].GetCartesianCoords();
	const double* stop =m_AxisCoords[1].GetCartesianCoords();
	m_BoundBox_CoordSys=CARTESIAN;

	double rad=GetBBRadius();
	// unit vector in direction of cylinder axis
	double v[3];
	int vnonzero = -1;  // TODO handle zero-height cylinders
	unsigned int zero_count = 0;
	for (unsigned int i=0; i<3; ++i)
	{
		v[i] = stop[i] - start[i];
		if (v[i] != 0)
		{
			vnonzero = i;
		}
		else
		{
			++zero_count;
			Direction+=pow(2,i);
		}
	}
	if (zero_count == 2)
	{
		for (unsigned int i=0; i<3; ++i)
		{
			if (i == vnonzero)
			{
				dBoundBox[2*i]   = fmin(start[i],stop[i]);
				dBoundBox[2*i+1] = fmax(start[i],stop[i]);
			}
			else
			{
				dBoundBox[2*i]   = -rad + start[i];
				dBoundBox[2*i+1] =  rad + stop[i];
			}
		}
	}
	else
	{
		NormalizeVector(v);

		// some unit vector perpendicular to cylinder axis
		double a[3];
		for (unsigned int i=0; i<3; ++i)
		{
			if (i == vnonzero)
			{
				unsigned int i1 = (i + 1) % 3;
				unsigned int i2 = (i + 2) % 3;
				a[i] = (v[i1] + v[i2]) / v[i];
			}
			else
			{
				a[i] = -1;
			}
		}
		NormalizeVector(a);

		// unit vector perpendicular to cylinder axis and a
		double b[3] = {a[1]*v[2] - a[2]*v[1], -a[0]*v[2] + a[2]*v[0], a[0]*v[1] - a[1]*v[0]};

		double theta[3];
		for (unsigned int i=0; i<3; ++i)
		{
			theta[i] = atan(b[i]/a[i]);
		}

		double min[3] = {start[0], start[1], start[2]};
		double max[3] = {start[0], start[1], start[2]};

		for (unsigned int i=0; i<2; ++i)
		{
			const double* coord = m_AxisCoords[i].GetCartesianCoords();
			for (unsigned int j=0; j<3; ++j)
			{
				double val = coord[j] + rad*a[j]*cos(theta[j]) + rad*b[j]*sin(theta[j]);
				double val2 = coord[j] + rad*a[j]*cos(theta[j]+PI) + rad*b[j]*sin(theta[j]+PI);
				min[j] = fmin(min[j], val);
				min[j] = fmin(min[j], val2);
				max[j] = fmax(max[j], val);
				max[j] = fmax(max[j], val2);
			}
		}

		for (unsigned int i=0; i<3; ++i)
		{
			dBoundBox[2*i]  =fmin(min[i],max[i]);
			dBoundBox[2*i+1]=fmax(min[i],max[i]);
		}
	}

	if (rad>0)
		m_Dimension=3;
	else if (Direction==7)
		m_Dimension=0;
	else
		m_Dimension=1;
	return accurate;
}

bool CSPrimCylinder::IsInside(const double* Coord, double /*tol*/)
{
	if (Coord==NULL) return false;

	const double* start=m_AxisCoords[0].GetCartesianCoords();
	const double* stop =m_AxisCoords[1].GetCartesianCoords();
	double pos[3];
	//transform incoming coordinates into cartesian coords
	TransformCoordSystem(Coord,pos,m_MeshType,CARTESIAN);
	if (m_Transform)
		m_Transform->InvertTransform(pos,pos);

	for (int n=0;n<3;++n)
		if (pos[n]<m_BoundBox[2*n] || pos[n]>m_BoundBox[2*n+1])
			return false;

	double foot,dist;
	Point_Line_Distance(pos,start,stop,foot,dist);

	if ((foot<0) || (foot>1)) //the foot point is not on the axis
		return false;
	if (dist>psRadius.GetValue())
		return false;

	return true;
}

bool CSPrimCylinder::Update(std::string *ErrStr)
{
	int EC=0;
	bool bOK=m_AxisCoords[0].Evaluate(ErrStr) && m_AxisCoords[1].Evaluate(ErrStr);
	if (bOK==false)
	{
		std::stringstream stream;
		stream << std::endl << "Error in " << PrimTypeName << " Coord (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
	}
	m_AxisCoords[0].SetCoordinateSystem(m_PrimCoordSystem, m_MeshType);
	m_AxisCoords[1].SetCoordinateSystem(m_PrimCoordSystem, m_MeshType);


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

bool CSPrimCylinder::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimitives::Write2XML(elem,parameterised);

	WriteTerm(psRadius,elem,"Radius",parameterised);

	TiXmlElement Start("P1");
	m_AxisCoords[0].Write2XML(&Start,parameterised);
	elem.InsertEndChild(Start);

	TiXmlElement Stop("P2");
	m_AxisCoords[1].Write2XML(&Stop,parameterised);
	elem.InsertEndChild(Stop);

	return true;
}

bool CSPrimCylinder::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;
	if (ReadTerm(psRadius,*elem,"Radius")==false) return false;

	if (m_AxisCoords[0].ReadFromXML(root.FirstChildElement("P1")) == false)	return false;
	if (m_AxisCoords[1].ReadFromXML(root.FirstChildElement("P2")) == false)	return false;

	return true;
}

void CSPrimCylinder::ShowPrimitiveStatus(std::ostream& stream)
{
	CSPrimitives::ShowPrimitiveStatus(stream);
	stream << "  Axis-Start: " << m_AxisCoords[0].GetValueString(0) << "," << m_AxisCoords[0].GetValueString(1) << "," << m_AxisCoords[0].GetValueString(2) << std::endl;
	stream << "  Axis-Stop : " << m_AxisCoords[1].GetValueString(0) << "," << m_AxisCoords[1].GetValueString(1) << "," << m_AxisCoords[1].GetValueString(2) << std::endl;
	stream << "  Radius: " << psRadius.GetValueString() << std::endl;
}
