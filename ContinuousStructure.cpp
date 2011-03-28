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

#include "ContinuousStructure.h"
#include "tinyxml.h"

/*********************ContinuousStructure********************************************************************/
ContinuousStructure::ContinuousStructure(void)
{
	clParaSet = new ParameterSet();
	//init datastructures...
	clear();
}

ContinuousStructure::~ContinuousStructure(void)
{
	clear();
	delete clParaSet;clParaSet=NULL;
//	for (unsigned int n=0;n<vPrimitives.size();++n)
//	{
//			delete vPrimitives.at(n);vPrimitives.at(n)=NULL;
//	}
//	vPrimitives.clear();
//	for (unsigned int n=0;n<vProperties.size();++n)
//	{
//			delete vProperties.at(n);vProperties.at(n)=NULL;
//	}
//	vProperties.clear();
}

void ContinuousStructure::AddPrimitive(CSPrimitives* prim)
{
	if (prim==NULL) return;
	vPrimitives.push_back(prim);
	prim->SetCoordInputType(m_MeshType, false);
	prim->Update(&ErrString);
	if (prim->GetID()<=maxID) prim->SetID(maxID++);
	else maxID=prim->GetID()+1;
}

void ContinuousStructure::AddProperty(CSProperties* prop)
{
	if (prop==NULL) return;
	prop->SetCoordInputType(m_MeshType);
	prop->Update(&ErrString);
	//unsigned int ID=prop->GetID();
	//for (size_t i=0;i<vProperties.size();++i) if (vProperties.at(i)->GetID()==ID) return false;
	vProperties.push_back(prop);
	prop->SetUniqueID(UniqueIDCounter++);
	this->UpdateIDs();
}

bool ContinuousStructure::ReplaceProperty(CSProperties* oldProp, CSProperties* newProp)
{
	vector<CSProperties*>::iterator iter;
	for (iter=vProperties.begin();iter<vProperties.end();++iter)
	{
		if (*iter==oldProp)
		{
			CSPrimitives* prim=oldProp->GetPrimitive(0);
			while (prim!=NULL)
			{
				newProp->AddPrimitive(prim);
				prim->SetProperty(newProp);
				prim=oldProp->GetPrimitive(0);
			}
			delete *iter;
			*iter=newProp;
			newProp->SetUniqueID(UniqueIDCounter++);
			return true;
		}
	}
	return false;
}


void ContinuousStructure::DeletePrimitive(size_t index)
{
	if (index>=vPrimitives.size()) return;
	vector<CSPrimitives*>::iterator iter=vPrimitives.begin();
	delete vPrimitives.at(index);
	vPrimitives.erase(iter+index);
}

void ContinuousStructure::DeletePrimitive(CSPrimitives* prim)
{
	vector<CSPrimitives*>::iterator iter;
	for (iter=vPrimitives.begin();iter<vPrimitives.end();++iter)
	{
		if (*iter==prim)
		{
			delete *iter;
			vPrimitives.erase(iter);
		}
	}
}

void ContinuousStructure::DeleteProperty(size_t index)
{
	if (index>=vProperties.size()) return;
	vector<CSProperties*>::iterator iter=vProperties.begin();
	delete vProperties.at(index);
	vProperties.erase(iter+index);
	this->UpdateIDs();
}

void ContinuousStructure::DeleteProperty(CSProperties* prop)
{
	vector<CSProperties*>::iterator iter;
	for (iter=vProperties.begin();iter<vProperties.end();++iter)
	{
		if (*iter==prop)
		{
			delete *iter;
			vProperties.erase(iter);
		}
	}
	this->UpdateIDs();
}

int ContinuousStructure::GetIndex(CSPrimitives* prim)
{
	if (prim==NULL) return -1;
	for (size_t i=0;i<vPrimitives.size();++i)
		if (vPrimitives.at(i)==prim) return (int)i;
	return -1;
}

int ContinuousStructure::GetIndex(CSProperties* prop)
{
	if (prop==NULL) return -1;
	for (size_t i=0;i<vProperties.size();++i)
		if (vProperties.at(i)==prop) return (int)i;
	return -1;
}

size_t ContinuousStructure::GetQtyPropertyType(CSProperties::PropertyType type)
{
	size_t count=0;
	for (size_t i=0;i<vProperties.size();++i) if (vProperties.at(i)->GetType() & type) ++count;
	return count;
}

vector<CSProperties*>  ContinuousStructure::GetPropertyByType(CSProperties::PropertyType type)
{
	vector<CSProperties*> found;
	for (size_t i=0;i<vProperties.size();++i)
		if (vProperties.at(i)->GetType() & type)
			found.push_back(vProperties.at(i));
	return found;
}

