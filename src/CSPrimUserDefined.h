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

//! User defined Primitive given by an analytic formula
/*!
 This primitive is defined by a boolean result analytic formula. If a given coordinate results in a true result the primitive is assumed existing at these coordinate.
 */
class CSXCAD_EXPORT CSPrimUserDefined: public CSPrimitives
{
public:
	enum UserDefinedCoordSystem
	{
		CARESIAN_SYSTEM,CYLINDER_SYSTEM,SPHERE_SYSTEM
	};
	CSPrimUserDefined(ParameterSet* paraSet, CSProperties* prop);
	CSPrimUserDefined(CSPrimUserDefined* primUDef, CSProperties *prop=NULL);
	CSPrimUserDefined(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimUserDefined();

	virtual CSPrimUserDefined* GetCopy(CSProperties *prop=NULL) {return new CSPrimUserDefined(this,prop);}

	void SetCoordSystem(UserDefinedCoordSystem newSystem);
	UserDefinedCoordSystem GetCoordSystem() {return CoordSystem;}

	void SetCoordShift(int index, double val) {if ((index>=0) && (index<3)) dPosShift[index].SetValue(val);}
	void SetCoordShift(int index, const char* val) {if ((index>=0) && (index<3)) dPosShift[index].SetValue(val);}

	double GetCoordShift(int index) {if ((index>=0) && (index<3)) return dPosShift[index].GetValue(); else return 0;}
	ParameterScalar* GetCoordShiftPS(int index) {if ((index>=0) && (index<3)) return &dPosShift[index]; else return NULL;}

	void SetFunction(const char* func);
	const char* GetFunction() {return stFunction.c_str();}

	virtual bool GetBoundBox(double dBoundBox[6], bool PreserveOrientation=false);
	virtual bool IsInside(const double* Coord, double tol=0);

	virtual bool Update(std::string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	std::string stFunction;
	UserDefinedCoordSystem CoordSystem;
	CSFunctionParser* fParse;
	std::string fParameter;
	int iQtyParameter;
	ParameterScalar dPosShift[3];
};
