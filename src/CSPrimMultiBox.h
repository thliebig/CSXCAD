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

//! Multi-Box Primitive (Multi-Cube)
/*!
 This is a primitive defined by multiple cubes. Mostly used for already discretized objects.
 */
class CSXCAD_EXPORT CSPrimMultiBox : public CSPrimitives
{
public:
	CSPrimMultiBox(ParameterSet* paraSet, CSProperties* prop);
	CSPrimMultiBox(CSPrimMultiBox* multiBox,CSProperties *prop=NULL);
	CSPrimMultiBox(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimMultiBox();

	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL);

	void SetCoord(int index, double val);
	void SetCoord(int index, const char* val);

	void AddCoord(double val);
	void AddCoord(const char* val);

	void AddBox(int initBox=-1);
	void DeleteBox(size_t box);

	double GetCoord(int index);
	ParameterScalar* GetCoordPS(int index);

	double* GetAllCoords(size_t &Qty, double* array);

	void ClearOverlap();

	virtual bool GetBoundBox(double dBoundBox[6], bool PreserveOrientation=false);
	virtual bool IsInside(const double* Coord, double tol=0);

	unsigned int GetQtyBoxes();

	virtual bool Update(std::string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	std::vector<ParameterScalar*> vCoords;
};

