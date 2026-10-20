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
#include <fstream>
#include <iostream>
#include <limits>
#include <locale>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <unordered_map>
#include "tinyxml.h"
#include "stdint.h"

#include "CSPrimPolyhedronReader.h"
#include "CSProperties.h"
#include "CSUseful.h"

namespace {

//! Points and polygons read from a file
struct Mesh
{
	std::vector<double> points;  // x,y,z of every point
	std::vector<int> indices;    // the point indices of all polygons, one after the other
	std::vector<size_t> offsets; // start of each polygon in indices, plus the end
	Mesh() {offsets.push_back(0);}
	size_t NumPoints() const {return points.size()/3;}
	size_t NumPolys() const {return offsets.size()-1;}
	void AddPoint(double x, double y, double z) {points.push_back(x); points.push_back(y); points.push_back(z);}
	void ClosePoly() {offsets.push_back(indices.size());}
};

bool ReadWholeFile(const std::string& filename, std::string& data, std::string& err)
{
	std::ifstream file(filename.c_str(), std::ios::binary);
	if (!file)
	{
		err = "cannot open the file";
		return false;
	}
	std::ostringstream ss;
	ss << file.rdbuf();
	data = ss.str();
	return true;
}

std::string Lower(std::string s)
{
	for (size_t n=0;n<s.size();++n)
		s[n] = (char)std::tolower((unsigned char)s[n]);
	return s;
}

bool HostIsLittleEndian()
{
	const uint16_t one = 1;
	unsigned char b;
	memcpy(&b, &one, 1);
	return b==1;
}

//! copy size bytes from p into out, swapping the byte order if the file's differs from the host's
void ReadBytes(const char* p, size_t size, bool swap, void* out)
{
	if (!swap)
	{
		memcpy(out, p, size);
		return;
	}
	unsigned char* o = (unsigned char*)out;
	for (size_t n=0;n<size;++n)
		o[n] = (unsigned char)p[size-1-n];
}

/******************************************* STL *******************************************/

//! Merges points with exactly the same coordinates, as vtkSTLReader with merging did
class PointMerger
{
public:
	PointMerger(Mesh& mesh) : m_Mesh(mesh) {}
	int Insert(double x, double y, double z)
	{
		// -0 and 0 are the same point
		if (x==0) x=0;
		if (y==0) y=0;
		if (z==0) z=0;
		Key k;
		memcpy(k.c, &x, 8); memcpy(k.c+1, &y, 8); memcpy(k.c+2, &z, 8);
		auto it = m_Index.find(k);
		if (it!=m_Index.end())
			return it->second;
		int idx = (int)m_Mesh.NumPoints();
		m_Mesh.AddPoint(x, y, z);
		m_Index[k] = idx;
		return idx;
	}
private:
	struct Key
	{
		uint64_t c[3];
		bool operator==(const Key& o) const {return c[0]==o.c[0] && c[1]==o.c[1] && c[2]==o.c[2];}
	};
	struct Hash
	{
		size_t operator()(const Key& k) const
		{
			uint64_t h = k.c[0];
			h = h*0x9E3779B97F4A7C15ull ^ k.c[1];
			h = h*0x9E3779B97F4A7C15ull ^ k.c[2];
			return (size_t)(h ^ (h>>29));
		}
	};
	Mesh& m_Mesh;
	std::unordered_map<Key, int, Hash> m_Index;
};

//! Add a polygon of merged points, unless two of its corners are the same point
void AddSTLPolygon(Mesh& mesh, const std::vector<int>& corners)
{
	for (size_t i=0;i<corners.size();++i)
		for (size_t j=i+1;j<corners.size();++j)
			if (corners[i]==corners[j])
				return;
	mesh.indices.insert(mesh.indices.end(), corners.begin(), corners.end());
	mesh.ClosePoly();
}

bool ReadBinarySTL(const std::string& data, Mesh& mesh, std::string& err)
{
	uint32_t num;
	ReadBytes(data.data()+80, 4, !HostIsLittleEndian(), &num);
	PointMerger merger(mesh);
	std::vector<int> corners(3);
	const char* p = data.data()+84;
	const bool swap = !HostIsLittleEndian();
	for (uint32_t n=0;n<num;++n, p+=50)
	{
		for (int c=0;c<3;++c)
		{
			float v[3];
			for (int i=0;i<3;++i)
				ReadBytes(p+12+12*c+4*i, 4, swap, &v[i]); // after the normal
			corners[c] = merger.Insert(v[0], v[1], v[2]);
		}
		AddSTLPolygon(mesh, corners);
	}
	if (mesh.NumPolys()==0)
		err = "no triangles";
	return mesh.NumPolys()>0;
}

bool ReadAsciiSTL(std::istream& in, Mesh& mesh, std::string& err)
{
	in.imbue(std::locale::classic());
	PointMerger merger(mesh);
	std::vector<int> corners;
	std::string token;
	if (!(in >> token) || Lower(token)!="solid")
	{
		err = "not an STL file, neither binary nor starting with \"solid\"";
		return false;
	}
	std::getline(in, token); // the solid's name
	unsigned int facet = 0;
	bool inFacet = false;
	while (in >> token)
	{
		token = Lower(token);
		if (token=="vertex")
		{
			double v[3];
			if (!inFacet || !(in >> v[0] >> v[1] >> v[2]))
			{
				err = "facet " + std::to_string(facet) + ": " + (inFacet ? "invalid vertex coordinates" : "vertex outside of a facet");
				return false;
			}
			corners.push_back(merger.Insert(v[0], v[1], v[2]));
		}
		else if (token=="facet")
		{
			double nv[3];
			if (inFacet)
			{
				err = "facet " + std::to_string(facet) + ": no endfacet";
				return false;
			}
			++facet;
			inFacet = true;
			corners.clear();
			if (!(in >> token) || Lower(token)!="normal" || !(in >> nv[0] >> nv[1] >> nv[2]))
			{
				err = "facet " + std::to_string(facet) + ": invalid normal";
				return false;
			}
		}
		else if (token=="outer")
		{
			if (!(in >> token) || Lower(token)!="loop")
			{
				err = "facet " + std::to_string(facet) + ": \"outer\" without \"loop\"";
				return false;
			}
		}
		else if (token=="endloop")
			continue;
		else if (token=="endfacet")
		{
			if (!inFacet || corners.size()<3)
			{
				err = "facet " + std::to_string(facet) + ": fewer than three vertices";
				return false;
			}
			AddSTLPolygon(mesh, corners);
			inFacet = false;
		}
		else if (token=="endsolid")
			std::getline(in, token); // the solid's name
		else if (token=="solid")
			std::getline(in, token);
		else
		{
			err = "facet " + std::to_string(facet) + ": unexpected \"" + token + "\"";
			return false;
		}
	}
	if (inFacet)
	{
		err = "facet " + std::to_string(facet) + ": the file ends inside it";
		return false;
	}
	if (mesh.NumPolys()==0)
		err = "no facets";
	return mesh.NumPolys()>0;
}

bool ReadSTL(const std::string& filename, Mesh& mesh, std::string& err)
{
	std::ifstream file(filename.c_str(), std::ios::binary);
	if (!file)
	{
		err = "cannot open the file";
		return false;
	}
	file.seekg(0, std::ios::end);
	uint64_t size = (uint64_t)file.tellg();
	file.seekg(0);
	// binary: an 80 byte header, the triangle count, 50 bytes per triangle;
	// the header may start with "solid" like an ASCII file, the size tells them apart
	if (size>=84)
	{
		char head[84];
		file.read(head, 84);
		uint32_t num;
		ReadBytes(head+80, 4, !HostIsLittleEndian(), &num);
		if (size==84+50*(uint64_t)num)
		{
			std::string data;
			file.close();
			return ReadWholeFile(filename, data, err) && ReadBinarySTL(data, mesh, err);
		}
		std::istringstream in(std::string(head, 80));
		std::string first;
		if (!(in >> first) || Lower(first)!="solid")
		{
			err = "not an STL file, or a binary STL of " + std::to_string(size) + " bytes, which does not match its "
				+ std::to_string(num) + " triangles (" + std::to_string(84+50*(uint64_t)num) + " bytes)";
			return false;
		}
		file.seekg(0);
	}
	return ReadAsciiSTL(file, mesh, err);
}

/******************************************* PLY *******************************************/

enum PLYType {PLY_INVALID, PLY_INT8, PLY_UINT8, PLY_INT16, PLY_UINT16, PLY_INT32, PLY_UINT32, PLY_FLOAT32, PLY_FLOAT64};

PLYType PLYTypeByName(const std::string& name)
{
	if (name=="char"   || name=="int8")    return PLY_INT8;
	if (name=="uchar"  || name=="uint8")   return PLY_UINT8;
	if (name=="short"  || name=="int16")   return PLY_INT16;
	if (name=="ushort" || name=="uint16")  return PLY_UINT16;
	if (name=="int"    || name=="int32")   return PLY_INT32;
	if (name=="uint"   || name=="uint32")  return PLY_UINT32;
	if (name=="float"  || name=="float32") return PLY_FLOAT32;
	if (name=="double" || name=="float64") return PLY_FLOAT64;
	return PLY_INVALID;
}

size_t PLYTypeSize(PLYType t)
{
	switch (t)
	{
	case PLY_INT8: case PLY_UINT8: return 1;
	case PLY_INT16: case PLY_UINT16: return 2;
	case PLY_INT32: case PLY_UINT32: case PLY_FLOAT32: return 4;
	case PLY_FLOAT64: return 8;
	default: return 0;
	}
}

struct PLYProperty
{
	std::string name;
	PLYType type;      // of the value, or of the list items
	PLYType countType; // of the list count, PLY_INVALID if no list
};

struct PLYElement
{
	std::string name;
	size_t count;
	std::vector<PLYProperty> props;
};

//! Reads the values of a PLY body, ASCII or binary
class PLYValueReader
{
public:
	PLYValueReader(const std::string& data, size_t start, int format) : m_Data(data), m_Pos(start), m_Format(format), m_Ascii(data.substr(format==0 ? start : data.size()))
	{
		m_Ascii.imbue(std::locale::classic());
		m_Swap = (format==1) != HostIsLittleEndian();
	}
	//! the next value of type t, false at the end of the data or on an invalid number
	bool Read(PLYType t, double& val)
	{
		if (m_Format==0)
			return (bool)(m_Ascii >> val);
		size_t size = PLYTypeSize(t);
		if (m_Pos+size>m_Data.size())
			return false;
		const char* p = m_Data.data()+m_Pos;
		m_Pos += size;
		switch (t)
		{
		case PLY_INT8:    {int8_t v;   ReadBytes(p,1,false,&v);  val=v; break;}
		case PLY_UINT8:   {uint8_t v;  ReadBytes(p,1,false,&v);  val=v; break;}
		case PLY_INT16:   {int16_t v;  ReadBytes(p,2,m_Swap,&v); val=v; break;}
		case PLY_UINT16:  {uint16_t v; ReadBytes(p,2,m_Swap,&v); val=v; break;}
		case PLY_INT32:   {int32_t v;  ReadBytes(p,4,m_Swap,&v); val=v; break;}
		case PLY_UINT32:  {uint32_t v; ReadBytes(p,4,m_Swap,&v); val=v; break;}
		case PLY_FLOAT32: {float v;    ReadBytes(p,4,m_Swap,&v); val=v; break;}
		case PLY_FLOAT64: {double v;   ReadBytes(p,8,m_Swap,&v); val=v; break;}
		default: return false;
		}
		return true;
	}
private:
	const std::string& m_Data;
	size_t m_Pos;
	int m_Format; // 0: ascii, 1: binary little endian, 2: binary big endian
	bool m_Swap;
	std::istringstream m_Ascii;
};

bool ReadPLY(const std::string& filename, Mesh& mesh, std::string& err)
{
	std::string data;
	if (!ReadWholeFile(filename, data, err))
		return false;

	// the header, line by line up to end_header
	std::vector<PLYElement> elements;
	int format = -1;
	size_t pos = 0;
	bool first = true, done = false;
	while (!done)
	{
		size_t eol = data.find('\n', pos);
		if (eol==std::string::npos)
		{
			err = first ? "not a PLY file" : "the header has no end_header";
			return false;
		}
		std::string line = data.substr(pos, eol-pos);
		pos = eol+1;
		if (!line.empty() && line[line.size()-1]=='\r')
			line.erase(line.size()-1);
		std::istringstream ls(line);
		std::string key;
		ls >> key;
		if (first)
		{
			if (key!="ply")
			{
				err = "not a PLY file, it does not start with \"ply\"";
				return false;
			}
			first = false;
			continue;
		}
		if (key=="format")
		{
			std::string f;
			ls >> f;
			if (f=="ascii") format = 0;
			else if (f=="binary_little_endian") format = 1;
			else if (f=="binary_big_endian") format = 2;
			else
			{
				err = "unknown format \"" + f + "\", not ascii, binary_little_endian or binary_big_endian";
				return false;
			}
		}
		else if (key=="element")
		{
			PLYElement e;
			if (!(ls >> e.name >> e.count))
			{
				err = "invalid header line \"" + line + "\"";
				return false;
			}
			elements.push_back(e);
		}
		else if (key=="property")
		{
			PLYProperty p;
			std::string t;
			ls >> t;
			p.countType = PLY_INVALID;
			if (t=="list")
			{
				std::string ct;
				ls >> ct >> t;
				p.countType = PLYTypeByName(ct);
				if (p.countType==PLY_INVALID || p.countType==PLY_FLOAT32 || p.countType==PLY_FLOAT64)
				{
					err = "invalid list count type in \"" + line + "\"";
					return false;
				}
			}
			p.type = PLYTypeByName(t);
			if (!(ls >> p.name) || p.type==PLY_INVALID)
			{
				err = "invalid property \"" + line + "\"; the types: char, uchar, short, ushort, int, uint, float, double (or int8 ... float64)";
				return false;
			}
			if (elements.empty())
			{
				err = "property before any element";
				return false;
			}
			elements.back().props.push_back(p);
		}
		else if (key=="end_header")
			done = true;
		else if (key!="comment" && key!="obj_info" && !key.empty())
		{
			err = "unknown header line \"" + line + "\"";
			return false;
		}
	}
	if (format<0)
	{
		err = "the header has no format";
		return false;
	}

	// the body, element by element
	PLYValueReader reader(data, pos, format);
	bool haveVertex = false, haveFace = false;
	size_t numVertex = 0;
	for (size_t e=0;e<elements.size();++e)
	{
		const PLYElement& el = elements[e];
		int xyz[3] = {-1,-1,-1};
		int faceList = -1;
		if (el.name=="vertex")
		{
			for (size_t p=0;p<el.props.size();++p)
				for (int i=0;i<3;++i)
					if (el.props[p].countType==PLY_INVALID && el.props[p].name==std::string(1,(char)('x'+i)))
						xyz[i] = (int)p;
			if (xyz[0]<0 || xyz[1]<0 || xyz[2]<0)
			{
				err = "the vertex element has no x, y and z";
				return false;
			}
			haveVertex = true;
			numVertex = el.count;
			mesh.points.reserve(3*el.count);
		}
		else if (el.name=="face")
		{
			for (size_t p=0;p<el.props.size() && faceList<0;++p)
				if (el.props[p].countType!=PLY_INVALID && (el.props[p].name=="vertex_indices" || el.props[p].name=="vertex_index"))
					faceList = (int)p;
			if (faceList<0)
			{
				err = "the face element has no list vertex_indices";
				return false;
			}
			haveFace = true;
		}
		for (size_t n=0;n<el.count;++n)
		{
			double coord[3] = {0,0,0};
			for (size_t p=0;p<el.props.size();++p)
			{
				const PLYProperty& prop = el.props[p];
				double val;
				if (prop.countType==PLY_INVALID)
				{
					if (!reader.Read(prop.type, val))
					{
						err = el.name + " " + std::to_string(n) + ": the file ends early or has an invalid number";
						return false;
					}
					for (int i=0;i<3;++i)
						if ((int)p==xyz[i])
							coord[i] = val;
					continue;
				}
				double cnt;
				if (!reader.Read(prop.countType, cnt) || cnt<0 || cnt!=(double)(size_t)cnt)
				{
					err = el.name + " " + std::to_string(n) + ": the file ends early or has an invalid list";
					return false;
				}
				for (size_t i=0;i<(size_t)cnt;++i)
				{
					if (!reader.Read(prop.type, val))
					{
						err = el.name + " " + std::to_string(n) + ": the file ends early or has an invalid number";
						return false;
					}
					if ((int)p==faceList)
						mesh.indices.push_back((int)val);
				}
				if ((int)p==faceList)
					mesh.ClosePoly();
			}
			if (xyz[0]>=0)
				mesh.AddPoint(coord[0], coord[1], coord[2]);
		}
	}
	if (!haveVertex || !haveFace || numVertex==0 || mesh.NumPolys()==0)
	{
		err = "no vertices or no faces";
		return false;
	}
	for (size_t f=0;f<mesh.NumPolys();++f)
	{
		if (mesh.offsets[f+1]-mesh.offsets[f]<3)
		{
			err = "face " + std::to_string(f) + ": fewer than three vertices";
			return false;
		}
		for (size_t i=mesh.offsets[f];i<mesh.offsets[f+1];++i)
			if (mesh.indices[i]<0 || (size_t)mesh.indices[i]>=numVertex)
			{
				err = "face " + std::to_string(f) + ": vertex index " + std::to_string(mesh.indices[i]) + " out of range, there are " + std::to_string(numVertex) + " vertices";
				return false;
			}
	}
	return true;
}

}

