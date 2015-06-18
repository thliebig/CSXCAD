/*
*	Copyright (C) 2013 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#include "CSBackgroundMaterial.h"

#include "tinyxml.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

CSBackgroundMaterial::CSBackgroundMaterial()
{
	// init material
	Reset();
}

void CSBackgroundMaterial::Reset()
{
	SetEpsilon(1);
	SetMue(1);
	SetKappa(0);
	SetSigma(0);
}

void CSBackgroundMaterial::SetEpsilon(double val)
{
	if (val<1)
	{
		std::cerr << __func__ << ": Error, a relative electric permittivity smaller 1 is not allowed! Skipping. " << std::endl;
		return;
	}
	m_epsR=val;
}

void CSBackgroundMaterial::SetMue(double val)
{
	if (val<1)
	{
		std::cerr << __func__ << ": Error, a relative magnetic permeability smaller 1 is not allowed! Skipping. " << std::endl;
		return;
	}
	m_mueR=val;
}

void CSBackgroundMaterial::SetKappa(double val)
{
	if (val<0)
	{
		std::cerr << __func__ << ": Error, a negative electric conductivity is not allowed! Skipping. " << std::endl;
		return;
	}
	m_kappa=val;
}

void CSBackgroundMaterial::SetSigma(double val)
{
	if (val<0)
	{
		std::cerr << __func__ << ": Error, a negative (artificial) magnetic conductivity is not allowed! Skipping. " << std::endl;
		return;
	}
	m_sigma=val;
}

bool CSBackgroundMaterial::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	UNUSED(parameterised);
	UNUSED(sparse);
	TiXmlElement bg_elem("BackgroundMaterial");

	bg_elem.SetDoubleAttribute("Epsilon", GetEpsilon());
	bg_elem.SetDoubleAttribute("Mue", GetMue());
	bg_elem.SetDoubleAttribute("Kappa", GetKappa());
	bg_elem.SetDoubleAttribute("Sigma", GetSigma());

	root.InsertEndChild(bg_elem);

	return true;
}

bool CSBackgroundMaterial::ReadFromXML(TiXmlNode &root)
{
	Reset();
	TiXmlElement* rootElem=root.ToElement();

	if (rootElem==NULL)
		return false;

	double val;
	if (rootElem->QueryDoubleAttribute("Epsilon",&val) == TIXML_SUCCESS)
		SetEpsilon(val);
	if (rootElem->QueryDoubleAttribute("Mue",&val) == TIXML_SUCCESS)
		SetMue(val);
	if (rootElem->QueryDoubleAttribute("Kappa",&val) == TIXML_SUCCESS)
		SetKappa(val);
	if (rootElem->QueryDoubleAttribute("Sigma",&val) == TIXML_SUCCESS)
		SetSigma(val);

	return true;
}
