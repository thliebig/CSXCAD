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

#include "CSPrimPolyhedron.h"
#include "CSPrimPolyhedron_p.h"
#include "CSProperties.h"
#include "CSUseful.h"

void Polyhedron_Builder::operator()(HalfedgeDS &hds)
{
	// Postcondition: `hds' is a valid polyhedral surface.
	CGAL::Polyhedron_incremental_builder_3<HalfedgeDS> B( hds, true);
	B.begin_surface( m_polyhedron->m_Vertices.size(), m_polyhedron->m_Faces.size());
	typedef HalfedgeDS::Vertex   Vertex;
	typedef Vertex::Point Point;
	for (size_t n=0;n<m_polyhedron->m_Vertices.size();++n)
		B.add_vertex( Point( m_polyhedron->m_Vertices.at(n).coord[0], m_polyhedron->m_Vertices.at(n).coord[1], m_polyhedron->m_Vertices.at(n).coord[2]));

	for (size_t f=0;f<m_polyhedron->m_Faces.size();++f)
	{
		m_polyhedron->m_Faces.at(f).valid=false;
		int *first = m_polyhedron->m_Faces.at(f).vertices, *beyond = first+m_polyhedron->m_Faces.at(f).numVertex;
		if (B.test_facet(first, beyond))
		{
			B.add_facet(first, beyond);
			if (B.error())
			{
				std::cerr << "Polyhedron_Builder::operator(): Error in polyhedron construction" << std::endl;
				break;
			}
			m_polyhedron->m_Faces.at(f).valid=true;
		}
		else
		{
			std::cerr << "Polyhedron_Builder::operator(): Face " << f << ": Trying reverse order... ";
			int* help = new int [m_polyhedron->m_Faces.at(f).numVertex];
			for (unsigned int n=0;n<m_polyhedron->m_Faces.at(f).numVertex;++n)
				help[n]=m_polyhedron->m_Faces.at(f).vertices[m_polyhedron->m_Faces.at(f).numVertex-1-n];
			first = help;
			beyond = first+m_polyhedron->m_Faces.at(f).numVertex;
			if (B.test_facet(first, beyond))
			{
				B.add_facet(first, beyond);
				if (B.error())
				{
					std::cerr << "Polyhedron_Builder::operator(): Error in polyhedron construction" << std::endl;
					break;
				}
				std::cerr << "success" << std::endl;
				m_polyhedron->m_Faces.at(f).valid=true;
			}
			else
			{
				std::cerr << "failed" << std::endl;
				++m_polyhedron->m_InvalidFaces;
			}
			delete[] help; help = NULL;
		}
	}
	B.end_surface();
}

/*********************CSPrimPolyhedron********************************************************************/
CSPrimPolyhedron::CSPrimPolyhedron(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop), d_ptr(new CSPrimPolyhedronPrivate)
{
	Type = POLYHEDRON;
	PrimTypeName = "Polyhedron";
	d_ptr->m_PolyhedronTree = NULL;
	m_InvalidFaces = 0;
}

CSPrimPolyhedron::CSPrimPolyhedron(CSPrimPolyhedron* primPolyhedron, CSProperties *prop) : CSPrimitives(primPolyhedron,prop), d_ptr(new CSPrimPolyhedronPrivate)
{
	Type = POLYHEDRON;
	PrimTypeName = "Polyhedron";
	d_ptr->m_PolyhedronTree = NULL;
	m_InvalidFaces = 0;

	//copy all vertices
	for (size_t n=0;n<primPolyhedron->m_Vertices.size();++n)
		AddVertex(primPolyhedron->m_Vertices.at(n).coord);
	//copy all faces
	for (size_t n=0;n<primPolyhedron->m_Faces.size();++n)
		AddFace(primPolyhedron->m_Faces.at(n).numVertex,primPolyhedron->m_Faces.at(n).vertices);
}

