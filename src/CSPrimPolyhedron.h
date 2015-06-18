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

#pragma once

#include "CSPrimitives.h"

struct CSPrimPolyhedronPrivate;

//! Polyhedron Primitive
/*!
 This is a polyhedron primitive. A 3D solid object, defined by vertices and faces
 */
class CSXCAD_EXPORT CSPrimPolyhedron : public CSPrimitives
{
	friend class Polyhedron_Builder;
public:
	struct face
	{
		unsigned int numVertex;
		int* vertices;
		bool valid;
	};
	struct vertex
	{
		float coord[3];
	};

	CSPrimPolyhedron(ParameterSet* paraSet, CSProperties* prop);
	CSPrimPolyhedron(CSPrimPolyhedron* primPolyhedron, CSProperties *prop=NULL);
	CSPrimPolyhedron(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimPolyhedron();

	virtual void Reset();

	virtual void AddVertex(float p[3]) {AddVertex(p[0],p[1],p[2]);}
	virtual void AddVertex(double p[3]) {AddVertex(p[0],p[1],p[2]);}
	virtual void AddVertex(float px, float py, float pz);

	virtual unsigned int GetNumVertices() const {return m_Vertices.size();}
	virtual float* GetVertex(unsigned int n);

	virtual void AddFace(face f);
	virtual void AddFace(int numVertex, int* vertices);
	virtual void AddFace(std::vector<int> vertices);

	virtual bool BuildTree();

	virtual unsigned int GetNumFaces() const {return m_Faces.size();}
	virtual int* GetFace(unsigned int n, unsigned int &numVertices);
	virtual bool GetFaceValid(unsigned int n) const {return m_Faces.at(n).valid;}

	virtual CSPrimPolyhedron* GetCopy(CSProperties *prop=NULL) {return new CSPrimPolyhedron(this,prop);}

	virtual bool GetBoundBox(double dBoundBox[6], bool PreserveOrientation=false);
	virtual bool IsInside(const double* Coord, double tol=0);

	virtual bool Update(std::string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void ShowPrimitiveStatus(std::ostream& stream);

protected:
	unsigned int m_InvalidFaces;
	std::vector<vertex> m_Vertices;
	std::vector<face> m_Faces;
	CSPrimPolyhedronPrivate *d_ptr; //!< pointer to private data structure, to hide the CGAL dependency from applications
};
