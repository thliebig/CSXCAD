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

#ifndef CSBACKGROUNDMATERIAL_H
#define CSBACKGROUNDMATERIAL_H

#include "CSXCAD_Global.h"

class TiXmlNode;

class CSBackgroundMaterial
{
public:
	CSBackgroundMaterial();

	//! Get the rel. electric permittivity
	double GetEpsilon() const {return m_epsR;}
	//! Set the rel. electric permittivity
	void SetEpsilon(double val);

	//! Get the rel. magnetic permeability
	double GetMue() const {return m_mueR;}
	//! Set the rel. magnetic permeability
	void SetMue(double val);

	//! Get the electric conductivity
	double GetKappa() const {return m_kappa;}
	//! Set the electric conductivity
	void SetKappa(double val);

	//! Get the (artificial) magnetic conductivity
	double GetSigma() const {return m_sigma;}
	//! Set the (artificial) magnetic conductivity
	void SetSigma(double val);

	//! Reset all values to default
	void Reset();

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	double m_epsR;
	double m_mueR;
	double m_kappa;
	double m_sigma;
};

#endif // CSBACKGROUNDMATERIAL_H
