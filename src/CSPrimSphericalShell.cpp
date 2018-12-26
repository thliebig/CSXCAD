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

#include <sstream>
#include <iostream>
#include <limits>
#include "tinyxml.h"
#include "stdint.h"

#include "CSPrimSphericalShell.h"
#include "CSProperties.h"
#include "CSUseful.h"

CSPrimSphericalShell::CSPrimSphericalShell(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimSphere(ID,paraSet,prop)
{
	Type=SPHERICALSHELL;
	PrimTypeName = std::string("SphericalShell");
	psShellWidth.SetParameterSet(paraSet);
}

CSPrimSphericalShell::CSPrimSphericalShell(CSPrimSphericalShell* sphere, CSProperties *prop) : CSPrimSphere(sphere,prop)
{
	Type=SPHERICALSHELL;
	PrimTypeName = std::string("SphericalShell");
	psShellWidth.Copy(&sphere->psShellWidth);
}

CSPrimSphericalShell::CSPrimSphericalShell(ParameterSet* paraSet, CSProperties* prop) : CSPrimSphere(paraSet,prop)
{
	Type=SPHERICALSHELL;
	PrimTypeName = std::string("SphericalShell");
	psShellWidth.SetParameterSet(paraSet);
}


CSPrimSphericalShell::~CSPrimSphericalShell()
{
}

bool CSPrimSphericalShell::GetBoundBox(double dBoundBox[6], bool PreserveOrientation)
{
	UNUSED(PreserveOrientation); //has no orientation or preserved anyways
	const double* center = m_Center.GetCartesianCoords();
	m_BoundBox_CoordSys=CARTESIAN;
	double radius = psRadius.GetValue();
	double shellwidth = psShellWidth.GetValue();
	for (unsigned int i=0;i<3;++i)
	{
		dBoundBox[2*i]=center[i]-radius-shellwidth/2.0;
		dBoundBox[2*i+1]=center[i]+radius+shellwidth/2.0;
	}
	if (shellwidth>0)
		m_Dimension=3;
	else if (radius>0)
		m_Dimension=1;
	else
		m_Dimension=0;
	return true;
}

bool CSPrimSphericalShell::IsInside(const double* Coord, double /*tol*/)
{
	if (Coord==NULL) return false;
	double out[3];
	const double* center = m_Center.GetCartesianCoords();
	TransformCoordSystem(Coord,out,m_MeshType,CARTESIAN);
	if (m_Transform)
		m_Transform->InvertTransform(out,out);
	double dist=sqrt(pow(out[0]-center[0],2)+pow(out[1]-center[1],2)+pow(out[2]-center[2],2));
	if (fabs(dist-psRadius.GetValue())< psShellWidth.GetValue()/2.0)
		return true;
	return false;
}

bool CSPrimSphericalShell::Update(std::string *ErrStr)
{
	int EC=0;
	bool bOK=CSPrimSphere::Update(ErrStr);

	EC=psShellWidth.Evaluate();
	if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::PS_NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		std::stringstream stream;
		stream << std::endl << "Error in " << PrimTypeName << " shell-width (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

	//update local bounding box
	m_BoundBoxValid = GetBoundBox(m_BoundBox);

	return bOK;
}

bool CSPrimSphericalShell::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimSphere::Write2XML(elem,parameterised);

	WriteTerm(psShellWidth,elem,"ShellWidth",parameterised);
	return true;
}

bool CSPrimSphericalShell::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimSphere::ReadFromXML(root)==false) return false;

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;
	if (ReadTerm(psShellWidth,*elem,"ShellWidth")==false) return false;

	return true;
}

void CSPrimSphericalShell::ShowPrimitiveStatus(std::ostream& stream)
{
	CSPrimSphere::ShowPrimitiveStatus(stream);
	stream << "  Shell width: " << psShellWidth.GetValueString() << std::endl;
}
