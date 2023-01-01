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

#define PI 3.141592653589793238462643383279

void NormalizeVector(double *vec);

//! Cylinder Primitive
/*!
 This is a cylindrical primitive defined by its axis start-, end-coordinates and a radius.
 */
class CSXCAD_EXPORT CSPrimCylinder : public CSPrimitives
{
public:
	CSPrimCylinder(ParameterSet* paraSet, CSProperties* prop);
	CSPrimCylinder(CSPrimCylinder* cylinder, CSProperties *prop=NULL);
	CSPrimCylinder(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimCylinder();

	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimCylinder(this,prop);}

	void SetCoord(int index, double val);
	void SetCoord(int index, const char* val);
	void SetCoord(int index, std::string val);

	double GetCoord(int index) {if ((index>=0) && (index<6)) return m_AxisCoords[index%2].GetValue(index/2); else return 0;}
	ParameterScalar* GetCoordPS(int index) {if ((index>=0) && (index<6)) return m_AxisCoords[index%2].GetCoordPS(index/2); else return NULL;}

	ParameterCoord* GetAxisStartCoord() {return &m_AxisCoords[0];}
	ParameterCoord* GetAxisStopCoord() {return &m_AxisCoords[1];}

	void SetRadius(double val);
	void SetRadius(const char* val);

	double GetRadius() {return psRadius.GetValue();}
	ParameterScalar* GetRadiusPS() {return &psRadius;}

	virtual bool GetBoundBox(double dBoundBox[6], bool PreserveOrientation=false);
	virtual bool IsInside(const double* Coord, double tol=0);

	virtual bool Update(std::string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void ShowPrimitiveStatus(std::ostream& stream);

protected:
	ParameterCoord m_AxisCoords[2];
	ParameterScalar psRadius;
	virtual double GetBBRadius() {return psRadius.GetValue();} // Get the radius for the bounding box calculation
};
