/*
*	Copyright (C) 2008-2025 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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
#include <cmath>
#include <algorithm>
#include <vector>
#include "tinyxml.h"
#include "stdint.h"

#include "CSPrimPolyhedron.h"
#include "CSProperties.h"
#include "CSUseful.h"

//! The triangles of a polyhedron in a bounding volume tree, for the inside test
struct CSPrimPolyhedronPrivate
{
	//! the corners a, b, c of every triangle, 9 values each
	std::vector<double> tri;

	//! a node of the tree: a box around triangles, either two children or a range of order
	struct Node
	{
		double box[6];  // xmin, xmax, ymin, ymax, zmin, zmax
		int child[2];   // -1 for a leaf
		int start, count;
	};
	std::vector<Node> nodes;
	std::vector<int> order; // triangle indices, the leaves' ranges point into it

	//! no edge with an odd number of faces
	bool closed;
	//! every edge passed by as many faces in one direction as in the other
	bool oriented;
	//! the length of the bounding box diagonal
	double size;

	CSPrimPolyhedronPrivate() : closed(false), oriented(false), size(0) {}
	void Clear() {tri.clear(); nodes.clear(); order.clear(); closed=oriented=false; size=0;}

	//! build the tree over tri
	void Build();
	//! a triangle closer to p than dist
	bool Near(const double p[3], double dist) const;
	//! the faces crossed by a ray from p in direction d: count and winding (+1 leaving through a
	//! face's front); false if the ray passes too close to an edge or corner to tell
	bool Cast(const double p[3], const double d[3], int& count, int& winding) const;

private:
	int BuildNode(int start, int count);
};

namespace {

inline void Sub(const double* a, const double* b, double* r) {r[0]=a[0]-b[0]; r[1]=a[1]-b[1]; r[2]=a[2]-b[2];}
inline double Dot(const double* a, const double* b) {return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];}
inline void Cross(const double* a, const double* b, double* r)
{
	r[0]=a[1]*b[2]-a[2]*b[1];
	r[1]=a[2]*b[0]-a[0]*b[2];
	r[2]=a[0]*b[1]-a[1]*b[0];
}

//! squared distance of p to the triangle a, b, c (Ericson, Real-Time Collision Detection, 5.1.5)
double TriangleDist2(const double* p, const double* a, const double* b, const double* c)
{
	double ab[3], ac[3], ap[3], bp[3], cp[3], q[3], d[3];
	Sub(b,a,ab); Sub(c,a,ac); Sub(p,a,ap);
	double d1=Dot(ab,ap), d2=Dot(ac,ap);
	if (d1<=0 && d2<=0)
		return Dot(ap,ap);
	Sub(p,b,bp);
	double d3=Dot(ab,bp), d4=Dot(ac,bp);
	if (d3>=0 && d4<=d3)
		return Dot(bp,bp);
	double vc=d1*d4-d3*d2;
	if (vc<=0 && d1>=0 && d3<=0)
	{
		double v=d1/(d1-d3);
		for (int i=0;i<3;++i) q[i]=a[i]+v*ab[i];
		Sub(p,q,d);
		return Dot(d,d);
	}
	Sub(p,c,cp);
	double d5=Dot(ab,cp), d6=Dot(ac,cp);
	if (d6>=0 && d5<=d6)
		return Dot(cp,cp);
	double vb=d5*d2-d1*d6;
	if (vb<=0 && d2>=0 && d6<=0)
	{
		double w=d2/(d2-d6);
		for (int i=0;i<3;++i) q[i]=a[i]+w*ac[i];
		Sub(p,q,d);
		return Dot(d,d);
	}
	double va=d3*d6-d5*d4;
	if (va<=0 && (d4-d3)>=0 && (d5-d6)>=0)
	{
		double w=(d4-d3)/((d4-d3)+(d5-d6));
		for (int i=0;i<3;++i) q[i]=b[i]+w*(c[i]-b[i]);
		Sub(p,q,d);
		return Dot(d,d);
	}
	double denom=1.0/(va+vb+vc);
	double v=vb*denom, w=vc*denom;
	for (int i=0;i<3;++i) q[i]=a[i]+ab[i]*v+ac[i]*w;
	Sub(p,q,d);
	return Dot(d,d);
}

//! squared distance of p to a box
double BoxDist2(const double* p, const double* box)
{
	double d2=0;
	for (int i=0;i<3;++i)
	{
		double d=0;
		if (p[i]<box[2*i]) d=box[2*i]-p[i];
		else if (p[i]>box[2*i+1]) d=p[i]-box[2*i+1];
		d2+=d*d;
	}
	return d2;
}

//! does the ray p + t*d, t>=0, meet the box (slab test)
bool RayHitsBox(const double* p, const double* inv_d, const double* box)
{
	double t0=0, t1=std::numeric_limits<double>::infinity();
	for (int i=0;i<3;++i)
	{
		double ta=(box[2*i]-p[i])*inv_d[i], tb=(box[2*i+1]-p[i])*inv_d[i];
		if (ta>tb) std::swap(ta,tb);
		// a NaN (ray in the box's plane) leaves the interval as it is
		if (ta>t0) t0=ta;
		if (tb<t1) t1=tb;
		if (t0>t1) return false;
	}
	return true;
}

// the directions of the rays, in the order they are tried: Fibonacci points on a half sphere,
// z = 1-(i+1/2)/7 at the azimuth of i golden angles plus 150 degrees, turned by 82 degrees
// around x. So each is at least 18 degrees away from any axis, face and cube diagonal (the
// usual directions of edges and of lines through corners) and 45 degrees from the others.
const int NUM_RAYS = 7;
struct RayDirections
{
	double d[NUM_RAYS][3];
	RayDirections()
	{
		const double pi = 3.14159265358979323846;
		const double golden = pi*(3-std::sqrt(5.0));
		const double phi0 = 150*pi/180, tilt = 82*pi/180;
		for (int i=0;i<NUM_RAYS;++i)
		{
			double z = 1-(i+0.5)/NUM_RAYS, r = std::sqrt(1-z*z), phi = phi0+i*golden;
			double y = r*std::sin(phi);
			d[i][0] = r*std::cos(phi);
			d[i][1] = y*std::cos(tilt)-z*std::sin(tilt);
			d[i][2] = y*std::sin(tilt)+z*std::cos(tilt);
		}
	}
};
const RayDirections RAY_DIRS;

// how close to an edge (in parts of the triangle) a ray may pass before another direction is tried
const double EDGE_MARGIN = 1e-7;

}

void CSPrimPolyhedronPrivate::Build()
{
	nodes.clear();
	order.resize(tri.size()/9);
	for (size_t n=0;n<order.size();++n)
		order[n]=(int)n;
	if (order.empty())
		return;
	nodes.reserve(order.size()/2+1);
	BuildNode(0, (int)order.size());
}

int CSPrimPolyhedronPrivate::BuildNode(int start, int count)
{
	int idx = (int)nodes.size();
	nodes.push_back(Node());
	Node node;
	node.child[0]=node.child[1]=-1;
	node.start=start;
	node.count=count;
	double cbox[6];
	for (int i=0;i<3;++i)
	{
		node.box[2*i]=cbox[2*i]=std::numeric_limits<double>::infinity();
		node.box[2*i+1]=cbox[2*i+1]=-std::numeric_limits<double>::infinity();
	}
	for (int n=start;n<start+count;++n)
	{
		const double* t=&tri[9*order[n]];
		for (int i=0;i<3;++i)
		{
			double c=(t[i]+t[3+i]+t[6+i])/3;
			cbox[2*i]=std::min(cbox[2*i],c);
			cbox[2*i+1]=std::max(cbox[2*i+1],c);
			for (int k=0;k<3;++k)
			{
				node.box[2*i]=std::min(node.box[2*i],t[3*k+i]);
				node.box[2*i+1]=std::max(node.box[2*i+1],t[3*k+i]);
			}
		}
	}
	if (count>4)
	{
		// split at the median centre along the longest extent of the centres
		int axis=0;
		for (int i=1;i<3;++i)
			if (cbox[2*i+1]-cbox[2*i] > cbox[2*axis+1]-cbox[2*axis])
				axis=i;
		if (cbox[2*axis+1]>cbox[2*axis])
		{
			int half=count/2;
			const std::vector<double>& T=tri;
			std::nth_element(order.begin()+start, order.begin()+start+half, order.begin()+start+count,
				[&T,axis](int a, int b) {return T[9*a+axis]+T[9*a+3+axis]+T[9*a+6+axis] < T[9*b+axis]+T[9*b+3+axis]+T[9*b+6+axis];});
			node.child[0]=BuildNode(start, half);
			node.child[1]=BuildNode(start+half, count-half);
			node.count=0;
		}
	}
	nodes[idx]=node;
	return idx;
}

bool CSPrimPolyhedronPrivate::Near(const double p[3], double dist) const
{
	if (nodes.empty())
		return false;
	double d2=dist*dist;
	int stack[128];
	int top=0;
	stack[top++]=0;
	while (top>0)
	{
		const Node& node=nodes[stack[--top]];
		if (BoxDist2(p,node.box)>d2)
			continue;
		if (node.child[0]<0)
		{
			for (int n=node.start;n<node.start+node.count;++n)
			{
				const double* t=&tri[9*order[n]];
				if (TriangleDist2(p,t,t+3,t+6)<=d2)
					return true;
			}
			continue;
		}
		stack[top++]=node.child[0];
		stack[top++]=node.child[1];
	}
	return false;
}

bool CSPrimPolyhedronPrivate::Cast(const double p[3], const double d[3], int& count, int& winding) const
{
	count=0;
	winding=0;
	if (nodes.empty())
		return true;
	double inv_d[3];
	for (int i=0;i<3;++i)
		inv_d[i]=1.0/d[i];
	int stack[128];
	int top=0;
	stack[top++]=0;
	while (top>0)
	{
		const Node& node=nodes[stack[--top]];
		if (!RayHitsBox(p,inv_d,node.box))
			continue;
		if (node.child[0]>=0)
		{
			stack[top++]=node.child[0];
			stack[top++]=node.child[1];
			continue;
		}
		for (int n=node.start;n<node.start+node.count;++n)
		{
			// Moeller-Trumbore
			const double* a=&tri[9*order[n]];
			double e1[3], e2[3], pv[3], tv[3], qv[3];
			Sub(a+3,a,e1);
			Sub(a+6,a,e2);
			Cross(d,e2,pv);
			double det=Dot(e1,pv);
			double scale=std::sqrt(Dot(e1,e1)*Dot(e2,e2));
			Sub(p,a,tv);
			if (std::fabs(det)<=1e-12*scale)
			{
				// the ray runs along the triangle's plane: it only matters if it lies in it
				double nv[3];
				Cross(e1,e2,nv);
				if (std::fabs(Dot(tv,nv))<=1e-12*scale*std::sqrt(Dot(tv,tv)))
					return false;
				continue;
			}
			double inv=1.0/det;
			double u=Dot(tv,pv)*inv;
			if (u<-EDGE_MARGIN || u>1+EDGE_MARGIN)
				continue;
			Cross(tv,e1,qv);
			double v=Dot(d,qv)*inv;
			if (v<-EDGE_MARGIN || u+v>1+EDGE_MARGIN)
				continue;
			double t=Dot(e2,qv)*inv;
			if (t<0)
				continue;
			if (u<EDGE_MARGIN || v<EDGE_MARGIN || u+v>1-EDGE_MARGIN)
				return false; // too close to an edge or corner to tell
			++count;
			// det = -d.(e1 x e2): negative if the ray leaves through the front
			winding += (det<0) ? 1 : -1;
		}
	}
	return true;
}

/*********************CSPrimPolyhedron********************************************************************/
CSPrimPolyhedron::CSPrimPolyhedron(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimitives(ID,paraSet,prop), d_ptr(new CSPrimPolyhedronPrivate)
{
	Type = POLYHEDRON;
	PrimTypeName = "Polyhedron";
	m_InvalidFaces = 0;
}

CSPrimPolyhedron::CSPrimPolyhedron(CSPrimPolyhedron* primPolyhedron, CSProperties *prop) : CSPrimitives(primPolyhedron,prop), d_ptr(new CSPrimPolyhedronPrivate)
{
	Type = POLYHEDRON;
	PrimTypeName = "Polyhedron";
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
	d_ptr->Clear();
	m_InvalidFaces = 0;
}

void CSPrimPolyhedron::AddVertex(double px, double py, double pz)
{
	Invalidate();
	vertex nv;
	nv.coord[0]=px;nv.coord[1]=py;nv.coord[2]=pz;
	m_Vertices.push_back(nv);
}

double* CSPrimPolyhedron::GetVertex(unsigned int n)
{
	if (n<m_Vertices.size())
		return m_Vertices.at(n).coord;
	return NULL;
}

void CSPrimPolyhedron::AddFace(face f)
{
	Invalidate();
	m_Faces.push_back(f);
}

void CSPrimPolyhedron::AddFace(int numVertex, int* vertices)
{
	Invalidate();
	face f;
	f.numVertex=numVertex;
	f.vertices=new int[numVertex];
	for (int n=0;n<numVertex;++n)
		f.vertices[n]=vertices[n];
	m_Faces.push_back(f);
}

void CSPrimPolyhedron::AddFace(std::vector<int> vertices)
{
	Invalidate();
	face f;
	f.numVertex=vertices.size();
	f.vertices=new int[f.numVertex];
	for (unsigned int n=0;n<f.numVertex;++n)
		f.vertices[n]=vertices.at(n);
	m_Faces.push_back(f);
}

void CSPrimPolyhedron::Invalidate()
{
	if (m_Dimension<0)
		return;
	CSPrimitives::Invalidate();
	d_ptr->Clear();
}

bool CSPrimPolyhedron::BuildTree()
{
	Invalidate();
	m_InvalidFaces = 0;
	CSPrimPolyhedronPrivate& P = *d_ptr;

	// faces as triangles, a polygon as a fan from its first corner; faces of no area
	// and faces with points that do not exist are left out
	for (size_t f=0;f<m_Faces.size();++f)
	{
		face& fc = m_Faces.at(f);
		fc.valid = fc.numVertex>=3;
		for (unsigned int i=0;i<fc.numVertex && fc.valid;++i)
			fc.valid = fc.vertices[i]>=0 && (size_t)fc.vertices[i]<m_Vertices.size();
		if (!fc.valid)
		{
			++m_InvalidFaces;
			continue;
		}
		size_t before = P.tri.size();
		const double* a = m_Vertices.at(fc.vertices[0]).coord;
		for (unsigned int i=1;i+1<fc.numVertex;++i)
		{
			const double* b = m_Vertices.at(fc.vertices[i]).coord;
			const double* c = m_Vertices.at(fc.vertices[i+1]).coord;
			double ab[3], ac[3], n[3];
			Sub(b,a,ab); Sub(c,a,ac); Cross(ab,ac,n);
			if (n[0]==0 && n[1]==0 && n[2]==0)
				continue;
			P.tri.insert(P.tri.end(), a, a+3);
			P.tri.insert(P.tri.end(), b, b+3);
			P.tri.insert(P.tri.end(), c, c+3);
		}
		if (P.tri.size()==before)
		{
			fc.valid = false;
			++m_InvalidFaces;
		}
	}
	if (P.tri.empty())
	{
		m_Dimension = 0;
		m_BoundBoxValid = true;
		return true;
	}
	m_Dimension = 3;

	// the edges between faces, by the points' coordinates (points may be given twice):
	// closed if every edge has an even number of faces, oriented if they pass it
	// as often one way as the other
	std::vector<int> byCoord(m_Vertices.size());
	for (size_t n=0;n<byCoord.size();++n)
		byCoord[n] = (int)n;
	const std::vector<vertex>& V = m_Vertices;
	auto less = [&V](int a, int b) {return std::lexicographical_compare(V[a].coord, V[a].coord+3, V[b].coord, V[b].coord+3);};
	std::sort(byCoord.begin(), byCoord.end(), less);
	std::vector<int> id(m_Vertices.size());
	for (size_t n=0, next=0;n<byCoord.size();++n)
	{
		if (n>0 && less(byCoord[n-1], byCoord[n]))
			++next;
		id[byCoord[n]] = (int)next;
	}
	// every pass of a face along an edge: (smaller id, larger id, +1 from the smaller one, -1 back)
	struct Pass {int a, b, dir;};
	std::vector<Pass> passes;
	passes.reserve(3*m_Faces.size());
	for (size_t f=0;f<m_Faces.size();++f)
	{
		const face& fc = m_Faces.at(f);
		if (!fc.valid)
			continue;
		for (unsigned int i=0;i<fc.numVertex;++i)
		{
			int a = id[fc.vertices[i]], b = id[fc.vertices[(i+1)%fc.numVertex]];
			if (a!=b)
				passes.push_back(a<b ? Pass{a,b,1} : Pass{b,a,-1});
		}
	}
	std::sort(passes.begin(), passes.end(), [](const Pass& x, const Pass& y) {return x.a<y.a || (x.a==y.a && x.b<y.b);});
	P.closed = true;
	P.oriented = true;
	size_t open_edges = 0;
	for (size_t n=0;n<passes.size();)
	{
		size_t m = n;
		int uses = 0, dir = 0;
		for (;m<passes.size() && passes[m].a==passes[n].a && passes[m].b==passes[n].b;++m)
		{
			++uses;
			dir += passes[m].dir;
		}
		if (uses%2)
		{
			P.closed = false;
			++open_edges;
		}
		if (dir!=0)
			P.oriented = false;
		n = m;
	}
	if (!P.closed)
		std::cerr << "CSPrimPolyhedron::BuildTree: Warning, the surface is not closed (" << open_edges
				  << " open edges), the inside is decided by a majority of rays." << std::endl;
	else if (!P.oriented)
		std::cerr << "CSPrimPolyhedron::BuildTree: Warning, the faces are not consistently oriented; "
				  << "bodies inside each other or overlapping are holes there." << std::endl;

	P.Build();

	//update local bounding box
	GetBoundBox(m_BoundBox);
	double diag[3] = {m_BoundBox[1]-m_BoundBox[0], m_BoundBox[3]-m_BoundBox[2], m_BoundBox[5]-m_BoundBox[4]};
	P.size = std::sqrt(Dot(diag,diag));
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

	dBoundBox[0]=dBoundBox[1]=(double)m_Vertices.at(0).coord[0];
	dBoundBox[2]=dBoundBox[3]=(double)m_Vertices.at(0).coord[1];
	dBoundBox[4]=dBoundBox[5]=(double)m_Vertices.at(0).coord[2];

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

bool CSPrimPolyhedron::IsInside(const double* Coord, double tol)
{
	if (m_Dimension<0)
		this->Update();
	if (m_Dimension<3)
		return false;
	double pos[3];
	//transform incoming coordinates into cartesian coords
	TransformCoordSystem(Coord,pos,m_MeshType,CARTESIAN);
	if (m_Transform)
		m_Transform->InvertTransform(pos,pos);

	const CSPrimPolyhedronPrivate& P = *d_ptr;
	// the surface counts as inside; a little more than rounding errors always does
	tol = std::max(LocalTolerance(tol), 1e-12*P.size);
	for (unsigned int n=0;n<3;++n)
	{
		if ((m_BoundBox[2*n]-tol>pos[n]) || (m_BoundBox[2*n+1]+tol<pos[n])) return false;
	}
	if (P.Near(pos, tol))
		return true;

	// rays in several directions until one passes clear of edges and corners;
	// for an open surface the majority of three such rays
	int votes[2] = {0,0};
	int needed = P.closed ? 1 : 2;
	bool last = false;
	for (int r=0;r<NUM_RAYS;++r)
	{
		int count, winding;
		if (!P.Cast(pos, RAY_DIRS.d[r], count, winding))
			continue;
		last = P.oriented ? (winding!=0) : (count%2==1);
		if (++votes[last ? 1 : 0]>=needed)
			return last;
	}
	return last;
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
	stream << " Closed surface: " << (d_ptr->closed ? "yes" : "no") << ", consistently oriented: " << (d_ptr->oriented ? "yes" : "no") << std::endl;
}
