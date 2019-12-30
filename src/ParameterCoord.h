/*
*	Copyright (C) 2010 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#ifndef PARAMETERCOORD_H
#define PARAMETERCOORD_H

#include "ParameterObjects.h"

//! This class can hold a parameterized coordinate, defined in different coordinate systems.
class CSXCAD_EXPORT ParameterCoord
{
public:
	ParameterCoord();
	ParameterCoord(ParameterSet* ParaSet);
	ParameterCoord(CoordinateSystem cs);
	ParameterCoord(ParameterSet* ParaSet, const double value[3]);
	ParameterCoord(ParameterSet* ParaSet, const std::string value[3]);
	ParameterCoord(ParameterCoord* pc);
	~ParameterCoord();

	void SetParameterSet(ParameterSet *paraSet);

	//! Set the coordinate system used for this coordinates
	void SetCoordinateSystem(CoordinateSystem cs) {m_CoordSystem=cs; Update();}
	//! Convienient method to set the coordinate system, including a fall back if primary coordinate system is undefined.
	void SetCoordinateSystem(CoordinateSystem cs, CoordinateSystem fallBack_cs);
	//! Get the coordinate system that has been set for this coordinate
	CoordinateSystem GetCoordinateSystem() const {return m_CoordSystem;}

	int SetValue(int ny, std::string value);
	void SetValue(int ny, double value);

	//! Get the native coordinate values
	double GetValue(int ny);
	//! Get the native coordinate values as string
	const std::string GetValueString(int ny) const;
	//! Get the internal scalar parameter, use carefully...
	ParameterScalar* GetCoordPS(int ny);

	//! Get the coordinate in the given coordinate system
	double GetCoordValue(int ny, CoordinateSystem cs);

	const double* GetNativeCoords() const;
	const double* GetCartesianCoords() const {return m_CartesianCoords;}
	const double* GetCylindricalCoords() const {return m_CylindricalCoords;}
	const double* GetCoords(CoordinateSystem cs) const;

	//! Evaluate the parametric coordinates and return an error message. This methode should be called before requesting coordinate values to check for valid parametric coordinates.
	bool Evaluate(std::string *ErrStr);

	// Copy all values and parameter from pc to this.
	void Copy(ParameterCoord* pc);

	//! Write this coords to a XML-node.
	bool Write2XML(TiXmlElement *elem, bool parameterised=true);
	//! Read coords from a XML-node.
	bool ReadFromXML(TiXmlElement *elem);

protected:
	//! Update/evaluate the internal data structure
	void Update();
	ParameterScalar* m_Coords[3];

	//! Coordinate system used for this coordinate
	CoordinateSystem m_CoordSystem;

	//! evaluated cartesian coords
	double m_CartesianCoords[3];
	//! evaluated cylindrical coords
	double m_CylindricalCoords[3];
};

//! Convert a given coordinate into another coordinate system
CSXCAD_EXPORT double* TransformCoordSystem(const double* in, double* out, CoordinateSystem CS_In, CoordinateSystem CS_out);

#endif // PARAMETERCOORD_H
