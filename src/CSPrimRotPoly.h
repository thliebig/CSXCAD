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

#pragma once

#include "CSPrimitives.h"
#include "CSPrimPolygon.h"

//! Rotational extruded polygon Primitive
/*!
 This is a rotation extruded area polygon primitive defined by a number of points in space, an extrude axis and start-, stop-angle.
 Warning: This primitive currently can only be defined in Cartesian coordinates.
 */
class CSXCAD_EXPORT CSPrimRotPoly : public CSPrimPolygon
{
public:
	CSPrimRotPoly(ParameterSet* paraSet, CSProperties* prop);
	CSPrimRotPoly(CSPrimRotPoly* primPolygon, CSProperties *prop=NULL);
	CSPrimRotPoly(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimRotPoly();

	virtual CSPrimRotPoly* GetCopy(CSProperties *prop=NULL) {return new CSPrimRotPoly(this,prop);}

	void SetRotAxisDir(int dir) {if ((dir>=0) && (dir<3)) m_RotAxisDir=dir;}

	int GetRotAxisDir() const {return m_RotAxisDir;}

	void SetAngle(int index, double val) {if ((index>=0) && (index<2)) StartStopAngle[index].SetValue(val);}
	void SetAngle(int index, const std::string val) {if ((index>=0) && (index<2)) StartStopAngle[index].SetValue(val);}

	double GetAngle(int index) const {if ((index>=0) && (index<2)) return StartStopAngle[index].GetValue(); else return 0;}
	ParameterScalar* GetAnglePS(int index) {if ((index>=0) && (index<2)) return &StartStopAngle[index]; else return NULL;}

	virtual bool IsInside(const double* Coord, double tol=0);

	virtual bool Update(std::string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	//start-stop angle
	ParameterScalar StartStopAngle[2];
	//sorted and pre evaluated angles
	double m_StartStopAng[2];
	//rot axis direction
	int m_RotAxisDir;
};
