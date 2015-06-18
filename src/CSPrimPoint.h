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

//! Point Primitive
/*!
 This is a point primitive (useful for field probes).
 */
class CSXCAD_EXPORT CSPrimPoint : public CSPrimitives
{
public:
	CSPrimPoint(ParameterSet* paraSet, CSProperties* prop);
	CSPrimPoint(CSPrimPoint* primPoint, CSProperties *prop=NULL);
	CSPrimPoint(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimPoint();

	virtual CSPrimPoint* GetCopy(CSProperties *prop=NULL) {return new CSPrimPoint(this,prop);}

	void SetCoord(int index, double val);
	void SetCoord(int index, const std::string val);
	void SetCoords( double c1, double c2, double c3 );

	//! Get the point coordinates according to the input mesh type
	double GetCoord(int index);
	ParameterScalar* GetCoordPS(int index);

	const ParameterCoord* GetCoords() const {return &m_Coords;}

	virtual bool GetBoundBox(double dBoundBox[6], bool PreserveOrientation=false);
	virtual bool IsInside(const double* Coord, double tol=0);

	virtual bool Update(std::string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void ShowPrimitiveStatus(std::ostream& stream);

protected:
	//! Vector describing the point: x,y,z
	ParameterCoord m_Coords;
};

