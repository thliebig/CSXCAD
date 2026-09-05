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

#include "CSRectGrid.h"
#include "CSUseful.h"
#include "tinyxml.h"
#include "CSFunctionParser.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

CSRectGrid::CSRectGrid(void)
{
	dDeltaUnit=1;
	m_meshType = CARTESIAN;
}

CSRectGrid::~CSRectGrid(void)
{
}

void CSRectGrid::LineVector::EnsureSorted() const
{
	if (m_Dirty==false) return;
	std::vector<double>::iterator start = m_Lines.begin();
	std::vector<double>::iterator end = m_Lines.end();
	sort(start,end);
	end=unique(start,end);
	m_Lines.erase(end,m_Lines.end());
	m_Dirty=false;
}

void CSRectGrid::LineVector::Add(double val)
{
	if (m_Dirty==false)
	{
		// mesh lines usually arrive in increasing order, appending keeps the
		// invariant and avoids sorting the whole direction again
		if (m_Lines.empty() || (val>m_Lines.back()))
		{
			m_Lines.push_back(val);
			return;
		}
		if (val==m_Lines.back())
			return;
		m_Dirty=true;
	}
	m_Lines.push_back(val);
}

bool CSRectGrid::LineVector::Set(size_t idx, double val)
{
	EnsureSorted();
	if (idx>=m_Lines.size()) return false;
	m_Lines.at(idx)=val;
	// the new value may be anywhere, defer sorting to the next read
	m_Dirty=true;
	return true;
}

bool CSRectGrid::LineVector::RemoveAt(size_t idx)
{
	EnsureSorted();
	if (idx>=m_Lines.size()) return false;
	m_Lines.erase(m_Lines.begin()+idx);
	return true;
}

bool CSRectGrid::LineVector::RemoveValue(double val)
{
	EnsureSorted();
	std::vector<double>::iterator it = lower_bound(m_Lines.begin(),m_Lines.end(),val);
	if ((it==m_Lines.end()) || (*it!=val)) return false;
	m_Lines.erase(it);
	return true;
}

double CSRectGrid::LineVector::At(size_t idx) const
{
	EnsureSorted();
	if (idx>=m_Lines.size()) return 0;
	return m_Lines.at(idx);
}

CSRectGrid* CSRectGrid::Clone(CSRectGrid* original)
{
	CSRectGrid* clone = new CSRectGrid();
	clone->dDeltaUnit = original->dDeltaUnit;
	for (int i=0;i<3;++i)
		clone->Lines[i] = original->Lines[i];
	for (int i=0;i<6;++i)
		clone->SimBox[i] = original->SimBox[i];
	return clone;
}

void CSRectGrid::AddDiscLine(int direct, double val)
{
	if ((direct>=0)&&(direct<3)) Lines[direct].Add(val);
}

void CSRectGrid::AddDiscLines(int direct, int numLines, double* vals)
{
	for (int n=0;n<numLines;++n)
	{
		AddDiscLine(direct,vals[n]);
	}
}

std::string CSRectGrid::AddDiscLines(int direct, int numLines, double* vals, std::string DistFunction)
{
	if ((direct<0)||(direct>=3)) return std::string("Unknown grid direction!");
	if (DistFunction.empty()==false)
	{
		CSFunctionParser fParse;
		std::string dirVar;
		switch (direct)
		{
		case 0:
			dirVar = "x";
			break;
		case 1:
			dirVar = "y";
			break;
		case 2:
			dirVar = "z";
			break;
		}
		fParse.Parse(DistFunction,dirVar);
		if (fParse.GetParseErrorType()!=FunctionParser::FP_NO_ERROR)
			return std::string("An error occured parsing f(") + dirVar + std::string(") - Parser message:\n") + std::string(fParse.ErrorMsg());

		double dValue=0;
		bool error=false;
		for (int n=0;n<numLines;++n)
		{
			dValue=fParse.Eval(&vals[n]);
			if (fParse.EvalError()!=0) error=true;
			AddDiscLine(direct,dValue);
		}
		if (error) return std::string("An error occured evaluation the grid function f(") + dirVar + std::string(")!");
	}
	return "";
}

