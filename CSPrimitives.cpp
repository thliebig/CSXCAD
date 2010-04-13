/*
*	Copyright (C) 2008,2009,2010 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#include "CSPrimitives.h"
#include "CSProperties.h"
#include <sstream>
#include <iostream>
#include <limits>
#include "tinyxml.h"
#include "CSFunctionParser.h"


void Point_Line_Distance(double P[], double start[], double stop[], double &foot, double &dist)
{
	double dir[] = {stop[0]-start[0],stop[1]-start[1],stop[2]-start[2]};

	double LL = pow(dir[0],2)+pow(dir[1],2)+pow(dir[2],2); //Line length^2
	foot = (P[0]-start[0])*dir[0] +  (P[1]-start[1])*dir[1] + (P[2]-start[2])*dir[2];
	foot /= LL;

	double footP[] = {start[0] + foot*dir[0], start[1] + foot*dir[1], start[2] + foot*dir[2]};

	dist = sqrt(pow(P[0]-footP[0],2)+pow(P[1]-footP[1],2)+pow(P[2]-footP[2],2));
}

/*********************CSPrimitives********************************************************************/
CSPrimitives::CSPrimitives(unsigned int ID, ParameterSet* paraSet, CSProperties* prop)
{
	for (int i=0;i<6;++i) dBoundBox[i]=0;
	clProperty=NULL;
	SetProperty(prop);
	uiID=ID;
	clParaSet=paraSet;
	iPriority=0;
	PrimTypeName = string("Base Type");
}

CSPrimitives::CSPrimitives(CSPrimitives* prim, CSProperties *prop)
{
	for (int i=0;i<6;++i) dBoundBox[i]=prim->dBoundBox[i];
	clProperty=NULL;
	if (prop==NULL) SetProperty(prim->clProperty);
	else SetProperty(prop);
	uiID=prim->uiID;
	clParaSet=prim->clParaSet;
	iPriority=prim->iPriority;
	PrimTypeName = string("Base Type");
}


CSPrimitives::CSPrimitives(ParameterSet* paraSet, CSProperties* prop)
{
	for (int i=0;i<6;++i) dBoundBox[i]=0;
	clProperty=NULL;
	SetProperty(prop);
	clParaSet=paraSet;
	uiID=0;
	iPriority=0;
	PrimTypeName = string("Base Type");
}

void CSPrimitives::SetProperty(CSProperties *prop)
{
	if (clProperty!=NULL) clProperty->RemovePrimitive(this);
	clProperty=prop;
	if (prop!=NULL) prop->AddPrimitive(this);
}

CSPrimitives::~CSPrimitives()
{
	if (clProperty!=NULL) clProperty->RemovePrimitive(this);
}

bool CSPrimitives::Write2XML(TiXmlElement &elem, bool /*parameterised*/)
{
	elem.SetAttribute("ID",uiID);
//	if (clProperty!=NULL) elem.SetAttribute("PropertyID",clProperty->GetID());
//	else elem.SetAttribute("PropertyID",-1);
	elem.SetAttribute("Priority",iPriority);

	return true;
}

bool CSPrimitives::ReadFromXML(TiXmlNode &root)
{
	int help;
	TiXmlElement* elem=root.ToElement();
	if (elem==NULL) return false;
	if (elem->QueryIntAttribute("ID",&help)!=TIXML_SUCCESS) uiID=0;
	uiID=(unsigned int)help;
	if (elem->QueryIntAttribute("Priority",&iPriority)!=TIXML_SUCCESS) return false;

	return true;
}


/*********************CSPrimBox********************************************************************/
CSPrimBox::CSPrimBox(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=BOX;
	for (int i=0;i<6;++i) {psCoords[i].SetParameterSet(paraSet);}
	PrimTypeName = string("Box");
}

CSPrimBox::CSPrimBox(CSPrimBox* primBox, CSProperties *prop) : CSPrimitives(primBox,prop)
{
	Type=BOX;
	for (int i=0;i<6;++i) {psCoords[i]=ParameterScalar(primBox->psCoords[i]);}
	PrimTypeName = string("Box");
}

CSPrimBox::CSPrimBox(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=BOX;
	for (int i=0;i<6;++i) {psCoords[i].SetParameterSet(paraSet);}
	PrimTypeName = string("Box");
}


CSPrimBox::~CSPrimBox()
{
}

double* CSPrimBox::GetBoundBox(bool &accurate, bool PreserveOrientation)
{
	accurate=true;
	for (int i=0;i<6;++i) dBoundBox[i]=psCoords[i].GetValue();
	if (PreserveOrientation==true) return dBoundBox;
	for (int i=0;i<3;++i)
		if (dBoundBox[2*i]>dBoundBox[2*i+1])
		{
			double help=dBoundBox[2*i];
			dBoundBox[2*i]=dBoundBox[2*i+1];
			dBoundBox[2*i+1]=help;
		}
	return dBoundBox;
}

bool CSPrimBox::IsInside(double* Coord, double /*tol*/)
{
	if (Coord==NULL) return false;

	bool accBnd=false;
	double* box=this->GetBoundBox(accBnd);

	for (unsigned int n=0;n<3;++n)
	{
		if ((box[2*n]>Coord[n]) || (box[2*n+1]<Coord[n])) return false;
	}
	return true;
}


bool CSPrimBox::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	for (int i=0;i<6;++i)
	{
		EC=psCoords[i].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
		{
			bOK=false;
			stringstream stream;
			stream << endl << "Error in Box (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}
	return bOK;
}

bool CSPrimBox::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimitives::Write2XML(elem,parameterised);

	TiXmlElement P1("P1");
	WriteTerm(psCoords[0],P1,"X",parameterised);
	WriteTerm(psCoords[2],P1,"Y",parameterised);
	WriteTerm(psCoords[4],P1,"Z",parameterised);
	elem.InsertEndChild(P1);

	TiXmlElement P2("P2");
	WriteTerm(psCoords[1],P2,"X",parameterised);
	WriteTerm(psCoords[3],P2,"Y",parameterised);
	WriteTerm(psCoords[5],P2,"Z",parameterised);
	elem.InsertEndChild(P2);
	return true;
}

bool CSPrimBox::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;

	//P1
	TiXmlElement* P1=root.FirstChildElement("P1");
	if (P1==NULL) return false;
	if (ReadTerm(psCoords[0],*P1,"X")==false) return false;
	if (ReadTerm(psCoords[2],*P1,"Y")==false) return false;
	if (ReadTerm(psCoords[4],*P1,"Z")==false) return false;
	TiXmlElement* P2=root.FirstChildElement("P2");
	if (P1==NULL) return false;
	if (ReadTerm(psCoords[1],*P2,"X")==false) return false;
	if (ReadTerm(psCoords[3],*P2,"Y")==false) return false;
	if (ReadTerm(psCoords[5],*P2,"Z")==false) return false;

	return true;
}

/*********************CSPrimMultiBox********************************************************************/
CSPrimMultiBox::CSPrimMultiBox(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=MULTIBOX;
	PrimTypeName = string("Multi Box");
}

CSPrimMultiBox::CSPrimMultiBox(CSPrimMultiBox* multiBox, CSProperties *prop) : CSPrimitives(multiBox, prop)
{
	Type=MULTIBOX;
	for (size_t i=0;i<multiBox->vCoords.size();++i) vCoords.push_back(ParameterScalar(multiBox->vCoords.at(i)));
	PrimTypeName = string("Multi Box");
}

CSPrimMultiBox::CSPrimMultiBox(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=MULTIBOX;
	PrimTypeName = string("Multi Box");
}

CSPrimMultiBox::~CSPrimMultiBox()
{
}

