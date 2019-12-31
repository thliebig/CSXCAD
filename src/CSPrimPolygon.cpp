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

#include "CSPrimPolygon.h"
#include "CSProperties.h"
#include "CSUseful.h"

CSPrimPolygon::CSPrimPolygon(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=POLYGON;
	m_NormDir = 0;
	Elevation.SetParameterSet(paraSet);
	PrimTypeName = std::string("Polygon");
}

CSPrimPolygon::CSPrimPolygon(CSPrimPolygon* primPolygon, CSProperties *prop) : CSPrimitives(primPolygon,prop)
{
	Type=POLYGON;
	m_NormDir = primPolygon->m_NormDir;
	Elevation.Copy(&primPolygon->Elevation);
	PrimTypeName = std::string("Polygon");
}

CSPrimPolygon::CSPrimPolygon(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=POLYGON;
	m_NormDir = 0;
	Elevation.SetParameterSet(paraSet);
	PrimTypeName = std::string("Polygon");
}

CSPrimPolygon::~CSPrimPolygon()
{
}

CSPrimPolygon* CSPrimPolygon::GetCopy(CSProperties *prop) {return new CSPrimPolygon(this,prop);}

void CSPrimPolygon::SetCoord(int index, double val)
{
	if ((index>=0) && (index<(int)vCoords.size())) vCoords.at(index).SetValue(val);
}

void CSPrimPolygon::SetCoord(int index, const std::string val)
{
	if ((index>=0) && (index<(int)vCoords.size())) vCoords.at(index).SetValue(val);
}

void CSPrimPolygon::AddCoord(double val)
{
	vCoords.push_back(ParameterScalar(clParaSet,val));
}

void CSPrimPolygon::AddCoord(const std::string val)
{
	vCoords.push_back(ParameterScalar(clParaSet,val));
}

void CSPrimPolygon::RemoveCoords(int /*index*/)
{
	//not yet implemented
}

double CSPrimPolygon::GetCoord(int index)
{
	if ((index>=0) && (index<(int)vCoords.size())) return vCoords.at(index).GetValue();
	return 0;
}

ParameterScalar* CSPrimPolygon::GetCoordPS(int index)
{
	if ((index>=0) && (index<(int)vCoords.size())) return &vCoords.at(index);
	return NULL;
}

size_t CSPrimPolygon::GetQtyCoords() {return vCoords.size()/2;}

double* CSPrimPolygon::GetAllCoords(size_t &Qty, double* array)
{
	Qty=vCoords.size();
	delete[] array;
	array = new double[Qty];
	for (size_t i=0;i<Qty;++i) array[i]=vCoords.at(i).GetValue();
	return array;
}

void CSPrimPolygon::SetNormDir(int dir) {if ((dir>=0) && (dir<3)) m_NormDir=dir;}

bool CSPrimPolygon::GetBoundBox(double dBoundBox[6], bool PreserveOrientation)
{
	UNUSED(PreserveOrientation); //has no orientation or preserved anyways
	bool accurate=false;
	m_BoundBox_CoordSys = CARTESIAN;
	if (vCoords.size()<2)
	{
		for (int i=0;i<6;++i) dBoundBox[i]=0;
		return dBoundBox;
	}
	double xmin = vCoords.at(0).GetValue(), xmax = vCoords.at(0).GetValue();
	double ymin = vCoords.at(1).GetValue(), ymax = vCoords.at(1).GetValue();
	for (size_t i=1;i<vCoords.size()/2;++i)
	{
		double x = vCoords.at(2*i).GetValue();
		double y = vCoords.at(2*i+1).GetValue();
		if (x<xmin) xmin=x;
		else if (x>xmax) xmax=x;
		if (y<ymin) ymin=y;
		else if (y>ymax) ymax=y;
	}
	int nP = (m_NormDir+1)%3;
	int nPP = (m_NormDir+2)%3;
	dBoundBox[2*m_NormDir] = dBoundBox[2*m_NormDir+1] = Elevation.GetValue();
	dBoundBox[2*nP] = xmin;
	dBoundBox[2*nP+1] = xmax;
	dBoundBox[2*nPP] = ymin;
	dBoundBox[2*nPP+1] = ymax;
	m_Dimension=0;
	for (int n=0;n<3;++n)
	{
		if (dBoundBox[2*n]!=dBoundBox[2*n+1])
			++m_Dimension;
	}
	return accurate;
}

