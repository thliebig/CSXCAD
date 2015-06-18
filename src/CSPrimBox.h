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

//! Box Primitive (Cube)
/*!
 This is a cube primitive defined by its start-, end-coordinates.
 */
class CSXCAD_EXPORT CSPrimBox : public CSPrimitives
{
public:
	CSPrimBox(ParameterSet* paraSet, CSProperties* prop);
	CSPrimBox(CSPrimBox* primBox, CSProperties *prop=NULL);
	CSPrimBox(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimBox();

	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimBox(this,prop);}

	void SetCoord(int index, double val) {if ((index>=0) && (index<6)) m_Coords[index%2].SetValue(index/2,val);}
	void SetCoord(int index, const char* val) {if ((index>=0) && (index<6)) m_Coords[index%2].SetValue(index/2,val);}
	void SetCoord(int index, std::string val) {if ((index>=0) && (index<6)) m_Coords[index%2].SetValue(index/2,val);}

	double GetCoord(int index) {if ((index>=0) && (index<6)) return m_Coords[index%2].GetValue(index/2); else return 0;}
	ParameterScalar* GetCoordPS(int index) {if ((index>=0) && (index<6)) return m_Coords[index%2].GetCoordPS(index/2); else return NULL;}

	ParameterCoord* GetStartCoord() {return &m_Coords[0];}
	ParameterCoord* GetStopCoord() {return &m_Coords[1];}

	virtual bool GetBoundBox(double dBoundBox[6], bool PreserveOrientation=false);
	virtual bool IsInside(const double* Coord, double tol=0);

	virtual bool Update(std::string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void ShowPrimitiveStatus(std::ostream& stream);

protected:
	//start and stop coords defining the box
	ParameterCoord m_Coords[2];
};

