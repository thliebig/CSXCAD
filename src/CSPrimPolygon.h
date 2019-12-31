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

//! 2D Polygon Primitive
/*!
 This is an area polygon primitive defined by a number of points in space.
 Warning: This primitive currently can only be defined in Cartesian coordinates.
 */
class CSXCAD_EXPORT CSPrimPolygon : public CSPrimitives
{
public:
	CSPrimPolygon(ParameterSet* paraSet, CSProperties* prop);
	CSPrimPolygon(CSPrimPolygon* primPolygon, CSProperties *prop=NULL);
	CSPrimPolygon(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimPolygon();

	virtual CSPrimPolygon* GetCopy(CSProperties *prop=NULL);

	void SetCoord(int index, double val);
	void SetCoord(int index, const std::string val);

	void AddCoord(double val);
	void AddCoord(const std::string val);

	void RemoveCoords(int index);
	void ClearCoords() {vCoords.clear();}

	double GetCoord(int index);
	ParameterScalar* GetCoordPS(int index);

	size_t GetQtyCoords();
	double* GetAllCoords(size_t &Qty, double* array);

	void SetNormDir(int dir);

	int GetNormDir() {return m_NormDir;}

	void SetElevation(double val) {Elevation.SetValue(val);}
	void SetElevation(const char* val) {Elevation.SetValue(val);}

	double GetElevation() {return Elevation.GetValue();}
	ParameterScalar* GetElevationPS() {return &Elevation;}

	virtual bool GetBoundBox(double dBoundBox[6], bool PreserveOrientation=false);
	virtual bool IsInside(const double* Coord, double tol=0);

	virtual bool Update(std::string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	///Vector describing the polygon, x1,y1,x2,y2 ... xn,yn
	std::vector<ParameterScalar> vCoords;
	///The polygon plane normal direction
	int m_NormDir;
	///The polygon plane elevation in direction of the normal vector
	ParameterScalar Elevation;
};

