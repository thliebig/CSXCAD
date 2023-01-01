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

//! Linear extruded polygon Primitive
/*!
 This is a linear extruded area polygon primitive defined by a number of points in space and an extrude vector.
 Warning: This primitive currently can only be defined in Cartesian coordinates.
 */
class CSXCAD_EXPORT CSPrimLinPoly : public CSPrimPolygon
{
public:
	CSPrimLinPoly(ParameterSet* paraSet, CSProperties* prop);
	CSPrimLinPoly(CSPrimLinPoly* primPolygon, CSProperties *prop=NULL);
	CSPrimLinPoly(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimLinPoly();

	virtual CSPrimLinPoly* GetCopy(CSProperties *prop=NULL) {return new CSPrimLinPoly(this,prop);}

	void SetLength(double val);
	void SetLength(const std::string val);

	double GetLength() {return extrudeLength.GetValue();}
	ParameterScalar* GetLengthPS() {return &extrudeLength;}

	virtual bool GetBoundBox(double dBoundBox[6], bool PreserveOrientation=false);
	virtual bool IsInside(const double* Coord, double tol=0);

	virtual bool Update(std::string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	ParameterScalar extrudeLength;
};
