/*
*	Copyright (C) 2008-2025 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#include "CSProperties.h"
#include "CSPropMaterial.h"

//! Continuous Structure Dispersive Material Property
/*!
  This abstarct Property can hold information about the special properties of dispersive materials.
  */
class CSXCAD_EXPORT CSPropDispersiveMaterial : public CSPropMaterial
{
public:
	virtual ~CSPropDispersiveMaterial();

	//! Get the dispersion order
	virtual int GetDispersionOrder() {return m_Order;}

	//! Set the dispersion order. This will reset all values!
	virtual void SetDispersionOrder(int);

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const std::string GetTypeXMLString() const {return std::string("DispersiveMaterial");}

protected:
	int m_Order;
	virtual void Init();
	virtual void InitValues() {};
	virtual void DeleteValues() {};

	virtual bool Update(std::string *ErrStr=NULL);
	double GetValue3(ParameterScalar **ps, int order, int ny);
	std::string GetTerm3(ParameterScalar **ps, int order, int ny);
	void SetValue3(double val, ParameterScalar **ps, int order, int ny);
	int SetValue3(std::string val, ParameterScalar **ps, int order, int ny);
	double GetWeight3(ParameterScalar **ps, int order, int ny, const double* coords);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

	CSPropDispersiveMaterial(ParameterSet* paraSet);
	CSPropDispersiveMaterial(CSPropDispersiveMaterial* prop, bool copyPrim=false);
	CSPropDispersiveMaterial(unsigned int ID, ParameterSet* paraSet);

	//! Create a copy of this property. Optional: Copy all primitives assigned to this property too.
	virtual CSProperties* GetCopy(bool incl_prim=false) {return new CSPropDispersiveMaterial(this, incl_prim);}
};