bool CSPrimPolygon::IsInside(const double* inCoord, double /*tol*/)
{
	if (inCoord==NULL) return false;
	if (vCoords.size()<2) return false;

	double Coord[3];
	//transform incoming coordinates into cartesian coords
	TransformCoordSystem(inCoord,Coord,m_MeshType,CARTESIAN);
	if (m_Transform && Type==POLYGON)
		TransformCoords(Coord,true, CARTESIAN);

	for (unsigned int n=0;n<3;++n)
		if ((m_BoundBox[2*n]>Coord[n]) || (m_BoundBox[2*n+1]<Coord[n])) return false;

	double x=0,y=0;
	int nP = (m_NormDir+1)%3;
	int nPP = (m_NormDir+2)%3;
	x = Coord[nP];
	y = Coord[nPP];

	int wn = 0;

	size_t np = vCoords.size()/2;
	double x1 = vCoords[2*np-2].GetValue();
	double y1 = vCoords[2*np-1].GetValue();
	double x2 = vCoords[0].GetValue();
	double y2 = vCoords[1].GetValue();
	bool startover = y1 >= y ? true : false;
	bool endover;

	for (size_t i=0;i<np;++i)
	{
		x2 = vCoords[2*i].GetValue();
		y2 = vCoords[2*i+1].GetValue();

		//check if coord is on a cartesian edge exactly
		if ((x2==x1) && (x1==x) && ( ((y<y1) && (y>y2)) || ((y>y1) && (y<y2)) ))
			return true;
		if ((y2==y1) && (y1==y) && ( ((x<x1) && (x>x2)) || ((x>x1) && (x<x2)) ))
			return true;

		endover = y2 >= y ? true : false;
		if (startover != endover)
		{
			if ((y2 - y)*(x2 - x1) <= (y2 - y1)*(x2 - x))
			{
				if (endover) wn ++;
			}
			else
			{
				if (!endover) wn --;
			}
		}
		startover = endover;
		y1 = y2;
		x1 = x2;
	}
	// return true if polygon is inside the polygon
	if (wn != 0)
		return true;

	return false;
}


bool CSPrimPolygon::Update(std::string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	if (! ((m_PrimCoordSystem==CARTESIAN) || (m_PrimCoordSystem==UNDEFINED_CS && m_MeshType==CARTESIAN)))
	{
		std::cerr << "CSPrimPolygon::Update: Warning: CSPrimPolygon can not be defined in non Cartesian coordinate systems! Result may be unexpected..." << std::endl;
		ErrStr->append("Warning: CSPrimPolygon can not be defined in non Cartesian coordinate systems! Result may be unexpected...\n");
	}
	for (size_t i=1;i<vCoords.size();++i)
	{
		EC=vCoords[i].Evaluate();
		if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
		{
			bOK=false;
			std::stringstream stream;
			stream << std::endl << "Error in Polygon (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}

	EC=Elevation.Evaluate();
	if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		std::stringstream stream;
		stream << std::endl << "Error in Polygon Elevation (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

	//update local bounding box used to speedup IsInside()
	m_BoundBoxValid = GetBoundBox(m_BoundBox);

	return bOK;
}

bool CSPrimPolygon::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimitives::Write2XML(elem,parameterised);

	WriteTerm(Elevation,elem,"Elevation",parameterised);

	elem.SetAttribute("NormDir",m_NormDir);

	elem.SetAttribute("QtyVertices",(int)vCoords.size()/2);

	for (size_t i=0;i<vCoords.size()/2;++i)
	{
		TiXmlElement VT("Vertex");
		WriteTerm(vCoords.at(i*2),VT,"X1",parameterised);
		WriteTerm(vCoords.at(i*2+1),VT,"X2",parameterised);
		elem.InsertEndChild(VT);
	}
	return true;
}

bool CSPrimPolygon::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;
	if (ReadTerm(Elevation,*elem,"Elevation")==false)
		Elevation.SetValue(0);

	int help;
	if (elem->QueryIntAttribute("NormDir",&help)!=TIXML_SUCCESS)
		 return false;
	m_NormDir=help;

	TiXmlElement *VT=root.FirstChildElement("Vertex");
	if (vCoords.size()!=0) return false;
	int i=0;
	while (VT)
	{
		for (int n=0;n<2;++n) this->AddCoord(0.0);

		if (ReadTerm(vCoords.at(i*2),*VT,"X1")==false) return false;
		if (ReadTerm(vCoords.at(i*2+1),*VT,"X2")==false) return false;

		VT=VT->NextSiblingElement("Vertex");
		++i;
	};

	return true;
}
