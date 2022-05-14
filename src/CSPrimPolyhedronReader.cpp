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

#include <vtkSTLReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>

#include "CSPrimPolyhedronReader.h"
#include "CSProperties.h"
#include "CSUseful.h"

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
	vtkPolyData *polydata = NULL;
	switch (m_filetype)
	{
	case STL_FILE:
	{
		vtkSTLReader* reader = vtkSTLReader::New();
		reader->SetFileName(m_filename.c_str());
		reader->SetMerging(1);
		polydata = reader->GetOutput(0);
		reader->Update();
		break;
	}
	case PLY_FILE:
	{
		vtkPLYReader* reader = vtkPLYReader::New();
		reader->SetFileName(m_filename.c_str());
		polydata = reader->GetOutput(0);
		reader->Update();
		break;
	}
	case UNKNOWN:
	default:
	{
		std::cerr << "CSPrimPolyhedronReader::ReadFile: unknown filetype, skipping..." << std::endl;
		return false;
		break;
	}
	}
	//polydata->Update(); // not availabe for vtk 6.x, now done only on reader?
	if ((polydata->GetNumberOfPoints()==0) || (polydata->GetNumberOfCells()==0))
	{
		std::cerr << "CSPrimPolyhedronReader::ReadFile: file invalid or empty, skipping ..." << std::endl;
		return false;
	}
	vtkCellArray *verts = polydata->GetPolys();
	if (verts->GetNumberOfCells()==0)
	{
		std::cerr << "CSPrimPolyhedronReader::ReadFile: file invalid or empty, skipping ..." << std::endl;
		return false;
	}

	for (int n=0;n<polydata->GetNumberOfPoints();++n)
		AddVertex(polydata->GetPoint(n));

	vtkIdType numP;
#if VTK_MAJOR_VERSION>=9
	const vtkIdType *vertices = nullptr;
#else
	vtkIdType *vertices = nullptr;
#endif
	verts->InitTraversal();
	while (verts->GetNextCell(numP, vertices))
	{
		face f;
		f.numVertex=numP;
		f.vertices = new int[f.numVertex];
		for (unsigned int np=0;np<f.numVertex;++np)
			f.vertices[np]=vertices[np];
		AddFace(f);
	}
	return true;
}