bool ContinuousStructure::InsertEdges2Grid(int nu)
{
	if (nu<0) return false;
	if (nu>2) return false;
	double box[6] = {0,0,0,0,0,0};
	bool accBound=false;
	for (size_t i=0;i<vPrimitives.size();++i)
	{
		accBound = vPrimitives.at(i)->GetBoundBox(box);
		if (accBound)
		{
			clGrid.AddDiscLine(nu,box[2*nu]);
			clGrid.AddDiscLine(nu,box[2*nu+1]);
		}
	}
	clGrid.Sort(nu);
	return true;
}

CSPrimitives* ContinuousStructure::GetPrimitiveByID(unsigned int ID)
{
	for (size_t i=0;i<vPrimitives.size();++i) if (vPrimitives.at(i)->GetID()==ID) return vPrimitives.at(i);
	return NULL;
}

CSProperties* ContinuousStructure::GetProperty(size_t index)
{
	if (index<vProperties.size()) return vProperties.at(index);
	return NULL;
}

CSPrimitives* ContinuousStructure::GetPrimitive(size_t index)
{
	if (index<vPrimitives.size()) return vPrimitives.at(index);
	return NULL;
}

CSProperties* ContinuousStructure::GetPropertyByCoordPriority(const double* coord, CSProperties::PropertyType type, bool markFoundAsUsed)
{
	CSProperties* winProp=NULL;
	CSPrimitives* winPrim=NULL;
	CSPrimitives* locPrim=NULL;
	int winPrio=0;
	int locPrio=0;
	for (size_t i=0;i<vProperties.size();++i)
	{
		if ((type==CSProperties::ANY) || (vProperties.at(i)->GetType() & type))
		{
			locPrim = vProperties.at(i)->CheckCoordInPrimitive(coord,locPrio,dDrawingTol);
			if (locPrim)
			{
				if (winProp==NULL)
				{
					winPrio=locPrio;
					winProp=vProperties.at(i);
					winPrim=locPrim;
				}
				else if (locPrio>winPrio)
				{
					winPrio=locPrio;
					winProp=vProperties.at(i);
					winPrim=locPrim;
				}
			}
		}
	}
	if ((markFoundAsUsed) && (winPrim))
		winPrim->SetPrimitiveUsed(true);
	return winProp;
}


CSProperties** ContinuousStructure::GetPropertiesByCoordsPriority(const double* /*coords*/, CSProperties::PropertyType /*type*/, bool /*markFoundAsUsed*/)
{
	cerr << "ContinuousStructure::GetPropertiesByCoordsPriority --> This methode has not been implemented yet!!! return NULL" << endl;
	return NULL;
}

void ContinuousStructure::WarnUnusedPrimitves(ostream& stream, CSProperties::PropertyType type)
{
	for (size_t i=0;i<vProperties.size();++i)
	{
		if ((type==CSProperties::ANY) || (vProperties.at(i)->GetType() & type))
		{
			vProperties.at(i)->WarnUnusedPrimitves(stream);
		}
	}
}

void ContinuousStructure::ShowPropertyStatus(ostream& stream, CSProperties::PropertyType type)
{
	for (size_t i=0;i<vProperties.size();++i)
	{
		if ((type==CSProperties::ANY) || (vProperties.at(i)->GetType() & type))
		{
			stream << "-----------------------------------------" << endl;
			vProperties.at(i)->ShowPropertyStatus(stream);
		}
	}
}

void ContinuousStructure::SetCoordInputType(CoordinateSystem type)
{
	m_MeshType = type;
	for (size_t i=0;i<vProperties.size();++i)
	{
		vProperties.at(i)->SetCoordInputType(type);
	}
}

bool ContinuousStructure::isGeometryValid()
{
	if (GetQtyProperties()<=0) return false;
	if (GetQtyPrimitives()<=0) return false;
	if (clGrid.GetQtyLines(0)<=1) return false;
	if (clGrid.GetQtyLines(1)<=1) return false;
	if (clGrid.GetQtyLines(2)<=0) return false;

	for (size_t i=0;i<vPrimitives.size();++i)
	{
		if (vPrimitives.at(i)->Update()==false) return false;
	}

	int excit=0;
	for (size_t i=0;i<vProperties.size();++i)
	{
		if (vProperties.at(i)->Update()==false) return false;
		if (vProperties.at(i)->GetType()==CSProperties::ELECTRODE)
		{
			if (vProperties.at(i)->GetQtyPrimitives()>0) ++excit;
		}
	}
	if (excit==0) return false;
	return true;
}

