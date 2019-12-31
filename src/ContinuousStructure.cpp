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

#include "CSPrimPoint.h"
#include "CSPrimBox.h"
#include "CSPrimMultiBox.h"
#include "CSPrimSphere.h"
#include "CSPrimSphericalShell.h"
#include "CSPrimCylinder.h"
#include "CSPrimCylindricalShell.h"
#include "CSPrimPolygon.h"
#include "CSPrimLinPoly.h"
#include "CSPrimRotPoly.h"
#include "CSPrimPolyhedron.h"
#include "CSPrimPolyhedronReader.h"
#include "CSPrimCurve.h"
#include "CSPrimWire.h"
#include "CSPrimUserDefined.h"

#include "CSPropUnknown.h"
#include "CSPropMaterial.h"
#include "CSPropDispersiveMaterial.h"
#include "CSPropLorentzMaterial.h"
#include "CSPropDebyeMaterial.h"
#include "CSPropDiscMaterial.h"
#include "CSPropLumpedElement.h"
#include "CSPropMetal.h"
#include "CSPropConductingSheet.h"
#include "CSPropExcitation.h"
#include "CSPropProbeBox.h"
#include "CSPropDumpBox.h"
#include "CSPropResBox.h"

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
	delete clParaSet;
	clParaSet=NULL;
}

ParameterSet* ContinuousStructure::GetParameterSet() {return clParaSet;}

CSRectGrid* ContinuousStructure::GetGrid() {return &clGrid;}

CSBackgroundMaterial* ContinuousStructure::GetBackgroundMaterial() {return &m_BG_Mat;}

void ContinuousStructure::AddProperty(CSProperties* prop)
{
	if (prop==NULL) return;
	prop->SetCoordInputType(m_MeshType);
	prop->Update(&ErrString);
	vProperties.push_back(prop);
	prop->SetUniqueID(UniqueIDCounter++);
	this->UpdateIDs();
}