bool CSRectGrid::RemoveDiscLine(int direct, int index)
{
	if ((direct<0) || (direct>=3)) return false;
	if (index<0) return false;
	return Lines[direct].RemoveAt((size_t)index);
}

bool CSRectGrid::RemoveDiscLine(int direct, double val)
{
	if ((direct<0) || (direct>=3)) return false;
	return Lines[direct].RemoveValue(val);
}

void CSRectGrid::clear()
{
	Lines[0].Clear();
	Lines[1].Clear();
	Lines[2].Clear();
	dDeltaUnit=1;
}

void CSRectGrid::ClearLines(int direct)
{
	if ((direct<0) || (direct>=3)) return;
	Lines[direct].Clear();
}

void CSRectGrid::SetDeltaUnit(double val) {dDeltaUnit=val;}

double CSRectGrid::GetDeltaUnit() {return dDeltaUnit;}

bool CSRectGrid::SetLine(int direct, size_t Index, double value)
{
	if ((direct<0) || (direct>=3)) return false;
	return Lines[direct].Set(Index,value);
}

double CSRectGrid::GetLine(int direct, size_t Index)
{
	if ((direct<0) || (direct>=3)) return 0;
	return Lines[direct].At(Index);
}

double* CSRectGrid::GetLines(int direct, double *array, unsigned int &qty, bool sorted)
{
	UNUSED(sorted); // the lines are always sorted
	if ((direct<0) || (direct>=3)) return 0;
	const std::vector<double>& lines = Lines[direct].Get();
	delete[] array;
	array = new double[lines.size()];
	for (size_t i=0;i<lines.size();++i) array[i]=lines.at(i);
	qty=lines.size();
	return array;
}

size_t CSRectGrid::GetQtyLines(int direct)
{
	if ((direct>=0) && (direct<3))
	return Lines[direct].Size();
	else return 0;
}

std::string CSRectGrid::GetLinesAsString(int direct)
{
	std::stringstream xStr;
	if ((direct<0)||(direct>=3)) return xStr.str();
	const std::vector<double>& lines = Lines[direct].Get();
	for (size_t i=0;i<lines.size();++i)
	{
		if (i>0) xStr << ", ";
		xStr<<lines.at(i);
	}
	return xStr.str();
}

unsigned int CSRectGrid::Snap2LineNumber(int ny, double value, bool &inside) const
{
	inside = false;
	if ((ny<0) || (ny>2))
		return -1;
	const std::vector<double>& lines = Lines[ny].Get();
	if (lines.size()==0)
		return -1;
	if (value<lines.at(0))
		return 0;
	if (value>lines.at(lines.size()-1))
		return lines.size()-1;
	inside = true;
	for (size_t n=0;n<lines.size()-1;++n)
	{
		if (value < 0.5*(lines.at(n)+lines.at(n+1)) )
			return n;
	}
	return lines.size()-1;
}

int CSRectGrid::GetDimension()
{
	if (Lines[0].Size()==0) return -1;
	if (Lines[1].Size()==0) return -1;
	if (Lines[2].Size()==0) return -1;
	int dim=0;
	if (Lines[0].Size()>1) ++dim;
	if (Lines[1].Size()>1) ++dim;
	if (Lines[2].Size()>1) ++dim;
	return dim;
}

void CSRectGrid::SetMeshType(CoordinateSystem type) {m_meshType=type;}

CoordinateSystem CSRectGrid::GetMeshType() {return m_meshType;}

void CSRectGrid::IncreaseResolution(int nu, int factor)
{
	if ((nu<0) || (nu>=GetDimension())) return;
	if ((factor<=1) || (factor>9)) return;
	// work on a copy, adding lines invalidates the reference into the storage
	const std::vector<double> lines = Lines[nu].Get();
	for (size_t i=0;i+1<lines.size();++i)
	{
		double delta=(lines.at(i+1)-lines.at(i))/factor;
		for (int n=1;n<factor;++n)
		{
			AddDiscLine(nu,lines.at(i)+n*delta);
		}
	}
}


