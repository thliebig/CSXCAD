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
#include "CSPrimCylinder.h"

//! CylindicalShell Primitive
/*!
 This is a cylindrical shell primitive derived from the cylinder primitive, adding a shell width which is centered around the cylinder radius.
 \sa CSPrimCylinder
 */
class CSXCAD_EXPORT CSPrimCylindricalShell : public CSPrimCylinder
{
public:
	CSPrimCylindricalShell(ParameterSet* paraSet, CSProperties* prop);
	CSPrimCylindricalShell(CSPrimCylindricalShell* cylinder, CSProperties *prop=NULL);
	CSPrimCylindricalShell(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimCylindricalShell();

	virtual CSPrimitives* GetCopy(CSProperties *prop=NULL) {return new CSPrimCylindricalShell(this,prop);}

	void SetShellWidth(double val) {psShellWidth.SetValue(val);}
	void SetShellWidth(const char* val) {psShellWidth.SetValue(val);}

	double GetShellWidth() {return psShellWidth.GetValue();}
	ParameterScalar* GetShellWidthPS() {return &psShellWidth;}

	virtual bool IsInside(const double* Coord, double tol=0);

	virtual bool Update(std::string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void ShowPrimitiveStatus(std::ostream& stream);

protected:
	ParameterScalar psShellWidth;
	virtual double GetBBRadius() {return psRadius.GetValue()+psShellWidth.GetValue()/2.0;} // Get the radius for the bounding box calculation
};

