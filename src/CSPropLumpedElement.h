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

#include "CSProperties.h"

//! Continuous Structure Lumped Element Property
/*!
  This property represents lumped elements, e.g. smd capacitors etc.
  */
class CSXCAD_EXPORT CSPropLumpedElement : public CSProperties
{
public:
	CSPropLumpedElement(ParameterSet* paraSet);
	CSPropLumpedElement(CSProperties* prop);
	CSPropLumpedElement(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropLumpedElement();

	virtual void Init();

	void SetResistance(double val)			{m_R.SetValue(val);}
	int SetResistance(const std::string val)		{return m_R.SetValue(val);}
	double GetResistance() const			{return m_R.GetValue();}
	const std::string GetResistanceTerm() const	{return m_R.GetString();}

	void SetCapacity(double val)			{m_C.SetValue(val);}
	int SetCapacity(const std::string val)		{return m_C.SetValue(val);}
	double GetCapacity() const				{return m_C.GetValue();}
	const std::string GetCapacityTerm() const	{return m_C.GetString();}

	void SetInductance(double val)			{m_L.SetValue(val);}
	int SetInductance(const std::string val)		{return m_L.SetValue(val);}
	double GetInductance() const			{return m_L.GetValue();}
	const std::string GetInductanceTerm() const	{return m_L.GetString();}

	void SetDirection(int ny);
	int GetDirection() const {return m_ny;}

	void SetCaps(bool val) {m_Caps=val;}
	int GetCaps() const {return m_Caps;}

	virtual void ShowPropertyStatus(std::ostream& stream);

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const std::string GetTypeXMLString() const {return std::string("LumpedElement");}

protected:
	int m_ny;
	bool m_Caps;
	ParameterScalar m_R,m_C,m_L;
	virtual bool Update(std::string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);
};

