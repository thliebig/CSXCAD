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

#include "CSPrimMultiBox.h"
#include "CSProperties.h"
#include "CSUseful.h"

CSPrimMultiBox::CSPrimMultiBox(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=MULTIBOX;
	PrimTypeName = std::string("Multi Box");
}

CSPrimMultiBox::CSPrimMultiBox(CSPrimMultiBox* multiBox, CSProperties *prop) : CSPrimitives(multiBox, prop)
{
	Type=MULTIBOX;
	for (size_t i=0;i<multiBox->vCoords.size();++i)
		vCoords.push_back(new ParameterScalar(multiBox->vCoords.at(i)));
	PrimTypeName = std::string("Multi Box");
}

CSPrimMultiBox::CSPrimMultiBox(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=MULTIBOX;
	PrimTypeName = std::string("Multi Box");
}

CSPrimMultiBox::~CSPrimMultiBox()
{
}

CSPrimitives* CSPrimMultiBox::GetCopy(CSProperties *prop) {return new CSPrimMultiBox(this,prop);}

void CSPrimMultiBox::SetCoord(int index, double val)
{
	if ((index>=0) && (index<(int)vCoords.size()))
		vCoords.at(index)->SetValue(val);
}

void CSPrimMultiBox::SetCoord(int index, const char* val)
{
	if ((index>=0) && (index<(int)vCoords.size()))
		vCoords.at(index)->SetValue(val);
}

void CSPrimMultiBox::AddCoord(double val)
{
	vCoords.push_back(new ParameterScalar(clParaSet,val));
}

void CSPrimMultiBox::AddCoord(const char* val)
{
	vCoords.push_back(new ParameterScalar(clParaSet,val));
}

void CSPrimMultiBox::AddBox(int initBox)
{
	ClearOverlap();
	if ((initBox<0) || (((initBox+1)*6)>(int)vCoords.size()))
	{
		for (unsigned int i=0;i<6;++i)
			AddCoord(0.0);
	}
	else for (unsigned int i=0;i<6;++i)
		vCoords.push_back(new ParameterScalar(vCoords.at(6*initBox+i)));
}

void CSPrimMultiBox::DeleteBox(size_t box)
{
	if ((box+1)*6>vCoords.size()) return;
	std::vector<ParameterScalar*>::iterator start=vCoords.begin()+(box*6);
	std::vector<ParameterScalar*>::iterator end=vCoords.begin()+(box*6+6);

	vCoords.erase(start,end);
}


double CSPrimMultiBox::GetCoord(int index)
{
	if ((index>=0) && (index<(int)vCoords.size()))
		return vCoords.at(index)->GetValue();
	return 0;
}

ParameterScalar* CSPrimMultiBox::GetCoordPS(int index)
{
	if ((index>=0) && (index<(int)vCoords.size()))
		return vCoords.at(index);
	return NULL;
}

double* CSPrimMultiBox::GetAllCoords(size_t &Qty, double* array)
{
	Qty=vCoords.size();
	delete[] array;
	array = new double[Qty];
	for (size_t i=0;i<Qty;++i)
		array[i]=vCoords.at(i)->GetValue();
	return array;
}

void CSPrimMultiBox::ClearOverlap()
{
	if (vCoords.size()%6==0) return;  //no work to be done

	vCoords.resize(vCoords.size()-vCoords.size()%6);
}

bool CSPrimMultiBox::GetBoundBox(double dBoundBox[6], bool PreserveOrientation)
{
	UNUSED(PreserveOrientation); //has no orientation or preserved anyways
	for (int n=0;n<6;++n) dBoundBox[n] = 0;
	//Update();
	for (unsigned int i=0;i<vCoords.size()/6;++i)
	{
		for (unsigned int n=0;n<3;++n)
		{
			if (vCoords.at(6*i+2*n)->GetValue()<=vCoords.at(6*i+2*n+1)->GetValue())
			{
				if (i==0)
				{
					dBoundBox[2*n]=vCoords.at(6*i+2*n)->GetValue();
					dBoundBox[2*n+1]=vCoords.at(6*i+2*n+1)->GetValue();
				}
				else
				{
					if (vCoords.at(6*i+2*n)->GetValue()<dBoundBox[2*n]) dBoundBox[2*n]=vCoords.at(6*i+2*n)->GetValue();
					if (vCoords.at(6*i+2*n+1)->GetValue()>dBoundBox[2*n+1]) dBoundBox[2*n+1]=vCoords.at(6*i+2*n+1)->GetValue();
				}

			}
			else
			{
				if (i==0)
				{
					dBoundBox[2*n]=vCoords.at(6*i+2*n+1)->GetValue();
					dBoundBox[2*n+1]=vCoords.at(6*i+2*n)->GetValue();
				}
				else
				{
					if (vCoords.at(6*i+2*n+1)->GetValue()<dBoundBox[2*n]) dBoundBox[2*n]=vCoords.at(6*i+2*n+1)->GetValue();
					if (vCoords.at(6*i+2*n)->GetValue()>dBoundBox[2*n+1]) dBoundBox[2*n+1]=vCoords.at(6*i+2*n)->GetValue();
				}
			}
		}
	}
	m_Dimension=0;
	m_BoundBox_CoordSys = m_MeshType;
	for (int n=0;n<3;++n)
	{
		if (dBoundBox[2*n]!=dBoundBox[2*n+1])
			++m_Dimension;
	}
	return false;
}