CSPrimPolyhedronReader::CSPrimPolyhedronReader(ParameterSet* paraSet, CSProperties* prop): CSPrimPolyhedron(paraSet,prop)
{
	Type = POLYHEDRONREADER;
	PrimTypeName = "PolyhedronReader";
	m_filetype = UNKNOWN;
	m_filename = std::string();
}

CSPrimPolyhedronReader::CSPrimPolyhedronReader(CSPrimPolyhedronReader* primPHReader, CSProperties *prop) : CSPrimPolyhedron(primPHReader, prop)
{
	Type = POLYHEDRONREADER;
	PrimTypeName = "PolyhedronReader";

	m_filename = primPHReader->m_filename;
	m_filetype = primPHReader->m_filetype;
}

CSPrimPolyhedronReader::CSPrimPolyhedronReader(unsigned int ID, ParameterSet* paraSet, CSProperties* prop) : CSPrimPolyhedron(ID, paraSet, prop)
{
	Type = POLYHEDRONREADER;
	PrimTypeName = "PolyhedronReader";
	m_filetype = UNKNOWN;
	m_filename = std::string();
}

CSPrimPolyhedronReader::~CSPrimPolyhedronReader()
{
}

bool CSPrimPolyhedronReader::Update(std::string *ErrStr)
{
	return CSPrimPolyhedron::Update(ErrStr);
}