double* ContinuousStructure::GetObjectArea()
{
	CSPrimitives* prim=NULL;
	bool AccBound;
	for (size_t i=0;i<GetQtyPrimitives();++i)
	{
		prim=GetPrimitive(i);
		double box[6] = {0,0,0,0,0,0};
		AccBound = prim->GetBoundBox(box);
		if (box!=NULL && AccBound)
		{
			if (i==0) for (int i=0;i<6;++i) ObjArea[i]=box[i];
			else
			{
				for (int i=0;i<3;++i)
				{
					if (ObjArea[2*i]>box[2*i]) ObjArea[2*i]=box[2*i];
					if (ObjArea[2*i+1]<box[2*i+1]) ObjArea[2*i+1]=box[2*i+1];
				}
			}
		}
	}
	return ObjArea;
}

const char* ContinuousStructure::Update()
{
	ErrString.clear();
	for (size_t i=0;i<vProperties.size();++i) vProperties.at(i)->Update(&ErrString);
	for (size_t i=0;i<vPrimitives.size();++i) vPrimitives.at(i)->Update(&ErrString);

	return ErrString.c_str();
}

void ContinuousStructure::clear()
{
	UniqueIDCounter=0;
	dDrawingTol=0;
	maxID=0;
	SetCoordInputType(CARTESIAN);
	for (size_t i=0;i<vPrimitives.size();++i) delete vPrimitives.at(i);
	vPrimitives.clear();
	for (size_t i=0;i<vProperties.size();++i) delete vProperties.at(i);
	vProperties.clear();
	clParaSet->clear();
	clGrid.clear();
}

bool ContinuousStructure::Write2XML(TiXmlNode* rootNode, bool parameterised, bool sparse)
{
	if (rootNode==NULL) return false;

	TiXmlElement Struct("ContinuousStructure");

	Struct.SetAttribute("CoordSystem",GetCoordInputType());

	clGrid.Write2XML(Struct,false);

	clParaSet->Write2XML(Struct);

	TiXmlElement Properties("Properties");
	for (size_t i=0;i<vProperties.size();++i)
	{
		TiXmlElement PropElem(vProperties.at(i)->GetTypeXMLString().c_str());
		vProperties.at(i)->Write2XML(PropElem,parameterised,sparse);
		Properties.InsertEndChild(PropElem);
	}
	Struct.InsertEndChild(Properties);

	rootNode->InsertEndChild(Struct);

	return true;
}

bool ContinuousStructure::Write2XML(const char* file, bool parameterised, bool sparse)
{
	TiXmlDocument doc(file);
	doc.InsertEndChild(TiXmlDeclaration("1.0","ISO-8859-1","yes"));

	if (Write2XML(&doc,parameterised,sparse)==false) return false;

	doc.SaveFile();
	return doc.SaveFile();
}

