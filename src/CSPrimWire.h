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
#include "CSPrimCurve.h"

//! Wire Primitive (Polygonal chain with finite radius)
/*!
 This is a wire primitive derived from a curve with an additional wire radius
 \sa CSPrimCurve
 */
class CSXCAD_EXPORT CSPrimWire : public CSPrimCurve
{
public:
	CSPrimWire(ParameterSet* paraSet, CSProperties* prop);
	CSPrimWire(CSPrimWire* primCurve, CSProperties *prop=NULL);
	CSPrimWire(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimWire();

	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimWire(this,prop);}

	void SetWireRadius(double val);
	void SetWireRadius(const char* val);

	double GetWireRadius() {return wireRadius.GetValue();}
	ParameterScalar* GetWireRadiusPS() {return &wireRadius;}

	virtual bool GetBoundBox(double dBoundBox[6], bool PreserveOrientation=false);
	virtual bool IsInside(const double* Coord, double tol=0);

	virtual bool Update(std::string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	ParameterScalar wireRadius;
};