bool ContinuousStructure::ReplaceProperty(CSProperties* oldProp, CSProperties* newProp)
{
	std::vector<CSProperties*>::iterator iter;
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

void ContinuousStructure::DeleteProperty(size_t index)
{
	if (index>=vProperties.size()) return;
	std::vector<CSProperties*>::iterator iter=vProperties.begin();
	delete vProperties.at(index);
	vProperties.erase(iter+index);
	this->UpdateIDs();
}

void ContinuousStructure::DeleteProperty(CSProperties* prop)
{
	std::vector<CSProperties*>::iterator iter;
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

int ContinuousStructure::GetIndex(CSProperties* prop)
{
	if (prop==NULL) return -1;
	for (size_t i=0;i<vProperties.size();++i)
		if (vProperties.at(i)==prop) return (int)i;
	return -1;
}

size_t ContinuousStructure::GetQtyProperties() {return vProperties.size();}

size_t ContinuousStructure::GetQtyPropertyType(CSProperties::PropertyType type)
{
	size_t count=0;
	for (size_t i=0;i<vProperties.size();++i) if (vProperties.at(i)->GetType() & type) ++count;
	return count;
}

std::vector<CSProperties*>  ContinuousStructure::GetPropertyByType(CSProperties::PropertyType type)
{
	std::vector<CSProperties*> found;
	for (size_t i=0;i<vProperties.size();++i)
		if (vProperties.at(i)->GetType() & type)
			found.push_back(vProperties.at(i));
	return found;
}

size_t ContinuousStructure::GetQtyPrimitives(CSProperties::PropertyType type)
{
	size_t count = 0;
	for (size_t i=0;i<vProperties.size();++i)
		if (vProperties.at(i)->GetType() & type)
			count+=vProperties.at(i)->GetQtyPrimitives();
	return count;
}

bool sortPrimByPrio(CSPrimitives* a, CSPrimitives* b)
{
	return a->GetPriority()<b->GetPriority();
}

std::vector<CSPrimitives*> ContinuousStructure::GetAllPrimitives(bool sorted, CSProperties::PropertyType type)
{
	std::vector<CSProperties*> props = this->GetPropertyByType(type);
	std::vector<CSPrimitives*> vPrim;
	vPrim.reserve(GetQtyPrimitives(type));
	for (size_t i=0;i<props.size();++i)
	{
		std::vector<CSPrimitives*> prop_prims = props.at(i)->GetAllPrimitives();
		vPrim.insert(vPrim.end(),prop_prims.begin(),prop_prims.end());
	}
	if (sorted)
		sort(vPrim.rbegin(), vPrim.rend(), sortPrimByPrio);
	return vPrim;
}

CSProperties* ContinuousStructure::HasPrimitive(CSPrimitives* prim)
{
	for (size_t i=0;i<vProperties.size();++i)
		if (vProperties.at(i)->HasPrimitive(prim))
			return vProperties.at(i);
	return NULL;
}

void ContinuousStructure::DeletePrimitive(CSPrimitives* prim)
{
	// no special handling is necessary, deleted primitive will release itself from its owning property
	delete prim;
}

std::vector<CSPrimitives*> ContinuousStructure::GetPrimitivesByType(CSPrimitives::PrimitiveType type)
{
	UNUSED(type);
	std::vector<CSPrimitives*> vPrim;
	std::cerr << __func__ << ": Error, not yet implemented!" << std::endl;
	return vPrim;
}

std::vector<CSPrimitives*>  ContinuousStructure::GetPrimitivesByBoundBox(const double* boundbox, bool sorted, CSProperties::PropertyType type)
{
	std::vector<CSPrimitives*> out_list;
	std::vector<CSPrimitives*> prims =this->GetAllPrimitives(sorted, type);
	for (size_t j=0;j<prims.size();++j)
	{
		// add primitive to list of IsInside reports 0 or 1 (unknown or true)
		if (prims.at(j)->IsInsideBox(boundbox)>=0)
			out_list.push_back(prims.at(j));
	}
	return out_list;
}

bool ContinuousStructure::InsertEdges2Grid(int nu)
{
	if (nu<0) return false;
	if (nu>2) return false;
	double box[6] = {0,0,0,0,0,0};
	bool accBound=false;
	std::vector<CSPrimitives*> vPrimitives=GetAllPrimitives();
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
	std::vector<CSPrimitives*> vPrimitives=GetAllPrimitives();
	for (size_t i=0;i<vPrimitives.size();++i)
		if (vPrimitives.at(i)->GetID()==ID)
			return vPrimitives.at(i);
	return NULL;
}

std::vector<CSProperties*> ContinuousStructure::GetPropertiesByName(std::string name)
{
	std::vector<CSProperties*> vProp;
	for (size_t i=0;i<vProperties.size();++i)
		if (name.compare(vProperties.at(i)->GetName())==0)
			vProp.push_back(vProperties.at(i));
	return vProp;
}

CSProperties* ContinuousStructure::GetProperty(size_t index)
{
	if (index<vProperties.size()) return vProperties.at(index);
	return NULL;
}

CSProperties* ContinuousStructure::GetPropertyByCoordPriority(const double* coord, CSProperties::PropertyType type, bool markFoundAsUsed, CSPrimitives** foundPrimitive)
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
	if (foundPrimitive)
		*foundPrimitive=winPrim;
	return winProp;
}


CSProperties** ContinuousStructure::GetPropertiesByCoordsPriority(const double* /*coords*/, CSProperties::PropertyType /*type*/, bool /*markFoundAsUsed*/)
{
	std::cerr << "ContinuousStructure::GetPropertiesByCoordsPriority --> This methode has not been implemented yet!!! return NULL" << std::endl;
	return NULL;
}

CSProperties* ContinuousStructure::GetPropertyByCoordPriority(const double* coord, std::vector<CSPrimitives*> primList, bool markFoundAsUsed, CSPrimitives** foundPrimitive)
{
	CSProperties* prop = NULL;
	// search in all given primitives if coordinate given is inside
	for (size_t i=0;i<primList.size();++i)
		if (primList.at(i)->IsInside(coord))
		{
			if (foundPrimitive)
				*foundPrimitive = primList.at(i);
			prop = primList.at(i)->GetProperty();
			if (markFoundAsUsed)
				primList.at(i)->SetPrimitiveUsed(true);
			// break as soon as a primitive is found since it is expected that the vPrim vector is priority sorted!
			break;
		}
	return prop;
}

void ContinuousStructure::WarnUnusedPrimitves(std::ostream& stream, CSProperties::PropertyType type)
{
	for (size_t i=0;i<vProperties.size();++i)
	{
		if ((type==CSProperties::ANY) || (vProperties.at(i)->GetType() & type))
		{
			vProperties.at(i)->WarnUnusedPrimitves(stream);
		}
	}
}

void ContinuousStructure::ShowPropertyStatus(std::ostream& stream, CSProperties::PropertyType type)
{
	for (size_t i=0;i<vProperties.size();++i)
	{
		if ((type==CSProperties::ANY) || (vProperties.at(i)->GetType() & type))
		{
			stream << "-----------------------------------------" << std::endl;
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

CoordinateSystem ContinuousStructure::GetCoordInputType() {return m_MeshType;}

void ContinuousStructure::SetDrawingTolerance(double val) {dDrawingTol=val;}

bool ContinuousStructure::isGeometryValid()
{
	if (GetQtyProperties()<=0) return false;
	if (GetQtyPrimitives()<=0) return false;
	if (clGrid.GetQtyLines(0)<=1) return false;
	if (clGrid.GetQtyLines(1)<=1) return false;
	if (clGrid.GetQtyLines(2)<=0) return false;

	std::vector<CSPrimitives*> vPrimitives=GetAllPrimitives();
	for (size_t i=0;i<vPrimitives.size();++i)
	{
		if (vPrimitives.at(i)->Update()==false)
			return false;
	}

	int excit=0;
	for (size_t i=0;i<vProperties.size();++i)
	{
		if (vProperties.at(i)->Update()==false) return false;
		if (vProperties.at(i)->GetType()==CSProperties::EXCITATION)
		{
			if (vProperties.at(i)->GetQtyPrimitives()>0) ++excit;
		}
	}
	if (excit==0) return false;
	return true;
}

double* ContinuousStructure::GetObjectArea(CSProperties::PropertyType type)
{
	CSPrimitives* prim=NULL;
	bool AccBound;
	std::vector<CSPrimitives*> vPrimitives=GetAllPrimitives(type);
	for (size_t i=0;i<vPrimitives.size();++i)
	{
		prim=vPrimitives.at(i);
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

std::string ContinuousStructure::Update()
{
	ErrString.clear();

	for (size_t i=0;i<vProperties.size();++i)
		vProperties.at(i)->Update(&ErrString);

	std::vector<CSPrimitives*> vPrimitives=GetAllPrimitives();
	for (size_t i=0;i<vPrimitives.size();++i)
		vPrimitives.at(i)->Update(&ErrString);

	return std::string(ErrString);
}

void ContinuousStructure::clear()
{
	UniqueIDCounter=0;
	dDrawingTol=0;
	maxID=0;
	m_BG_Mat.Reset();
	for (unsigned int n=0;n<vProperties.size();++n)
	{
		delete vProperties.at(n);
		vProperties.at(n)=NULL;
	}
	vProperties.clear();
	SetCoordInputType(CARTESIAN);
	if (clParaSet)
		clParaSet->clear();
	clGrid.clear();
}

bool ContinuousStructure::Write2XML(TiXmlNode* rootNode, bool parameterised, bool sparse)
{
	if (rootNode==NULL) return false;

	TiXmlElement Struct("ContinuousStructure");

	Struct.SetAttribute("CoordSystem",GetCoordInputType());

	clGrid.Write2XML(Struct,false);

	m_BG_Mat.Write2XML(Struct, false);

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
	return this->Write2XML(std::string(file), parameterised, sparse);
}

bool ContinuousStructure::Write2XML(std::string file, bool parameterised, bool sparse)
{
	TiXmlDocument doc(file);
	doc.InsertEndChild(TiXmlDeclaration("1.0","UTF-8","yes"));

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

	TiXmlNode* bg_node = root->FirstChild("BackgroundMaterial");
	if (bg_node==NULL)
		m_BG_Mat.Reset(); //reset to default;
	else
		if (m_BG_Mat.ReadFromXML(*bg_node)==false)
		{
			ErrString.append("Error: BackgroundMaterial invalid!!!\n");
			return ErrString.c_str();
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
		else if (strcmp(cProp,"DebyeMaterial")==0) newProp = new CSPropDebyeMaterial(clParaSet);
		else if (strcmp(cProp,"LumpedElement")==0) newProp = new CSPropLumpedElement(clParaSet);
		else if (strcmp(cProp,"Metal")==0) newProp = new CSPropMetal(clParaSet);
		else if (strcmp(cProp,"ConductingSheet")==0) newProp = new CSPropConductingSheet(clParaSet);
		else if (strcmp(cProp,"Excitation")==0) newProp = new CSPropExcitation(clParaSet);
		else if (strcmp(cProp,"ProbeBox")==0) newProp = new CSPropProbeBox(clParaSet);
		else if (strcmp(cProp,"ChargeBox")==0) newProp = new CSPropProbeBox(clParaSet); //old version support
		else if (strcmp(cProp,"ResBox")==0) newProp = new CSPropResBox(clParaSet);
		else if (strcmp(cProp,"DumpBox")==0) newProp = new CSPropDumpBox(clParaSet);
		else
		{
			std::cerr << "ContinuousStructure::ReadFromXML: Property with type: " << cProp << " is unknown... " << std::endl;
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
		else if (strcmp(cPrim,"Polyhedron")==0) newPrim = new CSPrimPolyhedron(clParaSet,prop);
		else if (strcmp(cPrim,"PolyhedronReader")==0) newPrim = new CSPrimPolyhedronReader(clParaSet,prop);
		else if (strcmp(cPrim,"Curve")==0) newPrim = new CSPrimCurve(clParaSet,prop);
		else if (strcmp(cPrim,"Wire")==0) newPrim = new CSPrimWire(clParaSet,prop);
		else if (strcmp(cPrim,"UserDefined")==0) newPrim = new CSPrimUserDefined(clParaSet,prop);
		else if (strcmp(cPrim,"Point")==0) newPrim = new CSPrimPoint(clParaSet,prop);
		else
		{
			std::cerr << "ContinuousStructure::ReadFromXML: Primitive with type: " << cPrim << " is unknown... " << std::endl;
			newPrim=NULL;
		}
		if (newPrim)
		{
			if (newPrim->ReadFromXML(*PrimNode))
			{
				newPrim->SetCoordInputType(m_MeshType, false);
				newPrim->Update(&ErrString);
			}
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
	if (!doc.LoadFile(TIXML_ENCODING_UTF8)) { ErrString.append("Error: File-Loading failed!!! File: ");ErrString.append(file); return ErrString.c_str();}

	return ReadFromXML(&doc);
}

std::string ContinuousStructure::ReadFromXML(std::string file)
{
	return ReadFromXML(file.c_str());
}

void ContinuousStructure::UpdateIDs()
{
	for (size_t i=0;i<vProperties.size();++i)
		vProperties.at(i)->SetID((unsigned int)i);
}

std::string ContinuousStructure::GetInfoLine(bool shortInfo)
{
	if (shortInfo)
	{
		std::string InfoLine = std::string(_CSXCAD_LIB_NAME_SHORT_)
						  +std::string(" -- Version: ") + std::string(_CSXCAD_VERSION_);
		return InfoLine;
	}

	std::string InfoLine = std::string(_CSXCAD_LIB_NAME_)
					  +std::string("\nAuthor: ") + std::string(_CSXCAD_AUTHOR_)
					  +std::string("\nMail: ") +std::string(_CSXCAD_AUTHOR_MAIL_)
					  +std::string("\nVersion: ") + std::string(_CSXCAD_VERSION_)
					  +std::string("\tBuild: ") + std::string(__DATE__) + std::string(" ") + std::string(__TIME__)
					  +std::string("\nLicense: ") + std::string(_CSXCAD_LICENSE_);
	return InfoLine;
}




