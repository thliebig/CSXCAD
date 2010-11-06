/*
*	Copyright (C) 2010 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#include "ParameterCoord.h"
#include "tinyxml.h"
#include <sstream>

double* TransformCoords(const double* in, double* out, CoordinateSystem CS_In, CoordinateSystem CS_out)
{
	switch (CS_In)
	{
	case CARTESIAN: //input coords are cartesian
		switch (CS_out)
		{
		default: //unknown transform --> just copy
		case CARTESIAN: // transform cartesian --> cartesian
			for (int n=0;n<3;++n)
				out[n] = in[n];  //just copy
			break;
		case CYLINDRICAL: // transform cartesian --> cylindrical
			out[0] = sqrt(in[0]*in[0]+in[1]*in[1]); // r = sqrt(x²+y²)
			out[1] = atan2(in[1],in[0]); //alpha = atan2(y,x)
			out[2] = in[2]; //z==z
			break;
		}
		break;
	case CYLINDRICAL: //input coords are cylindrical
		switch (CS_out)
		{
		case CARTESIAN: // transform cylindrical --> cartesian
			out[0] = in[0] * cos(in[1]); // x = r * cos(alpha)
			out[1] = in[0] * sin(in[1]); // y = r * sin(alpha)
			out[2] = in[2]; // z = z
			break;
		default: //unknown transform --> just copy
		case CYLINDRICAL: // transform cylindrical --> cylindrical
			for (int n=0;n<3;++n)
				out[n] = in[n];  //just copy
			break;
		}
		break;
	default: //unknown transform --> just copy
		for (int n=0;n<3;++n)
			out[n] = in[n];  //just copy
	}
	return out;
}

ParameterCoord::ParameterCoord()
{
	m_CoordSystem = UNDEFINED_CS;
	for (int n=0;n<3;++n)
		m_Coords[n] = new ParameterScalar();
	Update();
}

ParameterCoord::ParameterCoord(ParameterSet* ParaSet)
{
	m_CoordSystem = UNDEFINED_CS;
	for (int n=0;n<3;++n)
		m_Coords[n] = new ParameterScalar(ParaSet,0);
	Update();
}

ParameterCoord::ParameterCoord(CoordinateSystem cs)
{
	m_CoordSystem = cs;
	for (int n=0;n<3;++n)
		m_Coords[n] = new ParameterScalar();
	Update();
}

ParameterCoord::ParameterCoord(ParameterSet* ParaSet, const double value[3])
{
	m_CoordSystem = UNDEFINED_CS;
	for (int n=0;n<3;++n)
		m_Coords[n] = new ParameterScalar(ParaSet, value[n]);
	Update();
}

ParameterCoord::ParameterCoord(ParameterSet* ParaSet, const string value[3])
{
	m_CoordSystem = UNDEFINED_CS;
	for (int n=0;n<3;++n)
		m_Coords[n] = new ParameterScalar(ParaSet, value[n]);
	Update();
}

ParameterCoord::ParameterCoord(ParameterCoord* pc)
{
	m_CoordSystem = pc->m_CoordSystem;
	for (int n=0;n<3;++n)
		m_Coords[n] = new ParameterScalar(pc->m_Coords[n]);
	Update();
}

ParameterCoord::~ParameterCoord()
{
	for (int n=0;n<3;++n)
	{
		delete m_Coords[n];
		m_Coords[n]=NULL;
	}
}

void ParameterCoord::SetParameterSet(ParameterSet *paraSet)
{
	for (int n=0;n<3;++n)
		m_Coords[n]->SetParameterSet(paraSet);
	Update();
}

void ParameterCoord::SetCoordinateSystem(CoordinateSystem cs, CoordinateSystem fallBack_cs)
{
	if (cs!=UNDEFINED_CS)
		return SetCoordinateSystem(cs);
	return SetCoordinateSystem(fallBack_cs);
}

int ParameterCoord::SetValue(int ny, string value)
{
	if ((ny<0) || (ny>2))
		return -1;
	int EC = m_Coords[ny]->SetValue(value);
	Update();
	return EC;
}

void ParameterCoord::SetValue(int ny, double value)
{
	if ((ny<0) || (ny>2))
		return;
	m_Coords[ny]->SetValue(value);
	Update();
}

double ParameterCoord::GetValue(int ny)
{
	if ((ny<0) || (ny>2))
		return nan("");
	return m_Coords[ny]->GetValue();
}

double ParameterCoord::GetValue(int ny, CoordinateSystem cs)
{
	if ((ny<0) || (ny>2))
		return nan("");
	return GetCoords(cs)[ny];
}

ParameterScalar* ParameterCoord::GetCoordPS(int ny)
{
	if ((ny>=0) && (ny<3))
		return m_Coords[ny];
	return 0;
}

const double* ParameterCoord::GetNativeCoords()
{
	switch (m_CoordSystem)
	{
	default:
	case CARTESIAN:
		return GetCartesianCoords();
	case CYLINDRICAL:
		return GetCylindricalCoords();
	}
	return NULL; //this should not happen...
}

const double* ParameterCoord::GetCoords(CoordinateSystem cs)
{
	switch (cs)
	{
	case CARTESIAN:
		return GetCartesianCoords();
	case CYLINDRICAL:
		return GetCylindricalCoords();
	default:
		return GetNativeCoords();
	}
}

bool ParameterCoord::Evaluate(string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	for (int i=0;i<3;++i)
	{
		EC=m_Coords[i]->Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
		{
			stringstream stream;
			stream << endl << "Error in ParameterCoord (component: " << i << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}
	return bOK;
}

void ParameterCoord::Update()
{
	double coords[3] = {m_Coords[0]->GetValue(),m_Coords[1]->GetValue(),m_Coords[2]->GetValue()};
	TransformCoords(coords, m_CartesianCoords, m_CoordSystem, CARTESIAN);
	TransformCoords(coords, m_CylindricalCoords, m_CoordSystem, CYLINDRICAL);
}

bool ParameterCoord::Write2XML(TiXmlElement *elem, bool parameterised)
{
	if (elem==NULL)
		return false;
	WriteTerm(*m_Coords[0],*elem,"X",parameterised);
	WriteTerm(*m_Coords[1],*elem,"Y",parameterised);
	WriteTerm(*m_Coords[2],*elem,"Z",parameterised);
	return true;
}

bool ParameterCoord::ReadFromXML(TiXmlElement *elem)
{
	if (elem==NULL)
		return false;
	if (ReadTerm(*m_Coords[0],*elem,"X")==false) return false;
	if (ReadTerm(*m_Coords[1],*elem,"Y")==false) return false;
	if (ReadTerm(*m_Coords[2],*elem,"Z")==false) return false;
	return true;
}
