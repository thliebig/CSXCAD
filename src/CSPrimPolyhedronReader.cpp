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
}

CSPrimPolyhedronReader::~CSPrimPolyhedronReader()
{
}

bool CSPrimPolyhedronReader::Update(string *ErrStr)
{
	return CSPrimPolyhedron::Update(ErrStr);
}

bool CSPrimPolyhedronReader::Write2XML(TiXmlElement &elem, bool parameterised)
{
	elem.SetAttribute("Filename",m_filename);

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
		cerr << "CSPrimPolyhedronReader::ReadFromXML: Error, can't read filename!" << endl;
		return false;
	}
	string type;
	if (elem->QueryStringAttribute("FileType",&type)!=TIXML_SUCCESS)
	{
		cerr << "CSPrimPolyhedronReader::ReadFromXML: Error, can't read file type!" << endl;
		return false;
	}
	if (type.compare("STL")==0)
		m_filetype=STL_FILE;
	if (type.compare("PLY")==0)
		m_filetype=PLY_FILE;
	else
		m_filetype=UNKNOWN;

	if (ReadFile(m_filename)==false)
	{
		cerr << "CSPrimPolyhedronReader::ReadFromXML: Failed to read file." << endl;
		return false;
	}

	return BuildTree();
}

bool CSPrimPolyhedronReader::ReadFile(string filename)
{
	vtkPolyData *polydata = NULL;
	switch (m_filetype)
	{
	case STL_FILE:
	{
		vtkSTLReader* reader = vtkSTLReader::New();
		reader->SetFileName(filename.c_str());
		reader->SetMerging(1);
		polydata = reader->GetOutput(0);
		break;
	}
	case PLY_FILE:
	{
		vtkPLYReader* reader = vtkPLYReader::New();
		reader->SetFileName(filename.c_str());
		polydata = reader->GetOutput(0);
		break;
	}
	case UNKNOWN:
	default:
		cerr << "CSPrimPolyhedronReader::ReadFile: unknown filetype, skipping..." << endl;
		return false;
		break;
	}
	polydata->Update();
	if ((polydata->GetNumberOfPoints()==0) || (polydata->GetNumberOfCells()==0))
	{
		cerr << "CSPrimPolyhedronReader::ReadFile: file invalid or empty, skipping ..." << endl;
		return false;
	}
	vtkCellArray *verts = polydata->GetPolys();
	if (verts->GetNumberOfCells()==0)
	{
		cerr << "CSPrimPolyhedronReader::ReadFile: file invalid or empty, skipping ..." << endl;
		return false;
	}

	for (unsigned int n=0;n<polydata->GetNumberOfPoints();++n)
		AddVertex(polydata->GetPoint(n));

	vtkIdType numP;
	vtkIdType *vertices = new vtkIdType[10];
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