void CSPrimMultiBox::SetCoord(int index, double val)
{
	if ((index>=0) && (index<(int)vCoords.size())) vCoords.at(index).SetValue(val);
}

void CSPrimMultiBox::SetCoord(int index, const char* val)
{
	if ((index>=0) && (index<(int)vCoords.size())) vCoords.at(index).SetValue(val);
}

void CSPrimMultiBox::AddCoord(double val)
{
	vCoords.push_back(ParameterScalar(clParaSet,val));
}

void CSPrimMultiBox::AddCoord(const char* val)
{
	vCoords.push_back(ParameterScalar(clParaSet,val));
}

void CSPrimMultiBox::AddBox(int initBox)
{
	ClearOverlap();
	if ((initBox<0) || (((initBox+1)*6)>(int)vCoords.size()))
	{
		for (unsigned int i=0;i<6;++i) AddCoord(0.0);
	}
	else for (unsigned int i=0;i<6;++i) vCoords.push_back(ParameterScalar(vCoords.at(6*initBox+i)));
}

void CSPrimMultiBox::DeleteBox(size_t box)
{
	if ((box+1)*6>vCoords.size()) return;
	vector<ParameterScalar>::iterator start=vCoords.begin()+(box*6);
	vector<ParameterScalar>::iterator end=vCoords.begin()+(box*6+6);

	vCoords.erase(start,end);
}


double CSPrimMultiBox::GetCoord(int index)
{
	if ((index>=0) && (index<(int)vCoords.size())) return vCoords.at(index).GetValue();
	return 0;
}

ParameterScalar* CSPrimMultiBox::GetCoordPS(int index)
{
	if ((index>=0) && (index<(int)vCoords.size())) return &vCoords.at(index);
	return NULL;
}

double* CSPrimMultiBox::GetAllCoords(size_t &Qty, double* array)
{
	Qty=vCoords.size();
	delete[] array;
	array = new double[Qty];
	for (size_t i=0;i<Qty;++i) array[i]=vCoords.at(i).GetValue();
	return array;
}

void CSPrimMultiBox::ClearOverlap()
{
	if (vCoords.size()%6==0) return;  //no work to be done

	vCoords.resize(vCoords.size()-vCoords.size()%6);
}

double* CSPrimMultiBox::GetBoundBox(bool &accurate)
{
	//Update();
	for (unsigned int i=0;i<vCoords.size()/6;++i)
	{
		for (unsigned int n=0;n<3;++n)
		{
			if (vCoords.at(6*i+2*n).GetValue()<=vCoords.at(6*i+2*n+1).GetValue())
			{
				if (i==0)
				{
					dBoundBox[2*n]=vCoords.at(6*i+2*n).GetValue();
					dBoundBox[2*n+1]=vCoords.at(6*i+2*n+1).GetValue();
				}
				else
				{
					if (vCoords.at(6*i+2*n).GetValue()<dBoundBox[2*n]) dBoundBox[2*n]=vCoords.at(6*i+2*n).GetValue();
					if (vCoords.at(6*i+2*n+1).GetValue()>dBoundBox[2*n+1]) dBoundBox[2*n+1]=vCoords.at(6*i+2*n+1).GetValue();
				}

			}
			else
			{
				if (i==0)
				{
					dBoundBox[2*n]=vCoords.at(6*i+2*n+1).GetValue();
					dBoundBox[2*n+1]=vCoords.at(6*i+2*n).GetValue();
				}
				else
				{
					if (vCoords.at(6*i+2*n+1).GetValue()<dBoundBox[2*n]) dBoundBox[2*n]=vCoords.at(6*i+2*n+1).GetValue();
					if (vCoords.at(6*i+2*n).GetValue()>dBoundBox[2*n+1]) dBoundBox[2*n+1]=vCoords.at(6*i+2*n).GetValue();
				}
			}
		}
	}
	accurate=false;
	return dBoundBox;
}

bool CSPrimMultiBox::IsInside(double* Coord, double /*tol*/)
{
	if (Coord==NULL) return false;
	bool in=false;
	double UpVal,DownVal;
	//fprintf(stderr,"here\n");
	for (unsigned int i=0;i<vCoords.size()/6;++i)
	{
		in=true;
		for (unsigned int n=0;n<3;++n)
		{
			//fprintf(stderr,"%e %e %e \n",vCoords.at(6*i+2*n).GetValue(),vCoords.at(6*i+2*n+1).GetValue());
			UpVal=vCoords.at(6*i+2*n+1).GetValue();
			DownVal=vCoords.at(6*i+2*n).GetValue();
			if (DownVal<UpVal)
			{
				if (DownVal>Coord[n]) {in=false;break;}
				if (UpVal<Coord[n]) {in=false;break;}
			}
			else
			{
				if (DownVal<Coord[n]) {in=false;break;}
				if (UpVal>Coord[n]) {in=false;break;}
			}
		}
		if (in==true) {	return true;}
	}
	return false;
}

bool CSPrimMultiBox::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	for (size_t i=0;i<vCoords.size();++i)
	{
		EC=vCoords.at(i).Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=0)  && (ErrStr!=NULL))
		{
			bOK=false;
			stringstream stream;
			stream << endl << "Error in MultiBox (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}
	return bOK;
}

bool CSPrimMultiBox::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimitives::Write2XML(elem,parameterised);
	elem.SetAttribute("QtyBox",(int)vCoords.size()/6);

	for (size_t i=0;i<vCoords.size()/6;++i)
	{
		TiXmlElement SP("StartP");
		WriteTerm(vCoords.at(i*6),SP,"X",parameterised);
		WriteTerm(vCoords.at(i*6+2),SP,"Y",parameterised);
		WriteTerm(vCoords.at(i*6+4),SP,"Z",parameterised);
		elem.InsertEndChild(SP);

		TiXmlElement EP("EndP");
		WriteTerm(vCoords.at(i*6+1),EP,"X",parameterised);
		WriteTerm(vCoords.at(i*6+3),EP,"Y",parameterised);
		WriteTerm(vCoords.at(i*6+5),EP,"Z",parameterised);
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

		if (ReadTerm(vCoords.at(i*6),*SP,"X")==false) return false;
		if (ReadTerm(vCoords.at(i*6+2),*SP,"Y")==false) return false;
		if (ReadTerm(vCoords.at(i*6+4),*SP,"Z")==false) return false;

		if (ReadTerm(vCoords.at(i*6+1),*EP,"X")==false) return false;
		if (ReadTerm(vCoords.at(i*6+3),*EP,"Y")==false) return false;
		if (ReadTerm(vCoords.at(i*6+5),*EP,"Z")==false) return false;

//		for (int n=0;n<6;++n) fprintf(stderr,"%e ",vCoords.at(i*6+n).GetValue());
//		fprintf(stderr,"\n");

		SP=SP->NextSiblingElement("StartP");
		EP=EP->NextSiblingElement("EndP");
		++i;
	};
	return true;
}

/*********************CSPrimSphere********************************************************************/
CSPrimSphere::CSPrimSphere(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=SPHERE;
	for (int i=0;i<3;++i) {psCenter[i].SetParameterSet(paraSet);}
	psRadius.SetParameterSet(paraSet);
	PrimTypeName = string("Sphere");
}

CSPrimSphere::CSPrimSphere(CSPrimSphere* sphere, CSProperties *prop) : CSPrimitives(sphere,prop)
{
	Type=SPHERE;
	for (int i=0;i<3;++i) {psCenter[i]=ParameterScalar(sphere->psCenter[i]);}
	psRadius=ParameterScalar(sphere->psRadius);
	PrimTypeName = string("Sphere");
}

CSPrimSphere::CSPrimSphere(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=SPHERE;
	for (int i=0;i<3;++i) {psCenter[i].SetParameterSet(paraSet);}
	psRadius.SetParameterSet(paraSet);
	PrimTypeName = string("Sphere");
}


