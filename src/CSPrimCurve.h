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

//! Curve Primitive (Polygonal chain)
/*!
 This is a curve primitive defined by a number of 3D points
 */
class CSXCAD_EXPORT CSPrimCurve : public CSPrimitives
{
public:
	CSPrimCurve(ParameterSet* paraSet, CSProperties* prop);
	CSPrimCurve(CSPrimCurve* primCurve, CSProperties *prop=NULL);
	CSPrimCurve(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimCurve();

	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimCurve(this,prop);}

	virtual size_t AddPoint(double coords[]);
	virtual void SetCoord(size_t point_index, int nu, double val);
	virtual void SetCoord(size_t point_index, int nu, std::string val);

	virtual size_t GetNumberOfPoints() {return points.size();}
	virtual bool GetPoint(size_t point_index, double point[3]);
	//! Get the point coordinates for the given index in the specified coordinate system
	virtual bool GetPoint(size_t point_index, double* point, CoordinateSystem c_system, bool transform=true);

	virtual void ClearPoints();

	virtual bool GetBoundBox(double dBoundBox[6], bool PreserveOrientation=false);
	virtual bool IsInside(const double* Coord, double tol=0);

	virtual bool Update(std::string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	std::vector<ParameterCoord*> points;
};