void CSRectGrid::Sort(int direct)
{
	// kept for compatibility only, any read access sorts by itself
	if ((direct<0) || (direct>=3)) return;
	Lines[direct].Get();
}

double* CSRectGrid::GetSimArea()
{
	for (int i=0;i<3;++i)
	{
		const std::vector<double>& lines = Lines[i].Get();
		if (lines.size()!=0)
		{
			SimBox[2*i]=lines.front();
			SimBox[2*i+1]=lines.back();
		}
		else SimBox[2*i]=SimBox[2*i+1]=0;
	}
	return SimBox;
}

bool CSRectGrid::isValid()
{
	for (int n=0;n<3;++n)
		if (GetQtyLines(n)<2)
			return false;
	return true;
}


bool CSRectGrid::Write2XML(TiXmlNode &root, bool sorted)
{
	UNUSED(sorted); // the lines are always sorted
	TiXmlElement grid("RectilinearGrid");

	grid.SetDoubleAttribute("DeltaUnit",dDeltaUnit);
	grid.SetAttribute("CoordSystem",(int)this->GetMeshType());

	TiXmlElement XLines("XLines");
	XLines.SetAttribute("Qty",(int)Lines[0].Size());
	if (Lines[0].Size()>0)
	{
		TiXmlText XText(CombineVector2String(Lines[0].Get(),','));
		XLines.InsertEndChild(XText);
	}
	grid.InsertEndChild(XLines);

	TiXmlElement YLines("YLines");
	YLines.SetAttribute("Qty",(int)Lines[1].Size());
	if (Lines[1].Size()>0)
	{
		TiXmlText YText(CombineVector2String(Lines[1].Get(),','));
		YLines.InsertEndChild(YText);
	}
	grid.InsertEndChild(YLines);

	TiXmlElement ZLines("ZLines");
	ZLines.SetAttribute("Qty",(int)Lines[2].Size());
	if (Lines[2].Size()>0)
	{
		TiXmlText ZText(CombineVector2String(Lines[2].Get(),','));
		ZLines.InsertEndChild(ZText);
	}
	grid.InsertEndChild(ZLines);

	root.InsertEndChild(grid);

	return true;
}

bool CSRectGrid::ReadFromXML(TiXmlNode &root)
{
	TiXmlElement* Lines=root.ToElement();
	if (Lines->QueryDoubleAttribute("DeltaUnit",&dDeltaUnit)!=TIXML_SUCCESS) dDeltaUnit=1.0;

	int help;
	if (Lines->QueryIntAttribute("CoordSystem",&help)==TIXML_SUCCESS)
		SetMeshType((CoordinateSystem)help);

	TiXmlNode* FN=NULL;
	TiXmlText* Text=NULL;
	std::string LineStr[3];

	Lines = root.FirstChildElement("XLines");
	if (Lines==NULL) return false;
	FN = Lines->FirstChild();
	if (FN!=NULL)
	{
		Text = FN->ToText();
		if (Text!=NULL)	LineStr[0]=std::string(Text->Value());
	}

	Lines = root.FirstChildElement("YLines");
	if (Lines==NULL) return false;
	FN = Lines->FirstChild();
	if (FN!=NULL)
	{
		Text = FN->ToText();
		if (Text!=NULL)	LineStr[1]=std::string(Text->Value());
	}

	Lines = root.FirstChildElement("ZLines");
	if (Lines==NULL) return false;
	FN = Lines->FirstChild();
	if (FN!=NULL)
	{
		Text = FN->ToText();
		if (Text!=NULL)	LineStr[2]=std::string(Text->Value());
	}

	for (int i=0;i<3;++i)
	{
		std::vector<double> lines = SplitString2Double(LineStr[i],',');
		for (size_t n=0;n<lines.size();++n)
			AddDiscLine(i,lines.at(n));
		Sort(i);
	}

	return true;
}