CSPrimSphere::~CSPrimSphere()
{
}

double* CSPrimSphere::GetBoundBox(bool &accurate)
{
	for (unsigned int i=0;i<3;++i)
	{
		dBoundBox[2*i]=psCenter[i].GetValue()-psRadius.GetValue();
		dBoundBox[2*i+1]=psCenter[i].GetValue()+psRadius.GetValue();
	}
	accurate=true;
	return dBoundBox;
}

bool CSPrimSphere::IsInside(double* Coord, double /*tol*/)
{
	if (Coord==NULL) return false;
	double dist=sqrt(pow(Coord[0]-psCenter[0].GetValue(),2)+pow(Coord[1]-psCenter[1].GetValue(),2)+pow(Coord[2]-psCenter[2].GetValue(),2));
	if (dist<psRadius.GetValue()) return true;
	return false;
}

bool CSPrimSphere::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	for (int i=0;i<3;++i)
	{
		EC=psCenter[i].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
		{
			bOK=false;
			stringstream stream;
			stream << endl << "Error in " << PrimTypeName << " Center Point (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}

	EC=psRadius.Evaluate();
	if (EC!=ParameterScalar::NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		stringstream stream;
		stream << endl << "Error in " << PrimTypeName << " Radius (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

	return bOK;
}

bool CSPrimSphere::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimitives::Write2XML(elem,parameterised);

	WriteTerm(psRadius,elem,"Radius",parameterised);

	TiXmlElement Center("Center");
	WriteTerm(psCenter[0],Center,"X",parameterised);
	WriteTerm(psCenter[1],Center,"Y",parameterised);
	WriteTerm(psCenter[2],Center,"Z",parameterised);
	elem.InsertEndChild(Center);
	return true;
}

bool CSPrimSphere::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;
	if (ReadTerm(psRadius,*elem,"Radius")==false) return false;

	TiXmlElement* Center=root.FirstChildElement("Center");
	if (Center==NULL) return false;
	if (ReadTerm(psCenter[0],*Center,"X")==false) return false;
	if (ReadTerm(psCenter[1],*Center,"Y")==false) return false;
	if (ReadTerm(psCenter[2],*Center,"Z")==false) return false;

	return true;
}

