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
#include "CSPrimPolyhedron.h"

//! STL/PLY import primitive
class CSXCAD_EXPORT CSPrimPolyhedronReader : public CSPrimPolyhedron
{
public:
	//! Import file type
	enum FileType
	{
		UNKNOWN, STL_FILE, PLY_FILE
	};

	CSPrimPolyhedronReader(ParameterSet* paraSet, CSProperties* prop);
	CSPrimPolyhedronReader(CSPrimPolyhedronReader* primPHReader, CSProperties *prop=NULL);
	CSPrimPolyhedronReader(unsigned int ID, ParameterSet* paraSet, CSProperties* prop);
	virtual ~CSPrimPolyhedronReader();

	virtual CSPrimPolyhedronReader* GetCopy(CSProperties *prop=NULL) {return new CSPrimPolyhedronReader(this,prop);}

	virtual void SetFilename(std::string name) {m_filename=name;}
	virtual std::string GetFilename() const {return m_filename;}

	virtual void SetFileType(FileType ft) {m_filetype=ft;}
	virtual FileType GetFileType() const {return m_filetype;}

	virtual bool Update(std::string *ErrStr=NULL);
	virtual bool Write2XML(TiXmlElement &elem, bool parameterised=true);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual bool ReadFile();

protected:
	std::string m_filename;
	FileType m_filetype;
};