const char* ContinuousStructure::ReadFromXML(TiXmlNode* rootNode)
{
	clear();
	TiXmlNode* root = rootNode->FirstChild("ContinuousStructure");
	if (root==NULL) { ErrString.append("Error: No ContinuousStructure found!!!\n"); return ErrString.c_str();}

	TiXmlElement* rootElem = root->ToElement();
	if (rootElem)
	{
		int CS_mesh = 0;
		if (rootElem->QueryIntAttribute("CoordSystem",&CS_mesh) == TIXML_SUCCESS)
			SetCoordInputType((CoordinateSystem)CS_mesh);
	}

	TiXmlNode* grid = root->FirstChild("RectilinearGrid");
	if (grid==NULL) { ErrString.append("Error: No RectilinearGrid found!!!\n"); return ErrString.c_str();}
	if (clGrid.ReadFromXML(*grid)==false) { ErrString.append("Error: RectilinearGrid invalid!!!\n"); return ErrString.c_str();}

	TiXmlNode* paraSet = root->FirstChild("ParameterSet");
	if (paraSet!=NULL) if (clParaSet->ReadFromXML(*paraSet)==false) { ErrString.append("Warning: ParameterSet reading failed!!!\n");}

	/***Properties***/
	TiXmlNode* probs = root->FirstChild("Properties");
	if (probs==NULL) { ErrString.append("Warning: Properties not found!!!\n"); return ErrString.c_str();}

	TiXmlElement* PropNode = probs->FirstChildElement();
	CSProperties* newProp=NULL;
	while (PropNode!=NULL)
	{
		const char* cProp=PropNode->Value();
		if (strcmp(cProp,"Unknown")==0) newProp = new CSPropUnknown(clParaSet);
		else if (strcmp(cProp,"Material")==0) newProp = new CSPropMaterial(clParaSet);
		else if (strcmp(cProp,"DiscMaterial")==0) newProp = new CSPropDiscMaterial(clParaSet);
		else if (strcmp(cProp,"LorentzMaterial")==0) newProp = new CSPropLorentzMaterial(clParaSet);
		else if (strcmp(cProp,"Metal")==0) newProp = new CSPropMetal(clParaSet);
		else if (strcmp(cProp,"Electrode")==0) newProp = new CSPropElectrode(clParaSet);
		else if (strcmp(cProp,"ProbeBox")==0) newProp = new CSPropProbeBox(clParaSet);
		else if (strcmp(cProp,"ChargeBox")==0) newProp = new CSPropProbeBox(clParaSet); //old version support
		else if (strcmp(cProp,"ResBox")==0) newProp = new CSPropResBox(clParaSet);
		else if (strcmp(cProp,"DumpBox")==0) newProp = new CSPropDumpBox(clParaSet);
		else
		{
			cerr << "ContinuousStructure::ReadFromXML: Property with type: " << cProp << " is unknown... " << endl;
			newProp=NULL;
		}
		if (newProp)
		{
			if (newProp->ReadFromXML(*PropNode))
			{
				AddProperty(newProp);
				ReadPropertyPrimitives(PropNode,newProp);
			}
			else
			{
				delete newProp;
				newProp = new CSPropUnknown(clParaSet);
				if (newProp->ReadFromXML(*PropNode))
				{
					AddProperty(newProp);
					ReadPropertyPrimitives(PropNode,newProp);
					ErrString.append("Warning: Unknown Property found!!!\n");
				}
				else
				{
					ErrString.append("Warning: invalid Property found!!!\n");
					delete newProp;
					newProp=NULL;
				}
			}
		}
        PropNode=PropNode->NextSiblingElement();
	}

//	/***THIS IS THE OLD Primitives read-in***/
//	TiXmlNode* prims = root->FirstChild("Primitives");
//	if (prims==NULL) { ErrString.append("Warning: Primitives not found!!!\n");}
//
//	TiXmlElement* PrimNode = prims->FirstChildElement();
//	CSPrimitives* newPrim=NULL;
//	while (PrimNode!=NULL)
//	{
//		int PropID;
//		if (PrimNode->QueryIntAttribute("PropertyID",&PropID)!=TIXML_SUCCESS)
//		{
//			ErrString.append("Warning: invalid Primitve found!!!\n");
//			PropID=-1;
//		}
//		CSProperties* prop=NULL;
//		if (PropID>=0) prop=this->GetProperty(PropID);
//		if (prop==NULL) ErrString.append("Warning: Can't read Primitve with invalid Property-ID!!!\n");
//		else
//		{
//			const char* cPrim=PrimNode->Value();
//			if (strcmp(cPrim,"Box")==0) newPrim = new CSPrimBox(clParaSet,prop);
//			else if (strcmp(cPrim,"MultiBox")==0) newPrim = new CSPrimMultiBox(clParaSet,prop);
//			else if (strcmp(cPrim,"Sphere")==0) newPrim = new CSPrimSphere(clParaSet,prop);
//			else if (strcmp(cPrim,"SphericalShell")==0) newPrim = new CSPrimSphericalShell(clParaSet,prop);
//			else if (strcmp(cPrim,"Cylinder")==0) newPrim = new CSPrimCylinder(clParaSet,prop);
//			else if (strcmp(cPrim,"CylindricalShell")==0) newPrim = new CSPrimCylindricalShell(clParaSet,prop);
//			else if (strcmp(cPrim,"Polygon")==0) newPrim = new CSPrimPolygon(clParaSet,prop);
//			else if (strcmp(cPrim,"LinPoly")==0) newPrim = new CSPrimLinPoly(clParaSet,prop);
//			else if (strcmp(cPrim,"RotPoly")==0) newPrim = new CSPrimRotPoly(clParaSet,prop);
//			else if (strcmp(cPrim,"Curve")==0) newPrim = new CSPrimCurve(clParaSet,prop);
//			else if (strcmp(cPrim,"Wire")==0) newPrim = new CSPrimWire(clParaSet,prop);
//			else if (strcmp(cPrim,"UserDefined")==0) newPrim = new CSPrimUserDefined(clParaSet,prop);
//			else
//			{
//				cerr << "ContinuousStructure::ReadFromXML: Primitive with type: " << cPrim << " is unknown... " << endl;
//				newPrim=NULL;
//			}
//			if (newPrim)
//			{
//				if (newPrim->ReadFromXML(*PrimNode)) AddPrimitive(newPrim);
//				else {delete newPrim; ErrString.append("Warning: invalid Primitive found!!!\n");}
//			}
//		}
//		PrimNode=PrimNode->NextSiblingElement();
//	}
	return ErrString.c_str();
}