/*********************CSPrimSphericalShell********************************************************************/
CSPrimSphericalShell::CSPrimSphericalShell(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimSphere(ID,paraSet,prop)
{
	Type=SPHERICALSHELL;
	PrimTypeName = string("SphericalShell");
	psShellWidth.SetParameterSet(paraSet);
}

CSPrimSphericalShell::CSPrimSphericalShell(CSPrimSphericalShell* sphere, CSProperties *prop) : CSPrimSphere(sphere,prop)
{
	Type=SPHERICALSHELL;
	PrimTypeName = string("SphericalShell");
	psShellWidth=ParameterScalar(sphere->psShellWidth);
}

CSPrimSphericalShell::CSPrimSphericalShell(ParameterSet* paraSet, CSProperties* prop) : CSPrimSphere(paraSet,prop)
{
	Type=SPHERICALSHELL;
	PrimTypeName = string("SphericalShell");
	psShellWidth.SetParameterSet(paraSet);
}


CSPrimSphericalShell::~CSPrimSphericalShell()
{
}

double* CSPrimSphericalShell::GetBoundBox(bool &accurate)
{
	for (unsigned int i=0;i<3;++i)
	{
		dBoundBox[2*i]=psCenter[i].GetValue()-psRadius.GetValue()-psShellWidth.GetValue()/2.0;
		dBoundBox[2*i+1]=psCenter[i].GetValue()+psRadius.GetValue()+psShellWidth.GetValue()/2.0;
	}
	accurate=true;
	return dBoundBox;
}

bool CSPrimSphericalShell::IsInside(double* Coord, double /*tol*/)
{
	if (Coord==NULL) return false;
	double dist=sqrt(pow(Coord[0]-psCenter[0].GetValue(),2)+pow(Coord[1]-psCenter[1].GetValue(),2)+pow(Coord[2]-psCenter[2].GetValue(),2));
	if (fabs(dist-psRadius.GetValue())< psShellWidth.GetValue()/2.0) return true;
	return false;
}

bool CSPrimSphericalShell::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=CSPrimSphere::Update(ErrStr);

	EC=psShellWidth.Evaluate();
	if (EC!=ParameterScalar::NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		stringstream stream;
		stream << endl << "Error in " << PrimTypeName << " shell-width (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

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

/*********************CSPrimCylinder********************************************************************/
CSPrimCylinder::CSPrimCylinder(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=CYLINDER;
	for (int i=0;i<6;++i) {psCoords[i].SetParameterSet(paraSet);}
	psRadius.SetParameterSet(paraSet);
	PrimTypeName = string("Cylinder");
}

CSPrimCylinder::CSPrimCylinder(CSPrimCylinder* cylinder, CSProperties *prop) : CSPrimitives(cylinder,prop)
{
	Type=CYLINDER;
	for (int i=0;i<6;++i) {psCoords[i]=ParameterScalar(cylinder->psCoords[i]);}
	psRadius=ParameterScalar(cylinder->psRadius);
	PrimTypeName = string("Cylinder");
}

CSPrimCylinder::CSPrimCylinder(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=CYLINDER;
	for (int i=0;i<6;++i) {psCoords[i].SetParameterSet(paraSet);}
	psRadius.SetParameterSet(paraSet);
	PrimTypeName = string("Cylinder");
}


CSPrimCylinder::~CSPrimCylinder()
{
}

double* CSPrimCylinder::GetBoundBox(bool &accurate)
{
	accurate=false;
	int Direction=0;
	double dCoords[6];
	for (unsigned int i=0;i<6;++i)
		dCoords[i]=psCoords[i].GetValue();
	double rad=psRadius.GetValue();
	for (unsigned int i=0;i<3;++i)
	{
		//vorerst ganz einfach... muss ueberarbeitet werden!!! //todo
		double min=dCoords[2*i];
		double max=dCoords[2*i+1];
		if (min<max)
		{
			dBoundBox[2*i]=min-rad;
			dBoundBox[2*i+1]=max+rad;
		}
		else
		{
			dBoundBox[2*i+1]=min+rad;
			dBoundBox[2*i]=max-rad;
		}
		if (min==max) Direction+=pow(2,i);
	}
	switch (Direction)
	{
	case 3: //orientaion in z-direction
		dBoundBox[4]=dBoundBox[4]+rad;
		dBoundBox[5]=dBoundBox[5]-rad;
		accurate=true;
		break;
	case 5: //orientaion in y-direction
		dBoundBox[2]=dBoundBox[2]+rad;
		dBoundBox[3]=dBoundBox[3]-rad;
		accurate=true;
		break;
	case 6: //orientaion in x-direction
		dBoundBox[1]=dBoundBox[1]+rad;
		dBoundBox[2]=dBoundBox[2]-rad;
		accurate=true;
		break;
	}
	return dBoundBox;
}

bool CSPrimCylinder::IsInside(double* Coord, double /*tol*/)
{
	//Lot-Fuss-Punkt
	//use Point_Line_Distance(...) in the future!!!
	if (Coord==NULL) return false;
	double* p=Coord; //punkt
	double r0[3]={psCoords[0].GetValue(),psCoords[2].GetValue(),psCoords[4].GetValue()}; //aufpunkt
	double r1[3]={psCoords[1].GetValue(),psCoords[3].GetValue(),psCoords[5].GetValue()}; //aufpunkt
	double a[3]={r1[0]-r0[0],r1[1]-r0[1],r1[2]-r0[2]}; //richtungsvektor
	double a2=(a[0]*a[0])+(a[1]*a[1])+(a[2]*a[2]);
	double FP[3];
	double e=0;
	double t=(p[0]-r0[0])*a[0]+(p[1]-r0[1])*a[1]+(p[2]-r0[2])*a[2];
	t/=a2;
	for (int i=0;i<3;++i)
	{
		FP[i]=r0[i]+t*a[i];
		if ((FP[i]<r0[i] || FP[i]>r1[i]) && (a[i]>0)) return false;
		if ((FP[i]>r0[i] || FP[i]<r1[i]) && (a[i]<0)) return false;
		e+=(FP[i]-p[i])*(FP[i]-p[i]);
	}
	double r=psRadius.GetValue();
	if (e<r*r) return true;
	return false;
}

bool CSPrimCylinder::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	for (int i=0;i<6;++i)
	{
		EC=psCoords[i].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
		{
			bOK=false;
			stringstream stream;
			stream << endl << "Error in " << PrimTypeName << " Coord (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}

	EC=psRadius.Evaluate();
	if (EC!=ParameterScalar::NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		stringstream stream;
		stream << endl << "Error in " << PrimTypeName << " Radius (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

	return bOK;
}

bool CSPrimCylinder::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimitives::Write2XML(elem,parameterised);

	WriteTerm(psRadius,elem,"Radius",parameterised);

	TiXmlElement Start("P0");
	WriteTerm(psCoords[0],Start,"X",parameterised);
	WriteTerm(psCoords[2],Start,"Y",parameterised);
	WriteTerm(psCoords[4],Start,"Z",parameterised);
	elem.InsertEndChild(Start);

	TiXmlElement Stop("P1");
	WriteTerm(psCoords[1],Stop,"X",parameterised);
	WriteTerm(psCoords[3],Stop,"Y",parameterised);
	WriteTerm(psCoords[5],Stop,"Z",parameterised);
	elem.InsertEndChild(Stop);
	return true;
}

bool CSPrimCylinder::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;
	if (ReadTerm(psRadius,*elem,"Radius")==false) return false;

	TiXmlElement* Point=root.FirstChildElement("P0");
	if (Point==NULL) return false;
	if (ReadTerm(psCoords[0],*Point,"X")==false) return false;
	if (ReadTerm(psCoords[2],*Point,"Y")==false) return false;
	if (ReadTerm(psCoords[4],*Point,"Z")==false) return false;

	Point=root.FirstChildElement("P1");
	if (Point==NULL) return false;
	if (ReadTerm(psCoords[1],*Point,"X")==false) return false;
	if (ReadTerm(psCoords[3],*Point,"Y")==false) return false;
	if (ReadTerm(psCoords[5],*Point,"Z")==false) return false;

	return true;
}

/*********************CSPrimCylindricalShell********************************************************************/
CSPrimCylindricalShell::CSPrimCylindricalShell(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimCylinder(ID,paraSet,prop)
{
	Type=CYLINDRICALSHELL;
	PrimTypeName = string("CylindricalShell");
	psShellWidth.SetParameterSet(paraSet);
}

CSPrimCylindricalShell::CSPrimCylindricalShell(CSPrimCylindricalShell* cylinder, CSProperties *prop) : CSPrimCylinder(cylinder,prop)
{
	Type=CYLINDRICALSHELL;
	PrimTypeName = string("CylindricalShell");
	psShellWidth=ParameterScalar(cylinder->psShellWidth);
}

CSPrimCylindricalShell::CSPrimCylindricalShell(ParameterSet* paraSet, CSProperties* prop) : CSPrimCylinder(paraSet,prop)
{
	Type=CYLINDRICALSHELL;
	PrimTypeName = string("CylindricalShell");
	psShellWidth.SetParameterSet(paraSet);
}

CSPrimCylindricalShell::~CSPrimCylindricalShell()
{
}

double* CSPrimCylindricalShell::GetBoundBox(bool &accurate)
{
	accurate=false;
	int Direction=0;
	double dCoords[6];
	for (unsigned int i=0;i<6;++i)
		dCoords[i]=psCoords[i].GetValue();
	double rad=psRadius.GetValue()+psShellWidth.GetValue()/2.0;
	for (unsigned int i=0;i<3;++i)
	{
		//vorerst ganz einfach... muss ueberarbeitet werden!!! //todo
		double min=dCoords[2*i];
		double max=dCoords[2*i+1];
		if (min<max)
		{
			dBoundBox[2*i]=min-rad;
			dBoundBox[2*i+1]=max+rad;
		}
		else
		{
			dBoundBox[2*i+1]=min+rad;
			dBoundBox[2*i]=max-rad;
		}
		if (min==max) Direction+=pow(2,i);
	}
	switch (Direction)
	{
	case 3: //orientaion in z-direction
		dBoundBox[4]=dBoundBox[4]+rad;
		dBoundBox[5]=dBoundBox[5]-rad;
		accurate=true;
		break;
	case 5: //orientaion in y-direction
		dBoundBox[2]=dBoundBox[2]+rad;
		dBoundBox[3]=dBoundBox[3]-rad;
		accurate=true;
		break;
	case 6: //orientaion in x-direction
		dBoundBox[1]=dBoundBox[1]+rad;
		dBoundBox[2]=dBoundBox[2]-rad;
		accurate=true;
		break;
	}
	return dBoundBox;
}

bool CSPrimCylindricalShell::IsInside(double* Coord, double /*tol*/)
{
	//Lot-Fuss-Punkt
	if (Coord==NULL) return false;
	double* p=Coord; //punkt
	double r0[3]={psCoords[0].GetValue(),psCoords[2].GetValue(),psCoords[4].GetValue()}; //aufpunkt
	double r1[3]={psCoords[1].GetValue(),psCoords[3].GetValue(),psCoords[5].GetValue()}; //aufpunkt
	double a[3]={r1[0]-r0[0],r1[1]-r0[1],r1[2]-r0[2]}; //richtungsvektor
	double a2=(a[0]*a[0])+(a[1]*a[1])+(a[2]*a[2]);
	double FP[3];
	double e=0;
	double t=(p[0]-r0[0])*a[0]+(p[1]-r0[1])*a[1]+(p[2]-r0[2])*a[2];
	t/=a2;
	for (int i=0;i<3;++i)
	{
		FP[i]=r0[i]+t*a[i];
		if ((FP[i]<r0[i] || FP[i]>r1[i]) && (a[i]>0)) return false;
		if ((FP[i]>r0[i] || FP[i]<r1[i]) && (a[i]<0)) return false;
		e+=(FP[i]-p[i])*(FP[i]-p[i]);
	}
	double r=psRadius.GetValue();
	if (fabs(sqrt(e)-r)<psShellWidth.GetValue()/2.0) return true;
	return false;
}

bool CSPrimCylindricalShell::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=CSPrimCylinder::Update(ErrStr);

	EC=psShellWidth.Evaluate();
	if (EC!=ParameterScalar::NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		stringstream stream;
		stream << endl << "Error in " << PrimTypeName << " shell-width (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

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

/*********************CSPrimPolygon********************************************************************/
CSPrimPolygon::CSPrimPolygon(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=POLYGON;
	NormDir[0].SetParameterSet(paraSet);
	NormDir[1].SetParameterSet(paraSet);
	NormDir[2].SetParameterSet(paraSet);
	Elevation.SetParameterSet(paraSet);
	PrimTypeName = string("Polygon");
}

CSPrimPolygon::CSPrimPolygon(CSPrimPolygon* primPolygon, CSProperties *prop) : CSPrimitives(primPolygon,prop)
{
	Type=POLYGON;
	NormDir[0] = ParameterScalar(primPolygon->NormDir[0]);
	NormDir[1] = ParameterScalar(primPolygon->NormDir[1]);
	NormDir[2] = ParameterScalar(primPolygon->NormDir[2]);
	Elevation = ParameterScalar(primPolygon->Elevation);
	PrimTypeName = string("Polygon");
}

CSPrimPolygon::CSPrimPolygon(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=POLYGON;
	NormDir[0].SetParameterSet(paraSet);
	NormDir[1].SetParameterSet(paraSet);
	NormDir[2].SetParameterSet(paraSet);
	Elevation.SetParameterSet(paraSet);
	PrimTypeName = string("Polygon");
}


CSPrimPolygon::~CSPrimPolygon()
{
}

void CSPrimPolygon::SetCoord(int index, double val)
{
	if ((index>=0) && (index<(int)vCoords.size())) vCoords.at(index).SetValue(val);
}

void CSPrimPolygon::SetCoord(int index, const string val)
{
	if ((index>=0) && (index<(int)vCoords.size())) vCoords.at(index).SetValue(val);
}

void CSPrimPolygon::AddCoord(double val)
{
	vCoords.push_back(ParameterScalar(clParaSet,val));
}

void CSPrimPolygon::AddCoord(const string val)
{
	vCoords.push_back(ParameterScalar(clParaSet,val));
}

void CSPrimPolygon::RemoveCoords(int /*index*/)
{
	//not yet implemented
}

double CSPrimPolygon::GetCoord(int index)
{
	if ((index>=0) && (index<(int)vCoords.size())) return vCoords.at(index).GetValue();
	return 0;
}

ParameterScalar* CSPrimPolygon::GetCoordPS(int index)
{
	if ((index>=0) && (index<(int)vCoords.size())) return &vCoords.at(index);
	return NULL;
}

double* CSPrimPolygon::GetAllCoords(size_t &Qty, double* array)
{
	Qty=vCoords.size();
	delete[] array;
	array = new double[Qty];
	for (size_t i=0;i<Qty;++i) array[i]=vCoords.at(i).GetValue();
	return array;
}


double* CSPrimPolygon::GetBoundBox(bool &accurate)
{
	accurate=false;
	if (vCoords.size()<2) 
	{
		for (int i=0;i<6;++i) dBoundBox[i]=0;
		return dBoundBox;
	}
	double xmin = vCoords.at(0).GetValue(), xmax = vCoords.at(0).GetValue();
	double ymin = vCoords.at(1).GetValue(), ymax = vCoords.at(1).GetValue();
	for (size_t i=1;i<vCoords.size()/2;++i)
	{
		double x = vCoords.at(2*i).GetValue();
		double y = vCoords.at(2*i+1).GetValue();
		if (x<xmin) xmin=x;
		else if (x>xmax) xmax=x;
		if (y<ymin) ymin=y;
		else if (y>ymax) ymax=y;
	}
	//only cartesian so far...
	if (NormDir[0].GetValue()!=0) //assume norm x, yz-plane
	{
		dBoundBox[0] = dBoundBox[1] = Elevation.GetValue();
		dBoundBox[2] = xmin;
		dBoundBox[3] = xmax;
		dBoundBox[4] = ymin;
		dBoundBox[5] = ymax;
	}
	else if (NormDir[1].GetValue()!=0) //assume norm y, zx-plane
	{
		dBoundBox[2] = dBoundBox[3] = Elevation.GetValue();
		dBoundBox[4] = xmin;
		dBoundBox[5] = xmax;
		dBoundBox[0] = ymin;
		dBoundBox[1] = ymax;
	}
	else if (NormDir[2].GetValue()!=0) //assume norm z, xy-plane
	{
		dBoundBox[4] = dBoundBox[5] = Elevation.GetValue();
		dBoundBox[0] = xmin;
		dBoundBox[1] = xmax;
		dBoundBox[2] = ymin;
		dBoundBox[3] = ymax;
	}
	return dBoundBox;
}

bool CSPrimPolygon::IsInside(double* Coord, double /*tol*/)
{
	if (Coord==NULL) return false;
	if (vCoords.size()<2) return false;
		
	bool accBnd=false;
	double* box=this->GetBoundBox(accBnd);

	for (unsigned int n=0;n<3;++n)
	{
		if ((box[2*n]>Coord[n]) || (box[2*n+1]<Coord[n])) return false;
	}
	double x=0,y=0;
	if (NormDir[0].GetValue()!=0) //assume norm x, yz-plane
	{
		x = Coord[1];
		y = Coord[2];
	}
	else if (NormDir[1].GetValue()!=0) //assume norm y, zx-plane
	{
		x = Coord[2];
		y = Coord[0];
	}
	else if (NormDir[2].GetValue()!=0) //assume norm z, xy-plane
	{
		x = Coord[0];
		y = Coord[1];
	}
	
	int wn = 0;

	size_t np = vCoords.size()/2;
	double x1 = vCoords[2*np-2].GetValue();
	double y1 = vCoords[2*np-1].GetValue();
	double x2 = vCoords[0].GetValue();
	double y2 = vCoords[1].GetValue();
	bool startover = y1 >= y ? true : false;
	bool endover;
	
	for (size_t i=0;i<np;++i)
	{
		x2 = vCoords[2*i].GetValue();
		y2 = vCoords[2*i+1].GetValue();
		endover = y2 >= y ? true : false;
		if (startover != endover) 
		{
			if ((y2 - y)*(x2 - x1) <= (y2 - y1)*(x2 - x)) 
			{
				if (endover) wn ++;
			}
			else
			{
				if (!endover) wn --;
			}
		}
		startover = endover;
		y1 = y2;
		x1 = x2;
	}
	return wn != 0;
}


bool CSPrimPolygon::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	for (size_t i=1;i<vCoords.size();++i)
	{
		EC=vCoords[i].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
		{
			bOK=false;
			stringstream stream;
			stream << endl << "Error in Polygon (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}
	
	EC=Elevation.Evaluate();
	if (EC!=ParameterScalar::NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		stringstream stream;
		stream << endl << "Error in Polygib Elevation (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}
	
	for (size_t i=1;i<3;++i)
	{
		EC=NormDir[i].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
		{
			bOK=false;
			stringstream stream;
			stream << endl << "Error in Polygon Normal Direction (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}
	return bOK;
}

bool CSPrimPolygon::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimitives::Write2XML(elem,parameterised);
	
	WriteTerm(Elevation,elem,"Elevation",parameterised);

	elem.SetAttribute("QtyVertices",(int)vCoords.size()/2);
	TiXmlElement NV("NormDir");
	WriteTerm(NormDir[0],NV,"X",parameterised);	
	WriteTerm(NormDir[1],NV,"Y",parameterised);	
	WriteTerm(NormDir[2],NV,"Z",parameterised);	
	elem.InsertEndChild(NV);

	for (size_t i=0;i<vCoords.size()/2;++i)
	{
		TiXmlElement VT("Vertex");
		WriteTerm(vCoords.at(i*2),VT,"X1",parameterised);
		WriteTerm(vCoords.at(i*2+1),VT,"X2",parameterised);
		elem.InsertEndChild(VT);
	}
	return true;
}

bool CSPrimPolygon::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;
	if (ReadTerm(Elevation,*elem,"Elevation")==false) return false;

	TiXmlElement* NV=root.FirstChildElement("NormDir");
	if (NV==NULL) return false;
	if (ReadTerm(NormDir[0],*NV,"X")==false) return false;
	if (ReadTerm(NormDir[1],*NV,"Y")==false) return false;
	if (ReadTerm(NormDir[2],*NV,"Z")==false) return false;
	
	TiXmlElement *VT=root.FirstChildElement("Vertex");
	if (vCoords.size()!=0) return false;
	int i=0;
	while (VT)
	{
		for (int n=0;n<2;++n) this->AddCoord(0.0);

		if (ReadTerm(vCoords.at(i*2),*VT,"X1")==false) return false;
		if (ReadTerm(vCoords.at(i*2+1),*VT,"X2")==false) return false;

		VT=VT->NextSiblingElement("Vertex");
		++i;
	};
	
	return true;
}

/*********************CSPrimLinPoly********************************************************************/
CSPrimLinPoly::CSPrimLinPoly(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimPolygon(ID,paraSet,prop)
{
	Type=LINPOLY;
	extrudeLength.SetParameterSet(paraSet);
	PrimTypeName = string("Lin-Poly");
}

CSPrimLinPoly::CSPrimLinPoly(CSPrimLinPoly* primLinPoly, CSProperties *prop) : CSPrimPolygon(primLinPoly,prop)
{
	Type=LINPOLY;
	extrudeLength = ParameterScalar(primLinPoly->extrudeLength);
	PrimTypeName = string("Lin-Poly");
}

CSPrimLinPoly::CSPrimLinPoly(ParameterSet* paraSet, CSProperties* prop) : CSPrimPolygon(paraSet,prop)
{
	Type=LINPOLY;
	extrudeLength.SetParameterSet(paraSet);
	PrimTypeName = string("Lin-Poly");
}


CSPrimLinPoly::~CSPrimLinPoly()
{
}

double* CSPrimLinPoly::GetBoundBox(bool &accurate)
{
	CSPrimPolygon::GetBoundBox(accurate);
	
	double len = extrudeLength.GetValue();
	
	if (NormDir[0].GetValue()!=0) //assume norm x, yz-plane
	{
		if (len>0)
		{
			dBoundBox[0] = Elevation.GetValue();
			dBoundBox[1] = dBoundBox[0] + len;
		}
		else
		{
			dBoundBox[1] = Elevation.GetValue();
			dBoundBox[0] = dBoundBox[1] + len;			
		}
	}
	else if (NormDir[1].GetValue()!=0) //assume norm y, zx-plane
	{
		if (len>0)
		{
			dBoundBox[2] = Elevation.GetValue();
			dBoundBox[3] = dBoundBox[2] + len;
		}
		else
		{
			dBoundBox[3] = Elevation.GetValue();
			dBoundBox[2] = dBoundBox[3] + len;			
		}
	}
	else if (NormDir[2].GetValue()!=0) //assume norm z, xy-plane
	{
		if (len>0)
		{
			dBoundBox[4] = Elevation.GetValue();
			dBoundBox[5] = dBoundBox[4] + len;
		}
		else
		{
			dBoundBox[5] = Elevation.GetValue();
			dBoundBox[4] = dBoundBox[5] + len;			
		}
	}
	
	return dBoundBox;
}

bool CSPrimLinPoly::IsInside(double* Coord, double tol)
{
	if (Coord==NULL) return false;	
	return CSPrimPolygon::IsInside(Coord, tol);
}


bool CSPrimLinPoly::Update(string *ErrStr)
{
	int EC=0;

	bool bOK = CSPrimPolygon::Update(ErrStr);
	
	EC=extrudeLength.Evaluate();
	if (EC!=ParameterScalar::NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		stringstream stream;
		stream << endl << "Error in Polygib Elevation (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}
	
	return bOK;
}

bool CSPrimLinPoly::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimPolygon::Write2XML(elem,parameterised);

	WriteTerm(extrudeLength,elem,"Length",parameterised);
	return true;
}

bool CSPrimLinPoly::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimPolygon::ReadFromXML(root)==false) return false;

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;
	if (ReadTerm(extrudeLength,*elem,"Length")==false) return false;

	return true;
}

/*********************CSPrimLinPoly********************************************************************/
CSPrimRotPoly::CSPrimRotPoly(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimPolygon(ID,paraSet,prop)
{
	Type=LINPOLY;
//	for (int i=0;i<6;++i) {psCoords[i].SetParameterSet(paraSet);}
	PrimTypeName = string("Rot-Poly");
}

CSPrimRotPoly::CSPrimRotPoly(CSPrimRotPoly* primRotPoly, CSProperties *prop) : CSPrimPolygon(primRotPoly,prop)
{
	Type=LINPOLY;
//	for (int i=0;i<6;++i) {psCoords[i]=ParameterScalar(primBox->psCoords[i]);}
	PrimTypeName = string("Rot-Poly");
}

CSPrimRotPoly::CSPrimRotPoly(ParameterSet* paraSet, CSProperties* prop) : CSPrimPolygon(paraSet,prop)
{
	Type=LINPOLY;
//	for (int i=0;i<6;++i) {psCoords[i].SetParameterSet(paraSet);}
	PrimTypeName = string("Rot-Poly");
}


CSPrimRotPoly::~CSPrimRotPoly()
{
}

double* CSPrimRotPoly::GetBoundBox(bool &accurate)
{
	accurate = false;
//	for (int i=0;i<6;++i) dBoundBox[i]=psCoords[i].GetValue();
//	for (int i=0;i<3;++i)
//		if (dBoundBox[2*i]>dBoundBox[2*i+1])
//		{
//			double help=dBoundBox[2*i];
//			dBoundBox[2*i]=dBoundBox[2*i+1];
//			dBoundBox[2*i+1]=help;
//		}
	return dBoundBox;
}

bool CSPrimRotPoly::IsInside(double* Coord, double /*tol*/)
{
	if (Coord==NULL) return false;

	bool accBnd=false;
	double* box=this->GetBoundBox(accBnd);

	for (unsigned int n=0;n<3;++n)
	{
		if ((box[2*n]>Coord[n]) || (box[2*n+1]<Coord[n])) return false;
	}
	//more checking needed!!
	return true;
}


bool CSPrimRotPoly::Update(string *ErrStr)
{
	int EC=0;
	bool bOK = CSPrimPolygon::Update(ErrStr);

	for (size_t i=1;i<3;++i)
	{
		EC=RotAxis[i].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
		{
			bOK=false;
			stringstream stream;
			stream << endl << "Error in Rot - Polygon Axis (ID: " << uiID << "): ";
			ErrStr->append(stream.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}
	
	EC=StartStopAngle[0].Evaluate();
	if (EC!=ParameterScalar::NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		stringstream stream;
		stream << endl << "Error in Polygib Start Angle (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}
	
	EC=StartStopAngle[1].Evaluate();
	if (EC!=ParameterScalar::NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		stringstream stream;
		stream << endl << "Error in Polygib Stop Angle (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}
	
	return bOK;
}

bool CSPrimRotPoly::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimPolygon::Write2XML(elem,parameterised);

	TiXmlElement RT("RotAxis");
	WriteTerm(RotAxis[0],RT,"X",parameterised);	
	WriteTerm(RotAxis[1],RT,"Y",parameterised);	
	WriteTerm(RotAxis[2],RT,"Z",parameterised);	
	elem.InsertEndChild(RT);
	
	TiXmlElement Ang("Angles");
	WriteTerm(StartStopAngle[0],Ang,"Start",parameterised);	
	WriteTerm(StartStopAngle[1],Ang,"Stop",parameterised);	
	elem.InsertEndChild(Ang);
	return true;
}

bool CSPrimRotPoly::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimPolygon::ReadFromXML(root)==false) return false;

	TiXmlElement* NV=root.FirstChildElement("RotAxis");
	if (NV==NULL) return false;
	if (ReadTerm(NormDir[0],*NV,"X")==false) return false;
	if (ReadTerm(NormDir[1],*NV,"Y")==false) return false;
	if (ReadTerm(NormDir[2],*NV,"Z")==false) return false;
	
	NV=root.FirstChildElement("Angles");
	if (NV==NULL) return false;
	if (ReadTerm(StartStopAngle[0],*NV,"Start")==false) return false;
	if (ReadTerm(StartStopAngle[1],*NV,"Stop")==false) return false;

	return true;
}

/*********************CSPrimCurve********************************************************************/
CSPrimCurve::CSPrimCurve(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=CURVE;
	PrimTypeName = string("Curve");
}

CSPrimCurve::CSPrimCurve(CSPrimCurve* primCurve, CSProperties *prop) : CSPrimitives(primCurve,prop)
{
	Type=CURVE;
	for (size_t i=0;i<primCurve->points[0].size();++i)
	{
		points[0].push_back(ParameterScalar(primCurve->points[0].at(i)));
		points[1].push_back(ParameterScalar(primCurve->points[1].at(i)));
		points[2].push_back(ParameterScalar(primCurve->points[2].at(i)));
	}
	PrimTypeName = string("Curve");
}

CSPrimCurve::CSPrimCurve(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=CURVE;
	PrimTypeName = string("Curve");
}


CSPrimCurve::~CSPrimCurve()
{
	points[0].clear();
	points[1].clear();
	points[2].clear();
}

size_t CSPrimCurve::AddPoint(double coords[])
{
	points[0].push_back(ParameterScalar(clParaSet,coords[0]));
	points[1].push_back(ParameterScalar(clParaSet,coords[1]));
	points[2].push_back(ParameterScalar(clParaSet,coords[2]));
	return points[0].size()-1;
}

void CSPrimCurve::SetCoord(size_t point_index, int nu, double val)
{
	if (point_index>=GetNumberOfPoints()) return;
	if ((nu<0) || (nu>2)) return;
	points[nu].at(point_index).SetValue(val);
}

void CSPrimCurve::SetCoord(size_t point_index, int nu, string val)
{
	if (point_index>=GetNumberOfPoints()) return;
	if ((nu<0) || (nu>2)) return;
	points[nu].at(point_index).SetValue(val);
}

bool CSPrimCurve::GetPoint(size_t point_index, double* point)
{
	if (point_index>=GetNumberOfPoints()) return false;
	point[0] = points[0].at(point_index).GetValue();
	point[1] = points[1].at(point_index).GetValue();
	point[2] = points[2].at(point_index).GetValue();
	return true;
}

double* CSPrimCurve::GetBoundBox(bool &accurate, bool /*PreserveOrientation*/)
{
	accurate=false;
	for (size_t i=0;i<points[0].size();++i)
	{
		for (int n=0;n<3;++n)
		{
			if (points[n].at(i).GetValue()<dBoundBox[n])
				dBoundBox[2*n]=points[n].at(i).GetValue();
			else if (points[n].at(i).GetValue()>dBoundBox[2*n+1])
				dBoundBox[2*n+1]=points[n].at(i).GetValue();
		}
	}
	return dBoundBox;
}

bool CSPrimCurve::IsInside(double* /*Coord*/, double /*tol*/)
{
	//this is a 1D-object, you can never be inside...
	return false;
}


bool CSPrimCurve::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	for (size_t i=0;i<GetNumberOfPoints();++i)
	{
		for (int n=0;n<3;++n)
		{
			EC=points[n][i].Evaluate();
			if (EC!=ParameterScalar::NO_ERROR) bOK=false;
			if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
			{
				bOK=false;
				stringstream stream;
				stream << endl << "Error in " << PrimTypeName << " (ID: " << uiID << "): ";
				ErrStr->append(stream.str());
				PSErrorCode2Msg(EC,ErrStr);
			}
		}
	}
	return bOK;
}

bool CSPrimCurve::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimitives::Write2XML(elem,parameterised);

	for (size_t i=0;i<points[0].size();++i)
	{
		TiXmlElement VT("Vertex");
		WriteTerm(points[0].at(i),VT,"X",parameterised);
		WriteTerm(points[1].at(i),VT,"Y",parameterised);
		WriteTerm(points[2].at(i),VT,"Z",parameterised);
		elem.InsertEndChild(VT);
	}
	return true;
}

bool CSPrimCurve::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;

	TiXmlElement *VT=root.FirstChildElement("Vertex");
	if (points[0].size()!=0) return false;
	double emptyP[] = {0.0,0.0,0.0};
	size_t current=0;
	while (VT)
	{
		current = this->AddPoint(emptyP);

		if (ReadTerm(points[0].at(current),*VT,"X")==false) return false;
		if (ReadTerm(points[1].at(current),*VT,"Y")==false) return false;
		if (ReadTerm(points[2].at(current),*VT,"Z")==false) return false;
		VT=VT->NextSiblingElement("Vertex");
	};
	
	return true;
}

/*********************CSPrimWire********************************************************************/
CSPrimWire::CSPrimWire(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimCurve(ID,paraSet,prop)
{
	Type=WIRE;
	PrimTypeName = string("Wire");
	wireRadius.SetParameterSet(paraSet);
}

CSPrimWire::CSPrimWire(CSPrimWire* primCurve, CSProperties *prop) : CSPrimCurve(primCurve,prop)
{
	Type=WIRE;
	PrimTypeName = string("Wire");
	wireRadius = ParameterScalar(primCurve->wireRadius);
}

CSPrimWire::CSPrimWire(ParameterSet* paraSet, CSProperties* prop) : CSPrimCurve(paraSet,prop)
{
	Type=WIRE;
	PrimTypeName = string("Wire");
	wireRadius.SetParameterSet(paraSet);
}


CSPrimWire::~CSPrimWire()
{
}


double* CSPrimWire::GetBoundBox(bool &accurate, bool PreserveOrientation)
{
	CSPrimCurve::GetBoundBox(accurate,PreserveOrientation);
	return dBoundBox;
}

bool CSPrimWire::IsInside(double* Coord, double /*tol*/)
{
	if (Coord==NULL) return false;
	double rad = wireRadius.GetValue();
	double p0[3];
	double p1[3];
	double foot,dist,distPP;
	for (size_t i=0;i<GetNumberOfPoints();++i)
	{
		p0[0]=points[0].at(i).GetValue();
		p0[1]=points[1].at(i).GetValue();
		p0[2]=points[2].at(i).GetValue();
		dist = sqrt(pow(Coord[0]-p0[0],2)+pow(Coord[1]-p0[1],2)+pow(Coord[2]-p0[2],2));
		if (dist<rad) return true;

		if (i<GetNumberOfPoints()-1)
		{
			p1[0]=points[0].at(i+1).GetValue();
			p1[1]=points[1].at(i+1).GetValue();
			p1[2]=points[2].at(i+1).GetValue();
			distPP = sqrt(pow(p1[0]-p0[0],2)+pow(p1[1]-p0[1],2)+pow(p1[2]-p0[2],2))+rad;
			if (dist<distPP)
			{
				Point_Line_Distance(Coord ,p0 ,p1 ,foot ,dist);
				if ((foot>0) && (foot<1) && (dist<rad))
					return true;
			}
		}
	}

	return false;
}

bool CSPrimWire::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	CSPrimCurve::Update(ErrStr);

	EC=wireRadius.Evaluate();
	if (EC!=ParameterScalar::NO_ERROR) bOK=false;
	if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
	{
		bOK=false;
		stringstream stream;
		stream << endl << "Error in " << PrimTypeName << " (ID: " << uiID << "): ";
		ErrStr->append(stream.str());
		PSErrorCode2Msg(EC,ErrStr);
	}

	return bOK;
}

bool CSPrimWire::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimCurve::Write2XML(elem,parameterised);

	WriteTerm(wireRadius,elem,"WireRadius",parameterised);
	return true;
}

bool CSPrimWire::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimCurve::ReadFromXML(root)==false) return false;

	TiXmlElement *elem = root.ToElement();
	if (elem==NULL) return false;
	if (ReadTerm(wireRadius,*elem,"WireRadius")==false) return false;
	return true;
}

/*********************CSPrimUserDefined********************************************************************/
CSPrimUserDefined::CSPrimUserDefined(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop)
{
	Type=USERDEFINED;
	fParse = new CSFunctionParser();
//	fParse->AddConstant("pi", 3.1415926535897932);
	stFunction = string();
	CoordSystem=CARESIAN_SYSTEM;
	for (int i=0;i<3;++i) {dPosShift[i].SetParameterSet(paraSet);}
	PrimTypeName = string("User-Defined");
}

CSPrimUserDefined::CSPrimUserDefined(CSPrimUserDefined* primUDef, CSProperties *prop) : CSPrimitives(primUDef,prop)
{
	Type=USERDEFINED;
	fParse = new CSFunctionParser(*primUDef->fParse);
//	fParse = primUDef->fParse;
	stFunction = string(primUDef->stFunction);
	CoordSystem = primUDef->CoordSystem;
	for (int i=0;i<3;++i) {dPosShift[i]=ParameterScalar(primUDef->dPosShift[i]);}
	PrimTypeName = string("User-Defined");
}

CSPrimUserDefined::CSPrimUserDefined(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop)
{
	Type=USERDEFINED;
	fParse = new CSFunctionParser();
//	fParse->AddConstant("pi", 3.1415926535897932);
	stFunction = string();
	CoordSystem=CARESIAN_SYSTEM;
	for (int i=0;i<3;++i) {dPosShift[i].SetParameterSet(paraSet);}
	PrimTypeName = string("User-Defined");
}


CSPrimUserDefined::~CSPrimUserDefined()
{
	delete fParse;fParse=NULL;
}

void CSPrimUserDefined::SetCoordSystem(UserDefinedCoordSystem newSystem)
{
	CoordSystem=newSystem;
}

void CSPrimUserDefined::SetFunction(const char* func)
{
	if (func==NULL) return;
	stFunction = string(func);
}

double* CSPrimUserDefined::GetBoundBox(bool &accurate)
{
	//this type has no simple bound box
	double max=std::numeric_limits<double>::max();
	dBoundBox[0]=dBoundBox[2]=dBoundBox[4]=-max;
	dBoundBox[1]=dBoundBox[3]=dBoundBox[5]=max;
	accurate=false;
	return dBoundBox;
}

bool CSPrimUserDefined::IsInside(double* Coord, double /*tol*/)
{
	if (Coord==NULL) return false;

	int NrPara=clParaSet->GetQtyParameter();
	if (NrPara!=iQtyParameter) return false;
	double *vars = new double[NrPara+6];

	vars=clParaSet->GetValueArray(vars);

	double x=Coord[0]-dPosShift[0].GetValue();
	double y=Coord[1]-dPosShift[1].GetValue();
	double z=Coord[2]-dPosShift[2].GetValue();
	double rxy=sqrt(x*x+y*y);
	vars[NrPara]=x;
	vars[NrPara+1]=y;
	vars[NrPara+2]=z;

	switch (CoordSystem)
	{
	case CARESIAN_SYSTEM:  //uses x,y,z
		vars[NrPara+3]=0;
		vars[NrPara+4]=0;
		vars[NrPara+5]=0;		break;
	case CYLINDER_SYSTEM: //uses x,y,z,r,a,0
		vars[NrPara+3]=rxy;
		vars[NrPara+4]=atan2(y,x);
		vars[NrPara+5]=0;
		break;
	case SPHERE_SYSTEM:   //uses x,y,z,r,a,t
		vars[NrPara+3]=sqrt(x*x+y*y+z*z);
		vars[NrPara+4]=atan2(y,x);
		vars[NrPara+5]=asin(1)-atan(z/rxy);
		//cout << "x::" << x << "y::" << y << "z::" << z << "r::" << vars[NrPara] << "a::" << vars[NrPara+1] << "t::" << vars[NrPara+2] << endl;
		break;
	default:
		//unknown System
		return false;
		break;
	}
	double dValue=0;

	if (fParse->GetParseErrorType()==FunctionParser::FP_NO_ERROR) dValue=fParse->Eval(vars);
	else dValue=0;
	delete[] vars;vars=NULL;

	return dValue==1;
}


bool CSPrimUserDefined::Update(string *ErrStr)
{
	int EC=0;
	bool bOK=true;
	string vars;
	switch (CoordSystem)
	{
	case CARESIAN_SYSTEM:
		vars=string("x,y,z");
		break;
	case CYLINDER_SYSTEM:
		vars=string("x,y,z,r,a");
		break;
	case SPHERE_SYSTEM:
		vars=string("x,y,z,r,a,t");
		break;
	default:
		//unknown System
		return false;
		break;
	}
	iQtyParameter=clParaSet->GetQtyParameter();
	if (iQtyParameter>0)
	{
		fParameter=string(clParaSet->GetParameterString());
		vars = fParameter + "," + vars;
	}

	fParse->Parse(stFunction,vars);

	EC=fParse->GetParseErrorType();
	//cout << fParse.ErrorMsg();

	if (EC!=FunctionParser::FP_NO_ERROR) bOK=false;

	if ((EC!=FunctionParser::FP_NO_ERROR)  && (ErrStr!=NULL))
	{
		ostringstream oss;
		oss << "\nError in User Defined Primitive Function (ID: " << uiID << "): " << fParse->ErrorMsg();
		ErrStr->append(oss.str());
		bOK=false;
	}

	for (int i=0;i<3;++i)
	{
		EC=dPosShift[i].Evaluate();
		if (EC!=ParameterScalar::NO_ERROR) bOK=false;
		if ((EC!=ParameterScalar::NO_ERROR)  && (ErrStr!=NULL))
		{
			bOK=false;
			ostringstream oss;
			oss << "\nError in User Defined Primitive Coord (ID: " << uiID << "): ";
			ErrStr->append(oss.str());
			PSErrorCode2Msg(EC,ErrStr);
		}
	}
	return bOK;
}

bool CSPrimUserDefined::Write2XML(TiXmlElement &elem, bool parameterised)
{
	CSPrimitives::Write2XML(elem,parameterised);

	elem.SetAttribute("CoordSystem",CoordSystem);

	TiXmlElement P1("CoordShift");
	WriteTerm(dPosShift[0],P1,"X",parameterised);
	WriteTerm(dPosShift[1],P1,"Y",parameterised);
	WriteTerm(dPosShift[2],P1,"Z",parameterised);
	elem.InsertEndChild(P1);

	TiXmlElement FuncElem("Function");
	TiXmlText FuncText(GetFunction());
	FuncElem.InsertEndChild(FuncText);

	elem.InsertEndChild(FuncElem);
	return true;
}

bool CSPrimUserDefined::ReadFromXML(TiXmlNode &root)
{
	if (CSPrimitives::ReadFromXML(root)==false) return false;

	int value;
	TiXmlElement* elem=root.ToElement();
	if (elem==NULL) return false;
    if (elem->QueryIntAttribute("CoordSystem",&value)!=TIXML_SUCCESS) return false;
	SetCoordSystem((UserDefinedCoordSystem)value);

	//P1
	TiXmlElement* P1=root.FirstChildElement("CoordShift");
	if (P1==NULL) return false;
	if (ReadTerm(dPosShift[0],*P1,"X")==false) return false;
	if (ReadTerm(dPosShift[1],*P1,"Y")==false) return false;
	if (ReadTerm(dPosShift[2],*P1,"Z")==false) return false;

	TiXmlElement* FuncElem=root.FirstChildElement("Function");
	if (FuncElem==NULL) return false;
	SetFunction(FuncElem->GetText());

	return true;
}