CSPrimPolyhedron::CSPrimPolyhedron(ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(paraSet,prop), d_ptr(new CSPrimPolyhedronPrivate)
{
	Type = POLYHEDRON;
	PrimTypeName = "Polyhedron";
	d_ptr->m_PolyhedronTree = NULL;
	m_InvalidFaces = 0;
}

CSPrimPolyhedron::~CSPrimPolyhedron()
{
	Reset();
	delete d_ptr;
}

void CSPrimPolyhedron::Reset()
{
	m_Vertices.clear();
	for (size_t n=0;n<m_Faces.size();++n)
	{
		delete[] m_Faces.at(n).vertices;
		m_Faces.at(n).vertices=NULL;
	}
	m_Faces.clear();
	d_ptr->m_Polyhedron.clear();
	d_ptr->m_PolyhedronTree = NULL;
	m_InvalidFaces = 0;
}

void CSPrimPolyhedron::AddVertex(float px, float py, float pz)
{
	vertex nv;
	nv.coord[0]=px;nv.coord[1]=py;nv.coord[2]=pz;
	m_Vertices.push_back(nv);
}

float* CSPrimPolyhedron::GetVertex(unsigned int n)
{
	if (n<m_Vertices.size())
		return m_Vertices.at(n).coord;
	return NULL;
}

void CSPrimPolyhedron::AddFace(face f)
{
	m_Faces.push_back(f);
}

void CSPrimPolyhedron::AddFace(int numVertex, int* vertices)
{
	face f;
	f.numVertex=numVertex;
	f.vertices=new int[numVertex];
	for (int n=0;n<numVertex;++n)
		f.vertices[n]=vertices[n];
	m_Faces.push_back(f);
}

void CSPrimPolyhedron::AddFace(std::vector<int> vertices)
{
	face f;
	f.numVertex=vertices.size();
	if (f.numVertex>3)
		std::cerr << __func__ << ": Warning, faces other than triangles are currently not supported for discretization, expect false results!!!" << std::endl;
	f.vertices=new int[f.numVertex];
	for (unsigned int n=0;n<f.numVertex;++n)
		f.vertices[n]=vertices.at(n);
	m_Faces.push_back(f);
}

bool CSPrimPolyhedron::BuildTree()
{
	Polyhedron_Builder builder(this);
	d_ptr->m_Polyhedron.delegate(builder);

	if (d_ptr->m_Polyhedron.is_closed())
		m_Dimension = 3;
	else
	{
		m_Dimension = 2;

		//if structure is not closed due to invalud faces, mark it as 3D
		if (m_InvalidFaces>0)
		{
			m_Dimension = 3;
			std::cerr << "CSPrimPolyhedron::BuildTree: Warning, found polyhedron has invalud faces and is not a closed surface, setting to 3D solid anyway!" << std::endl;
		}
	}

	//build tree
	delete d_ptr->m_PolyhedronTree;
#if CGAL_VERSION_NR >= CGAL_VERSION_NUMBER(4,6,0)
    d_ptr->m_PolyhedronTree = new CGAL::AABB_tree< Traits >(faces(d_ptr->m_Polyhedron).first,faces(d_ptr->m_Polyhedron).second,d_ptr->m_Polyhedron);
#else
    d_ptr->m_PolyhedronTree = new CGAL::AABB_tree< Traits >(d_ptr->m_Polyhedron.facets_begin(),d_ptr->m_Polyhedron.facets_end());
#endif

	//update local bounding box
	GetBoundBox(m_BoundBox);
	double p[3] = {m_BoundBox[1]*(1.0+(double)rand()/RAND_MAX),m_BoundBox[3]*(1.0+(double)rand()/RAND_MAX),m_BoundBox[5]*(1.0+(double)rand()/RAND_MAX)};
	d_ptr->m_RandPt = Point(p[0],p[1],p[2]);
	return true;
}

int* CSPrimPolyhedron::GetFace(unsigned int n, unsigned int &numVertices)
{
	numVertices = 0;
	if (n<m_Faces.size())
	{
		numVertices = m_Faces.at(n).numVertex;
		return m_Faces.at(n).vertices;
	}
	return NULL;
}

bool CSPrimPolyhedron::GetBoundBox(double dBoundBox[6], bool PreserveOrientation)
{
	UNUSED(PreserveOrientation); //has no orientation or preserved anyways
	m_BoundBox_CoordSys=CARTESIAN;

	if (m_Vertices.size()==0)
		return true;

	dBoundBox[0]=dBoundBox[1]=m_Vertices.at(0).coord[0];
	dBoundBox[2]=dBoundBox[3]=m_Vertices.at(0).coord[1];
	dBoundBox[4]=dBoundBox[5]=m_Vertices.at(0).coord[2];

	for (size_t n=0;n<m_Vertices.size();++n)
	{
		dBoundBox[0]=std::min(dBoundBox[0],(double)m_Vertices.at(n).coord[0]);
		dBoundBox[2]=std::min(dBoundBox[2],(double)m_Vertices.at(n).coord[1]);
		dBoundBox[4]=std::min(dBoundBox[4],(double)m_Vertices.at(n).coord[2]);
		dBoundBox[1]=std::max(dBoundBox[1],(double)m_Vertices.at(n).coord[0]);
		dBoundBox[3]=std::max(dBoundBox[3],(double)m_Vertices.at(n).coord[1]);
		dBoundBox[5]=std::max(dBoundBox[5],(double)m_Vertices.at(n).coord[2]);
	}
	return true;
}

bool CSPrimPolyhedron::IsInside(const double* Coord, double /*tol*/)
{
	if (m_Dimension<3)
		return false;

	double pos[3];
	//transform incoming coordinates into cartesian coords
	TransformCoordSystem(Coord,pos,m_MeshType,CARTESIAN);
	if (m_Transform)
		m_Transform->InvertTransform(pos,pos);

	for (unsigned int n=0;n<3;++n)
	{
		if ((m_BoundBox[2*n]>pos[n]) || (m_BoundBox[2*n+1]<pos[n])) return false;
	}

	Point p(pos[0], pos[1], pos[2]);
	Segment segment_query(p,d_ptr->m_RandPt);
	// return true for an odd number of intersections
	if ((d_ptr->m_PolyhedronTree->number_of_intersected_primitives(segment_query)%2)==1)
		return true;
	return false;
}


bool CSPrimPolyhedron::Update(std::string *ErrStr)
{
	BuildTree();
	//update local bounding box
	m_BoundBoxValid = GetBoundBox(m_BoundBox);
	return CSPrimitives::Update(ErrStr);
}

bool CSPrimPolyhedron::Write2XML(TiXmlElement &elem, bool parameterised)
{
	if (CSPrimitives::Write2XML(elem,parameterised)==false)
		return false;

	for (size_t n=0;n<m_Vertices.size();++n)
	{
		TiXmlElement vertex("Vertex");
		TiXmlText text(CombineArray2String(m_Vertices.at(n).coord,3,','));
		vertex.InsertEndChild(text);
		elem.InsertEndChild(vertex);
	}
	for (size_t n=0;n<m_Faces.size();++n)
	{
		TiXmlElement face("Face");
		TiXmlText text(CombineArray2String(m_Faces.at(n).vertices,m_Faces.at(n).numVertex,','));
		face.InsertEndChild(text);
		elem.InsertEndChild(face);
	}
	return true;
}

bool CSPrimPolyhedron::ReadFromXML(TiXmlNode &root)
{
	if (!CSPrimitives::ReadFromXML(root)) return false;
	TiXmlNode* FN=NULL;
	TiXmlText* Text=NULL;

	// read vertices
	std::vector<double> coords;
	TiXmlElement* vertex = root.FirstChildElement("Vertex");
	while (vertex)
	{
		coords.clear();
		FN = vertex->FirstChild();
		if (FN!=NULL)
		{
			Text = FN->ToText();
			if (Text!=NULL)
				coords = SplitString2Double(std::string(Text->Value()), ',');
			else
				return false;
			if (coords.size()!=3)
				return false;
			AddVertex(coords.at(0),coords.at(1),coords.at(2));
		}
		else
			return false;
		vertex = vertex->NextSiblingElement("Vertex");
	}

	// read faces
	std::vector<int> vertices;
	TiXmlElement* face = root.FirstChildElement("Face");
	while (face)
	{
		coords.clear();
		FN = face->FirstChild();
		if (FN!=NULL)
		{
			Text = FN->ToText();
			if (Text!=NULL)
				vertices = SplitString2Int(std::string(Text->Value()), ',');
			else
				return false;
			AddFace(vertices);
		}
		else
			return false;
		face = face->NextSiblingElement("Face");
	}
	return BuildTree();
}


void CSPrimPolyhedron::ShowPrimitiveStatus(std::ostream& stream)
{
	CSPrimitives::ShowPrimitiveStatus(stream);
	stream << " Number of Vertices: " << m_Vertices.size() << std::endl;
	stream << " Number of Faces: " << m_Faces.size() << std::endl;
	stream << " Number of invalid Faces: " << m_InvalidFaces << std::endl;
}