bool CSPrimMultiBox::IsInside(const double* Coord, double /*tol*/)
{
	if (Coord==NULL) return false;
	bool in=false;
	double UpVal,DownVal;
	double coords[3]={Coord[0],Coord[1],Coord[2]};
	TransformCoords(coords, true, m_MeshType);
	//fprintf(stderr,"here\n");
	for (unsigned int i=0;i<vCoords.size()/6;++i)
	{
		in=true;
		for (unsigned int n=0;n<3;++n)
		{
			//fprintf(stderr,"%e %e %e \n",vCoords.at(6*i+2*n)->GetValue(),vCoords.at(6*i+2*n+1)->GetValue());
			UpVal=vCoords.at(6*i+2*n+1)->GetValue();
			DownVal=vCoords.at(6*i+2*n)->GetValue();
			if (DownVal<UpVal)
			{
				if (DownVal>coords[n]) {in=false;break;}
				if (UpVal<coords[n]) {in=false;break;}
			}
			else
			{
				if (DownVal<coords[n]) {in=false;break;}
				if (UpVal>coords[n]) {in=false;break;}
			}
		}
		if (in==true) {	return true;}
	}
	return false;
}

unsigned int CSPrimMultiBox::GetQtyBoxes() {return (unsigned int) vCoords.size()/6;}

bool CSPrimMultiBox::Update(std::string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	for (size_t i=0;i<vCoords.size();++i)
	{
		EC=vCoords.at(i)->Evaluate();
		if (EC!=ParameterScalar::PS_NO_ERROR) bOK=false;
		if ((EC!=0)  && (ErrStr!=NULL))
		{
			bOK=false;
			std::stringstream stream;
			stream << std::endl << "Error in MultiBox (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}
	//update local bounding box
	m_BoundBoxValid = GetBoundBox(m_BoundBox);
	return bOK;
}

bool CSPrimMultiBox::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimitives::Write2XML(elem,parameterised);
	elem.SetAttribute("QtyBox",(int)vCoords.size()/6);

	for (size_t i=0;i<vCoords.size()/6;++i)
	{
		TiXmlElement SP("StartP");
		WriteTerm(*vCoords.at(i*6),SP,"X",parameterised);
		WriteTerm(*vCoords.at(i*6+2),SP,"Y",parameterised);
		WriteTerm(*vCoords.at(i*6+4),SP,"Z",parameterised);
		elem.InsertEndChild(SP);

		TiXmlElement EP("EndP");
		WriteTerm(*vCoords.at(i*6+1),EP,"X",parameterised);
		WriteTerm(*vCoords.at(i*6+3),EP,"Y",parameterised);
		WriteTerm(*vCoords.at(i*6+5),EP,"Z",parameterised);
		elem.InsertEndChild(EP);
	}
	return true;
}

bool CSPrimMultiBox::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;;

	TiXmlElement *SP=root.FirstChildElement("StartP");
	TiXmlElement *EP=root.FirstChildElement("EndP");
	if (vCoords.size()!=0) return false;
	int i=0;
	while ((SP!=NULL) && (EP!=NULL))
	{
		for (int n=0;n<6;++n) this->AddCoord(0.0);

		if (ReadTerm(*vCoords.at(i*6),*SP,"X")==false) return false;
		if (ReadTerm(*vCoords.at(i*6+2),*SP,"Y")==false) return false;
		if (ReadTerm(*vCoords.at(i*6+4),*SP,"Z")==false) return false;

		if (ReadTerm(*vCoords.at(i*6+1),*EP,"X")==false) return false;
		if (ReadTerm(*vCoords.at(i*6+3),*EP,"Y")==false) return false;
		if (ReadTerm(*vCoords.at(i*6+5),*EP,"Z")==false) return false;

//		for (int n=0;n<6;++n) fprintf(stderr,"%e ",vCoords.at(i*6+n)->GetValue());
//		fprintf(stderr,"\n");

		SP=SP->NextSiblingElement("StartP");
		EP=EP->NextSiblingElement("EndP");
		++i;
	};
	return true;
}
