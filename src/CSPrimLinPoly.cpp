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

#include "CSPrimLinPoly.h"
#include "CSProperties.h"
#include "CSUseful.h"

CSPrimLinPoly::CSPrimLinPoly(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimPolygon(ID,paraSet,prop)
{
	Type=LINPOLY;
	extrudeLength.SetParameterSet(paraSet);
	PrimTypeName = std::string("LinPoly");
}

CSPrimLinPoly::CSPrimLinPoly(CSPrimLinPoly* primLinPoly, CSProperties *prop) : CSPrimPolygon(primLinPoly,prop)
{
	Type=LINPOLY;
	extrudeLength.Copy(&primLinPoly->extrudeLength);
	PrimTypeName = std::string("LinPoly");
}

CSPrimLinPoly::CSPrimLinPoly(ParameterSet* paraSet, CSProperties* prop) : CSPrimPolygon(paraSet,prop)
{
	Type=LINPOLY;
	extrudeLength.SetParameterSet(paraSet);
	PrimTypeName = std::string("LinPoly");
}


CSPrimLinPoly::~CSPrimLinPoly()
{
}

bool CSPrimLinPoly::GetBoundBox(double dBoundBox[6], bool PreserveOrientation)
{
	UNUSED(PreserveOrientation); //has no orientation or preserved anyways
	bool accurate;
	accurate = CSPrimPolygon::GetBoundBox(dBoundBox);

	double len = extrudeLength.GetValue();

	if (len>0)
	{
		dBoundBox[2*m_NormDir] = Elevation.GetValue();
		dBoundBox[2*m_NormDir+1] = dBoundBox[2*m_NormDir] + len;
	}
	else
	{
		dBoundBox[2*m_NormDir+1] = Elevation.GetValue();
		dBoundBox[2*m_NormDir] = dBoundBox[2*m_NormDir+1] + len;
	}
	m_Dimension=0;
	for (int n=0;n<3;++n)
	{
		if (dBoundBox[2*n]!=dBoundBox[2*n+1])
			++m_Dimension;
	}
	return accurate;
}

bool CSPrimLinPoly::IsInside(const double* Coord, double tol)
{
	if (Coord==NULL) return false;
	double coords[3]={Coord[0],Coord[1],Coord[2]};
	if (m_Transform && Type==LINPOLY)
		TransformCoords(coords,true, m_MeshType);
	return CSPrimPolygon::IsInside(coords, tol);
}


bool CSPrimLinPoly::Update(std::string *ErrStr)
{
	int EC=0;

	bool bOK = CSPrimPolygon::Update(ErrStr);

	EC=extrudeLength.Evaluate();
	if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		std::stringstream stream;
		stream << std::endl << "Error in Polygon Elevation (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

	//update local bounding box
	m_BoundBoxValid = GetBoundBox(m_BoundBox);

	return bOK;
}

bool CSPrimLinPoly::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimPolygon::Write2XML(elem,parameterised);

	WriteTerm(extrudeLength,elem,"Length",parameterised);
	return true;
}

bool CSPrimLinPoly::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimPolygon::ReadFromXML(root)==false) return false;

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;
	if (ReadTerm(extrudeLength,*elem,"Length")==false) return false;

	return true;
}
