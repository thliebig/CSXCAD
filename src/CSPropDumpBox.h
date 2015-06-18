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

#include "CSPropProbeBox.h"


//! Continuous Structure Dump Property
/*!
  This Property defines an area (box) designated for field dumps.
  */
class CSXCAD_EXPORT CSPropDumpBox : public CSPropProbeBox
{
public:
	CSPropDumpBox(ParameterSet* paraSet);
	CSPropDumpBox(CSProperties* prop);
	CSPropDumpBox(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropDumpBox();

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const std::string GetTypeXMLString() const {return std::string("DumpBox");}

	//! Define an arbitrary dump-type \sa GetDumpType
	void SetDumpType(int type) {DumpType=type;}
	//! Get the arbitrary dump-type \sa SetDumpType
	int GetDumpType() {return DumpType;}

	//! Define an arbitrary dump-mode \sa GetDumpMode
	void SetDumpMode(int mode) {DumpMode=mode;}
	//! Get the arbitrary dump-mode \sa SetDumpMode
	int GetDumpMode() {return DumpMode;}

	//! Define an arbitrary file-type \sa GetFileType
	void SetFileType(int ftype) {FileType=ftype;}
	//! Get the arbitrary file-type \sa SetFileType
	int GetFileType() {return FileType;}

	//! Set the multi grid level to use (default is 0) \sa GetMultiGridLevel
	void SetMultiGridLevel(int mgl) {MultiGridLevel=mgl;}
	//! Get the multi grid level to use \sa SetMultiGridLevel
	int GetMultiGridLevel() {return MultiGridLevel;}

	bool GetSubSampling() {return m_SubSampling;}
	void SetSubSampling(bool val) {m_SubSampling=val;}
	void SetSubSampling(int ny, unsigned int val);
	void SetSubSampling(unsigned int val[]);
	void SetSubSampling(const char* vals);
	unsigned int GetSubSampling(int ny);

	//! Get status of opt resolution flag
	bool GetOptResolution() {return m_OptResolution;}
	//! Set status of opt resolution flag
	void SetOptResolution(bool val) {m_OptResolution=val;}
	//! Set the opt resoultion for a given direction
	void SetOptResolution(int ny, double val);
	//! Set the opt resoultion for all directions
	void SetOptResolution(double val[]);
	//! Set the opt resoultion for all directions as string
	void SetOptResolution(const char* vals);
	//! Get the optimal resolution for a given direction
	double GetOptResolution(int ny);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void Init();

	virtual void ShowPropertyStatus(std::ostream& stream);

protected:
	int DumpType;
	int DumpMode;
	int FileType;
	int MultiGridLevel;

	//sub-sampling
	bool m_SubSampling;
	unsigned int SubSampling[3];

	//sub-sampling
	bool m_OptResolution;
	double OptResolution[3];
};
