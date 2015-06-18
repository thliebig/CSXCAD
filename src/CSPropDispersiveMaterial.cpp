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

#include "tinyxml.h"

#include "CSPropDispersiveMaterial.h"

CSPropDispersiveMaterial::CSPropDispersiveMaterial(ParameterSet* paraSet) : CSPropMaterial(paraSet) {m_Order=0;Type=(CSProperties::PropertyType)(DISPERSIVEMATERIAL | MATERIAL);}
CSPropDispersiveMaterial::CSPropDispersiveMaterial(CSProperties* prop) : CSPropMaterial(prop) {m_Order=0;Type=(CSProperties::PropertyType)(DISPERSIVEMATERIAL | MATERIAL);}
CSPropDispersiveMaterial::CSPropDispersiveMaterial(unsigned int ID, ParameterSet* paraSet) : CSPropMaterial(ID,paraSet) {m_Order=0;Type=(CSProperties::PropertyType)(DISPERSIVEMATERIAL | MATERIAL);}
CSPropDispersiveMaterial::~CSPropDispersiveMaterial() {}

bool CSPropDispersiveMaterial::Update(std::string *ErrStr)
{
	return CSPropMaterial::Update(ErrStr);
}

bool CSPropDispersiveMaterial::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	return CSPropMaterial::Write2XML(root,parameterised,sparse);
}

bool CSPropDispersiveMaterial::ReadFromXML(TiXmlNode &root)
{
	return CSPropMaterial::ReadFromXML(root);
}
