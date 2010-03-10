#include "ContinuousStructure.h"
#include "tinyxml.h"

/*********************ContinuousStructure********************************************************************/
ContinuousStructure::ContinuousStructure(void)
{
	vPrimitives.clear();
	maxID=0;
	clParaSet = new ParameterSet();
	UniqueIDCounter=0;
	dDrawingTol=0;
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
	if (prim->GetID()<=maxID) prim->SetID(maxID++);
	else maxID=prim->GetID()+1;
}

void ContinuousStructure::AddProperty(CSProperties* prop)
{
	if (prop==NULL) return;
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
	double *box=NULL;
	bool accBound=false;
	for (size_t i=0;i<vPrimitives.size();++i)
	{
		box=vPrimitives.at(i)->GetBoundBox(accBound);
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

CSProperties* ContinuousStructure::GetPropertyByCoordPriority(double* coord, CSProperties::PropertyType type)
{
	CSProperties* winProp=NULL;
	int winPrio=0;
	int locPrio=0;
	for (size_t i=0;i<vProperties.size();++i)
	{
		if ((type==CSProperties::ANY) || (vProperties.at(i)->GetType() & type))
		{
			if (vProperties.at(i)->CheckCoordInPrimitive(coord,locPrio,dDrawingTol))
			{
				if (winProp==NULL) winPrio=locPrio-1;//make the first one found always highest
				if (locPrio>winPrio)
				{
					winPrio=locPrio;
					winProp=vProperties.at(i);
				}
			}
		}
	}
	return winProp;
}


CSProperties** ContinuousStructure::GetPropertiesByCoordsPriority(double* coords, CSProperties::PropertyType type)
{
	cerr << "ContinuousStructure::GetPropertiesByCoordsPriority --> This methode has not been implemented yet!!! return NULL" << endl;
	return NULL;
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
	bool AccBound=false;
	for (size_t i=0;i<GetQtyPrimitives();++i)
	{
		prim=GetPrimitive(i);
		double* box=prim->GetBoundBox(AccBound);
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
	maxID=0;
	for (size_t i=0;i<vPrimitives.size();++i) delete vPrimitives.at(i);
	vPrimitives.clear();
	for (size_t i=0;i<vProperties.size();++i) delete vProperties.at(i);
	vProperties.clear();
	clParaSet->clear();
	clGrid.clear();
}

bool ContinuousStructure::Write2XML(TiXmlNode* rootNode, bool parameterised)
{
	if (rootNode==NULL) return false;

	TiXmlElement Struct("ContinuousStructure");

	clGrid.Write2XML(Struct,false);

	clParaSet->Write2XML(Struct);

	TiXmlElement Properties("Properties");
	for (size_t i=0;i<vProperties.size();++i)
	{
		vProperties.at(i)->Write2XML(Properties,parameterised);
	}
	Struct.InsertEndChild(Properties);
	TiXmlElement Primitives("Primitives");
	for (size_t i=0;i<vPrimitives.size();++i)
	{
		vPrimitives.at(i)->Write2XML(Primitives,parameterised);
	}
	Struct.InsertEndChild(Primitives);

	rootNode->InsertEndChild(Struct);

	return true;
}

bool ContinuousStructure::Write2XML(const char* file, bool parameterised)
{
	TiXmlDocument doc(file);
	doc.InsertEndChild(TiXmlDeclaration("1.0","ISO-8859-1","yes"));

	if (Write2XML(&doc,parameterised)==false) return false;

	doc.SaveFile();
	return doc.SaveFile();
}

const char* ContinuousStructure::ReadFromXML(TiXmlNode* rootNode)
{
	clear();
	TiXmlNode* root = rootNode->FirstChild("ContinuousStructure");
	if (root==NULL) { ErrString.append("Error: No ContinuousStructure found!!!\n"); return ErrString.c_str();}

	TiXmlNode* grid = root->FirstChild("RectilinearGrid");
	if (grid==NULL) { ErrString.append("Error: No RectilinearGrid found!!!\n"); return ErrString.c_str();}
	if (clGrid.ReadFromXML(*grid)==false) { ErrString.append("Error: RectilinearGrid invalid!!!\n"); return ErrString.c_str();}

	TiXmlNode* paraSet = root->FirstChild("ParameterSet");
	if (paraSet!=NULL) if (clParaSet->ReadFromXML(*paraSet)==false) { ErrString.append("Warning: ParameterSet reading failed!!!\n");}

	/***Properties***/
	TiXmlNode* probs = root->FirstChild("Properties");
	if (probs==NULL) { ErrString.append("Warning: Properties not found!!!\n");}

	TiXmlElement* PropNode = probs->FirstChildElement();
	CSProperties* newProp=NULL;
	while (PropNode!=NULL)
	{
		const char* cProp=PropNode->Value();
		if (strcmp(cProp,"Unknown")==0) newProp = new CSPropUnknown(clParaSet);
		else if (strcmp(cProp,"Material")==0) newProp = new CSPropMaterial(clParaSet);
		else if (strcmp(cProp,"Metal")==0) newProp = new CSPropMetal(clParaSet);
		else if (strcmp(cProp,"Electrode")==0) newProp = new CSPropElectrode(clParaSet);
		else if (strcmp(cProp,"ProbeBox")==0) newProp = new CSPropProbeBox(clParaSet);
		else if (strcmp(cProp,"ChargeBox")==0) newProp = new CSPropProbeBox(clParaSet); //old version support
		else if (strcmp(cProp,"ResBox")==0) newProp = new CSPropResBox(clParaSet);
		else if (strcmp(cProp,"DumpBox")==0) newProp = new CSPropDumpBox(clParaSet);
		else break;

		if (newProp->ReadFromXML(*PropNode)) AddProperty(newProp);
		else
		{
			delete newProp;
			newProp = new CSPropUnknown(clParaSet);
			if (newProp->ReadFromXML(*PropNode))
			{
				AddProperty(newProp);
				ErrString.append("Warning: Unknown Property found!!!\n");
			}
			else
			{
				ErrString.append("Warning: invalid Property found!!!\n");
				delete newProp;
				newProp=NULL;
			}
		}
        PropNode=PropNode->NextSiblingElement();
	}

	/***Primitives***/
	TiXmlNode* prims = root->FirstChild("Primitives");
	if (prims==NULL) { ErrString.append("Warning: Primitives not found!!!\n");}

	TiXmlElement* PrimNode = prims->FirstChildElement();
	CSPrimitives* newPrim=NULL;
	while (PrimNode!=NULL)
	{
		int PropID;
		if (PrimNode->QueryIntAttribute("PropertyID",&PropID)!=TIXML_SUCCESS)
		{
			ErrString.append("Warning: invalid Primitve found!!!\n");
			PropID=-1;
		}
		CSProperties* prop=NULL;
		if (PropID>=0) prop=this->GetProperty(PropID);
		if (prop==NULL) ErrString.append("Warning: Can't read Primitve with invalid Property-ID!!!\n");
		else
		{
			const char* cPrim=PrimNode->Value();
			if (strcmp(cPrim,"Box")==0) newPrim = new CSPrimBox(clParaSet,prop);
			else if (strcmp(cPrim,"MultiBox")==0) newPrim = new CSPrimMultiBox(clParaSet,prop);
			else if (strcmp(cPrim,"Sphere")==0) newPrim = new CSPrimSphere(clParaSet,prop);
			else if (strcmp(cPrim,"Cylinder")==0) newPrim = new CSPrimCylinder(clParaSet,prop);
			else if (strcmp(cPrim,"Polygon")==0) newPrim = new CSPrimPolygon(clParaSet,prop);
			else if (strcmp(cPrim,"LinPoly")==0) newPrim = new CSPrimLinPoly(clParaSet,prop);
			else if (strcmp(cPrim,"RotPoly")==0) newPrim = new CSPrimRotPoly(clParaSet,prop);
			else if (strcmp(cPrim,"UserDefined")==0) newPrim = new CSPrimUserDefined(clParaSet,prop);
			else break;

			if (newPrim->ReadFromXML(*PrimNode)) AddPrimitive(newPrim);
			else {delete newPrim; ErrString.append("Warning: invalid Primitive found!!!\n"); }
		}
		PrimNode=PrimNode->NextSiblingElement();
	}
	ErrString.append(Update());
	return ErrString.c_str();
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

string ContinuousStructure::GetInfoLine()
{
	string InfoLine = string(_CSXCAD_LIB_NAME_) + string("\nAuthor: ") + string(_CSXCAD_AUTHOR_) +string("\nMail: ") +string(_CSXCAD_AUTHOR_MAIL_) + string("\nVersion: ") + string(_CSXCAD_VERSION_) +string("\tBuild: ") + string(__DATE__) + string(" ") + string(__TIME__);
	return InfoLine;
}




