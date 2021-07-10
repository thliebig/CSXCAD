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

#include "CSPrimCylindricalShell.h"
#include "CSProperties.h"
#include "CSUseful.h"

CSPrimCylindricalShell::CSPrimCylindricalShell(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimCylinder(ID,paraSet,prop)
{
	Type=CYLINDRICALSHELL;
	PrimTypeName = std::string("CylindricalShell");
	psShellWidth.SetParameterSet(paraSet);
}

CSPrimCylindricalShell::CSPrimCylindricalShell(CSPrimCylindricalShell* cylinder, CSProperties *prop) : CSPrimCylinder(cylinder,prop)
{
	Type=CYLINDRICALSHELL;
	PrimTypeName = std::string("CylindricalShell");
	psShellWidth.Copy(&cylinder->psShellWidth);
}

CSPrimCylindricalShell::CSPrimCylindricalShell(ParameterSet* paraSet, CSProperties* prop) : CSPrimCylinder(paraSet,prop)
{
	Type=CYLINDRICALSHELL;
	PrimTypeName = std::string("CylindricalShell");
	psShellWidth.SetParameterSet(paraSet);
}

CSPrimCylindricalShell::~CSPrimCylindricalShell()
{
}

bool CSPrimCylindricalShell::GetBoundBox(double dBoundBox[6], bool PreserveOrientation)
{
//	cerr << "CSPrimCylindricalShell::GetBoundBox: Warning: The bounding box for this object is not calculated properly... " << std::endl;
	UNUSED(PreserveOrientation); //has no orientation or preserved anyways
	bool accurate=false;
	int Direction=0;
	const double* start=m_AxisCoords[0].GetCartesianCoords();
	const double* stop =m_AxisCoords[1].GetCartesianCoords();
	m_BoundBox_CoordSys=CARTESIAN;
	double rad=psRadius.GetValue()+psShellWidth.GetValue()/2.0;
        // unit vector in direction of cylinder axis
        double v[3];
        int vnonzero = -1;  // TODO handle zero-height cylinders
        unsigned int zero_count = 0;
        for (unsigned int i=0; i<3; ++i)
        {
                v[i] = stop[i] - start[i];
                if (v[i] != 0)
                {
                        vnonzero = i;
                }
                else
                {
                        ++zero_count;
                        Direction+=pow(2,i);
                }
        }

        if (zero_count == 2)
        {
                bool flip = start[vnonzero] > stop[vnonzero];
                for (unsigned int i=0; i<3; ++i)
                {
                        if (i == vnonzero)
                        {
                                if (!flip)
                                {
                                        dBoundBox[2*i] = start[i];
                                        dBoundBox[2*i+1] = stop[i];
                                }
                                else
                                {
                                        dBoundBox[2*i] = stop[i];
                                        dBoundBox[2*i+1] = start[i];
                                }
                        }
                        else
                        {
                                if (!flip)
                                {
                                        dBoundBox[2*i] = -rad + start[i];
                                        dBoundBox[2*i+1] = rad + start[i];
                                }
                                else
                                {
                                        dBoundBox[2*i] = rad + start[i];
                                        dBoundBox[2*i+1] = -rad + start[i];
                                }
                        }
                }
        }
        else
        {
                NormalizeVector(v);

                // some unit vector perpendicular to cylinder axis
                double a[3];
                for (unsigned int i=0; i<3; ++i)
                {
                        if (i == vnonzero)
                        {
                                unsigned int i1 = (i + 1) % 3;
                                unsigned int i2 = (i + 2) % 3;
                                a[i] = (v[i1] + v[i2]) / v[i];
                        }
                        else
                        {
                                a[i] = -1;
                        }
                }
                NormalizeVector(a);

                // unit vector perpendicular to cylinder axis and a
                double b[3] = {a[1]*v[2] - a[2]*v[1], -a[0]*v[2] + a[2]*v[0], a[0]*v[1] - a[1]*v[0]};

                double theta[3];
                for (unsigned int i=0; i<3; ++i)
                {
                        theta[i] = atan(b[i]/a[i]);
                }

                double min[3] = {start[0], start[1], start[2]};
                double max[3] = {start[0], start[1], start[2]};

                for (unsigned int i=0; i<2; ++i)
                {
                        const double* coord = m_AxisCoords[i].GetCartesianCoords();
                        for (unsigned int j=0; j<3; ++j)
                        {
                                double val = coord[j] + rad*a[j]*cos(theta[j]) + rad*b[j]*sin(theta[j]);
                                double val2 = coord[j] + rad*a[j]*cos(theta[j]+PI) + rad*b[j]*sin(theta[j]+PI);
                                min[j] = fmin(min[j], val);
                                min[j] = fmin(min[j], val2);
                                max[j] = fmax(max[j], val);
                                max[j] = fmax(max[j], val2);
                        }
                }

                for (unsigned int i=0; i<3; ++i)
                {
                        if (start[i] <= stop[i])
                        {
                                dBoundBox[2*i]=min[i];
                                dBoundBox[2*i+1]=max[i];
                        }
                        else
                        {
                                dBoundBox[2*i]=max[i];
                                dBoundBox[2*i+1]=min[i];
                        }
                }
        }

	if (rad>0)
		m_Dimension=3;
	else if (Direction==7)
		m_Dimension=0;
	else
		m_Dimension=1;
	return accurate;
}

bool CSPrimCylindricalShell::IsInside(const double* Coord, double /*tol*/)
{
	if (Coord==NULL) return false;
	const double* start=m_AxisCoords[0].GetCartesianCoords();
	const double* stop =m_AxisCoords[1].GetCartesianCoords();
	double pos[3];
	//transform incoming coordinates into cartesian coords
	TransformCoordSystem(Coord,pos,m_MeshType,CARTESIAN);
	if (m_Transform)
		m_Transform->InvertTransform(pos,pos);

	for (int n=0;n<3;++n)
		if (pos[n]<m_BoundBox[2*n] || pos[n]>m_BoundBox[2*n+1])
			return false;

	double foot,dist;
	Point_Line_Distance(pos,start,stop,foot,dist);

	if ((foot<0) || (foot>1)) //the foot point is not on the axis
		return false;
	if (fabs(dist-psRadius.GetValue())>psShellWidth.GetValue()/2.0)
		return false;

	return true;
}

bool CSPrimCylindricalShell::Update(std::string *ErrStr)
{
	int EC=0;
	bool bOK=CSPrimCylinder::Update(ErrStr);

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

bool CSPrimCylindricalShell::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimCylinder::Write2XML(elem,parameterised);

	WriteTerm(psShellWidth,elem,"ShellWidth",parameterised);
	return true;
}

bool CSPrimCylindricalShell::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimCylinder::ReadFromXML(root)==false) return false;

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;
	if (ReadTerm(psShellWidth,*elem,"ShellWidth")==false) return false;
	return true;
}

void CSPrimCylindricalShell::ShowPrimitiveStatus(std::ostream& stream)
{
	CSPrimCylinder::ShowPrimitiveStatus(stream);
	stream << "  Shell width: " << psShellWidth.GetValueString() << std::endl;
}