bool ContinuousStructure::ReadPropertyPrimitives(TiXmlElement* PropNode, CSProperties* prop)
{
	/***Primitives***/
	TiXmlNode* prims = PropNode->FirstChild("Primitives");
	if (prims==NULL)
	{
		ErrString.append("Warning: No primitives found in property: ");
		ErrString.append(prop->GetName());
		ErrString.append("!\n");
		return false;
	}

	TiXmlElement* PrimNode = prims->FirstChildElement();
	CSPrimitives* newPrim=NULL;
	while (PrimNode!=NULL)
	{
		const char* cPrim=PrimNode->Value();
		if (strcmp(cPrim,"Box")==0) newPrim = new CSPrimBox(clParaSet,prop);
		else if (strcmp(cPrim,"MultiBox")==0) newPrim = new CSPrimMultiBox(clParaSet,prop);
		else if (strcmp(cPrim,"Sphere")==0) newPrim = new CSPrimSphere(clParaSet,prop);
		else if (strcmp(cPrim,"SphericalShell")==0) newPrim = new CSPrimSphericalShell(clParaSet,prop);
		else if (strcmp(cPrim,"Cylinder")==0) newPrim = new CSPrimCylinder(clParaSet,prop);
		else if (strcmp(cPrim,"CylindricalShell")==0) newPrim = new CSPrimCylindricalShell(clParaSet,prop);
		else if (strcmp(cPrim,"Polygon")==0) newPrim = new CSPrimPolygon(clParaSet,prop);
		else if (strcmp(cPrim,"LinPoly")==0) newPrim = new CSPrimLinPoly(clParaSet,prop);
		else if (strcmp(cPrim,"RotPoly")==0) newPrim = new CSPrimRotPoly(clParaSet,prop);
		else if (strcmp(cPrim,"Curve")==0) newPrim = new CSPrimCurve(clParaSet,prop);
		else if (strcmp(cPrim,"Wire")==0) newPrim = new CSPrimWire(clParaSet,prop);
		else if (strcmp(cPrim,"UserDefined")==0) newPrim = new CSPrimUserDefined(clParaSet,prop);
		else if (strcmp(cPrim,"Point")==0) newPrim = new CSPrimPoint(clParaSet,prop);
		else
		{
			cerr << "ContinuousStructure::ReadFromXML: Primitive with type: " << cPrim << " is unknown... " << endl;
			newPrim=NULL;
		}
		if (newPrim)
		{
			if (newPrim->ReadFromXML(*PrimNode)) AddPrimitive(newPrim);
			else
			{
				delete newPrim;
				ErrString.append("Warning: Invalid primitive found in property: ");
				ErrString.append(prop->GetName());
				ErrString.append("!\n");
			}
		}
		PrimNode=PrimNode->NextSiblingElement();
	}

	return true;
}

const char* ContinuousStructure::ReadFromXML(const char* file)
{
	ErrString.clear();

	TiXmlDocument doc(file);
	if (!doc.LoadFile()) { ErrString.append("Error: File-Loading failed!!! File: ");ErrString.append(file); return ErrString.c_str();}

	return ReadFromXML(&doc);
}

void ContinuousStructure::UpdateIDs()
{
	for (size_t i=0;i<vProperties.size();++i) vProperties.at(i)->SetID((unsigned int)i);
}

string ContinuousStructure::GetInfoLine(bool shortInfo)
{
	if (shortInfo)
	{
		string InfoLine = string(_CSXCAD_LIB_NAME_SHORT_)
						  +string(" -- Version: ") + string(_CSXCAD_VERSION_);
		return InfoLine;
	}

	string InfoLine = string(_CSXCAD_LIB_NAME_)
					  +string("\nAuthor: ") + string(_CSXCAD_AUTHOR_)
					  +string("\nMail: ") +string(_CSXCAD_AUTHOR_MAIL_)
					  +string("\nVersion: ") + string(_CSXCAD_VERSION_)
					  +string("\tBuild: ") + string(__DATE__) + string(" ") + string(__TIME__)
					  +string("\nLicense: ") + string(_CSXCAD_LICENSE_);
	return InfoLine;
}




