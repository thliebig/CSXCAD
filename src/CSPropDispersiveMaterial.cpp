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

#include "tinyxml.h"

#include "CSPropDispersiveMaterial.h"

CSPropDispersiveMaterial::CSPropDispersiveMaterial(ParameterSet* paraSet) : CSPropMaterial(paraSet) {Type=(CSProperties::PropertyType)(DISPERSIVEMATERIAL | MATERIAL);Init();}
CSPropDispersiveMaterial::CSPropDispersiveMaterial(CSPropDispersiveMaterial* prop, bool copyPrim) : CSPropMaterial(prop, copyPrim)
{
	Init();
	SetDispersionOrder(prop->m_Order);
	Type=(CSProperties::PropertyType)(DISPERSIVEMATERIAL | MATERIAL);
}
CSPropDispersiveMaterial::CSPropDispersiveMaterial(unsigned int ID, ParameterSet* paraSet) : CSPropMaterial(ID,paraSet) {Type=(CSProperties::PropertyType)(DISPERSIVEMATERIAL | MATERIAL);Init();}
CSPropDispersiveMaterial::~CSPropDispersiveMaterial() {m_Order = 0;}

bool CSPropDispersiveMaterial::Update(std::string *ErrStr)
{
	return CSPropMaterial::Update(ErrStr);
}

void CSPropDispersiveMaterial::Init()
{
	m_Order = 0;
}

void CSPropDispersiveMaterial::SetDispersionOrder(int val)
{
	if (m_Order>0)
		DeleteValues();
	m_Order=val;
	this->InitValues();
}

double CSPropDispersiveMaterial::GetValue3(ParameterScalar **ps, int order, int ny)
{
	if ((ny>2) || (ny<0)) return 0;
	if ((order>=m_Order) || (order<0)) return 0;
	return this->GetValue(ps[order], ny);
}

std::string CSPropDispersiveMaterial::GetTerm3(ParameterScalar **ps, int order, int ny)
{
	if ((ny>2) || (ny<0)) return 0;
	if ((order>=m_Order) || (order<0)) return 0;
	return this->GetTerm(ps[order], ny);
}

void CSPropDispersiveMaterial::SetValue3(double val, ParameterScalar **ps, int order, int ny)
{
	if ((ny>2) || (ny<0)) return;
	if ((order>=m_Order) || (order<0)) return;
	this->SetValue(val, ps[order], ny);
}

int CSPropDispersiveMaterial::SetValue3(std::string val, ParameterScalar **ps, int order, int ny)
{
	if ((ny>2) || (ny<0)) return 0;
	if ((order>=m_Order) || (order<0)) return 0;
	return this->SetValue(val, ps[order], ny);
}

double CSPropDispersiveMaterial::GetWeight3(ParameterScalar **ps, int order, int ny, const double* coords)
{
	if ((ny>2) || (ny<0)) return 0;
	if ((order>=m_Order) || (order<0)) return 0;
	return this->GetWeight(ps[order], ny, coords);
}

bool CSPropDispersiveMaterial::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	return CSPropMaterial::Write2XML(root,parameterised,sparse);
}

bool CSPropDispersiveMaterial::ReadFromXML(TiXmlNode &root)
{
	return CSPropMaterial::ReadFromXML(root);
}