bool CSPrimPolyhedronReader::Write2XML(TiXmlElement &elem, bool parameterised)
{
	elem.SetAttribute("FileName",m_filename);

	switch (m_filetype)
	{
	case STL_FILE:
		elem.SetAttribute("FileType","STL");
		break;
	case PLY_FILE:
		elem.SetAttribute("FileType","PLY");
		break;
	default:
		elem.SetAttribute("FileType","Unkown");
		break;
	}
	return CSPrimitives::Write2XML(elem,parameterised);
}

bool CSPrimPolyhedronReader::ReadFromXML(TiXmlNode &root)
{
	if (!CSPrimitives::ReadFromXML(root)) return false;

	TiXmlElement* elem=root.ToElement();
	if (elem==NULL) return false;
	if (elem->QueryStringAttribute("FileName",&m_filename)!=TIXML_SUCCESS)
	{
		std::cerr << "CSPrimPolyhedronReader::ReadFromXML: Error, can't read filename!" << std::endl;
		return false;
	}
	std::string type;
	if (elem->QueryStringAttribute("FileType",&type)!=TIXML_SUCCESS)
	{
		std::cerr << "CSPrimPolyhedronReader::ReadFromXML: Error, can't read file type!" << std::endl;
		return false;
	}
	if (type.compare("STL")==0)
		m_filetype=STL_FILE;
	else if (type.compare("PLY")==0)
		m_filetype=PLY_FILE;
	else
		m_filetype=UNKNOWN;

	if (ReadFile()==false)
	{
		std::cerr << "CSPrimPolyhedronReader::ReadFromXML: Failed to read file." << std::endl;
		return false;
	}

	return BuildTree();
}

bool CSPrimPolyhedronReader::ReadFile()
{
	Mesh mesh;
	std::string err;
	bool ok = false;
	switch (m_filetype)
	{
	case STL_FILE:
		ok = ReadSTL(m_filename, mesh, err);
		break;
	case PLY_FILE:
		ok = ReadPLY(m_filename, mesh, err);
		break;
	case UNKNOWN:
	default:
		std::cerr << "CSPrimPolyhedronReader::ReadFile: unknown filetype, skipping..." << std::endl;
		return false;
	}
	if (!ok)
	{
		std::cerr << "CSPrimPolyhedronReader::ReadFile: " << m_filename << ": " << err << ", skipping ..." << std::endl;
		return false;
	}

	Reset();
	for (size_t n=0;n<mesh.NumPoints();++n)
		AddVertex(&mesh.points[3*n]);
	for (size_t n=0;n<mesh.NumPolys();++n)
	{
		face f;
		f.numVertex = mesh.offsets[n+1]-mesh.offsets[n];
		f.vertices = new int[f.numVertex];
		f.valid = false;
		std::copy(mesh.indices.begin()+mesh.offsets[n], mesh.indices.begin()+mesh.offsets[n+1], f.vertices);
		AddFace(f);
	}
	return true;
}
