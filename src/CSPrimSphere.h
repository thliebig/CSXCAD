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

//! Sphere Primitive
/*!
 This is a spherical primitive defined by its center coordinates and a radius.
 */
class CSXCAD_EXPORT CSPrimSphere : public CSPrimitives
{
public:
	CSPrimSphere(ParameterSet* paraSet, CSProperties* prop);
	CSPrimSphere(CSPrimSphere* sphere, CSProperties *prop=NULL);
	CSPrimSphere(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimSphere();

	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimSphere(this,prop);}

	//! Set the center point coordinate
	void SetCoord(int index, double val) {m_Center.SetValue(index,val);}
	//! Set the center point coordinate as paramater string
	void SetCoord(int index, const char* val) {m_Center.SetValue(index,val);}
	//! Set the center point coordinate as paramater string
	void SetCoord(int index, std::string val) {m_Center.SetValue(index,val);}

	void SetCenter(double x1, double x2, double x3);
	void SetCenter(double x[3]);

	void SetCenter(std::string x1, std::string x2, std::string x3);
	void SetCenter(std::string x[3]);

	double GetCoord(int index) {return m_Center.GetValue(index);}
	ParameterScalar* GetCoordPS(int index) {return m_Center.GetCoordPS(index);}
	ParameterCoord* GetCenter() {return &m_Center;}

	void SetRadius(double val) {psRadius.SetValue(val);}
	void SetRadius(const char* val) {psRadius.SetValue(val);}

	double GetRadius() {return psRadius.GetValue();}
	ParameterScalar* GetRadiusPS() {return &psRadius;}

	virtual bool GetBoundBox(double dBoundBox[6], bool PreserveOrientation=false);
	virtual bool IsInside(const double* Coord, double tol=0);

	virtual bool Update(std::string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void ShowPrimitiveStatus(std::ostream& stream);

protected:
	ParameterCoord m_Center;
	ParameterScalar psRadius;
};

