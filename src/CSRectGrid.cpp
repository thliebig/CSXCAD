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
	if ((direct>=0)&&(direct<3)) Lines[direct].push_back(val);
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
	if ((index>=(int)Lines[direct].size()) || (index<0)) return false;
	std::vector<double>::iterator vIter=Lines[direct].begin();
	Lines[direct].erase(vIter+index);
	return true;
}

bool CSRectGrid::RemoveDiscLine(int direct, double val)
{
	if ((direct<0) || (direct>=3)) return false;
	for (size_t i=0;i<Lines[direct].size();++i)
	{
		if (Lines[direct].at(i)==val) return RemoveDiscLine(direct,(int)i);
	}
	return false;
}

void CSRectGrid::clear()
{
	Lines[0].clear();
	Lines[1].clear();
	Lines[2].clear();
	dDeltaUnit=1;
}

void CSRectGrid::ClearLines(int direct)
{
	if ((direct<0) || (direct>=3)) return;
	Lines[direct].clear();
}

void CSRectGrid::SetDeltaUnit(double val) {dDeltaUnit=val;}

double CSRectGrid::GetDeltaUnit() {return dDeltaUnit;}

bool CSRectGrid::SetLine(int direct, size_t Index, double value)
{
	if ((direct<0) || (direct>=3)) return false;
	if (Lines[direct].size()<=Index) return false;
	Lines[direct].at(Index) = value;
	return true;
}

double CSRectGrid::GetLine(int direct, size_t Index)
{
	if ((direct<0) || (direct>=3)) return 0;
	if (Lines[direct].size()<=Index) return 0;
	return Lines[direct].at(Index);
}

double* CSRectGrid::GetLines(int direct, double *array, unsigned int &qty, bool sorted)
{
	if ((direct<0) || (direct>=3)) return 0;
	if (sorted) Sort(direct);
	delete[] array;
	array = new double[Lines[direct].size()];
	for (size_t i=0;i<Lines[direct].size();++i) array[i]=Lines[direct].at(i);
	qty=Lines[direct].size();
	return array;
}

size_t CSRectGrid::GetQtyLines(int direct)
{
	if ((direct>=0) && (direct<3))
	return Lines[direct].size();
	else return 0;
}

std::string CSRectGrid::GetLinesAsString(int direct)
{
	std::stringstream xStr;
	if ((direct<0)||(direct>=3)) return xStr.str();
	if (Lines[direct].size()>0)
	{
		for (size_t i=0;i<Lines[direct].size();++i)
		{
			if (i>0) xStr << ", ";
			xStr<<Lines[direct].at(i);
		}
	}
	return xStr.str();
}

unsigned int CSRectGrid::Snap2LineNumber(int ny, double value, bool &inside) const
{
	inside = false;
	if ((ny<0) || (ny>2))
		return -1;
	if (Lines[ny].size()==0)
		return -1;
	if (value<Lines[ny].at(0))
		return 0;
	if (value>Lines[ny].at(Lines[ny].size()-1))
		return Lines[ny].size()-1;
	inside = true;
	for (size_t n=0;n<Lines[ny].size()-1;++n)
	{
		if (value < 0.5*(Lines[ny].at(n)+Lines[ny].at(n+1)) )
			return n;
	}
	return Lines[ny].size()-1;
}

int CSRectGrid::GetDimension()
{
	if (Lines[0].size()==0) return -1;
	if (Lines[1].size()==0) return -1;
	if (Lines[2].size()==0) return -1;
	int dim=0;
	if (Lines[0].size()>1) ++dim;
	if (Lines[1].size()>1) ++dim;
	if (Lines[2].size()>1) ++dim;
	return dim;
}

void CSRectGrid::SetMeshType(CoordinateSystem type) {m_meshType=type;}

CoordinateSystem CSRectGrid::GetMeshType() {return m_meshType;}

void CSRectGrid::IncreaseResolution(int nu, int factor)
{
	if ((nu<0) || (nu>=GetDimension())) return;
	if ((factor<=1) && (factor>9)) return;
	size_t size=Lines[nu].size();
	for (size_t i=0;i<size-1;++i)
	{
		double delta=(Lines[nu].at(i+1)-Lines[nu].at(i))/factor;
		for (int n=1;n<factor;++n)
		{
			AddDiscLine(nu,Lines[nu].at(i)+n*delta);
		}
	}
	Sort(nu);
}


void CSRectGrid::Sort(int direct)
{
	if ((direct<0) || (direct>=3)) return;
	std::vector<double>::iterator start = Lines[direct].begin();
	std::vector<double>::iterator end = Lines[direct].end();
	sort(start,end);
	end=unique(start,end);
	Lines[direct].erase(end,Lines[direct].end());
}

double* CSRectGrid::GetSimArea()
{
	for (int i=0;i<3;++i)
	{
		if (Lines[i].size()!=0)
		{
			SimBox[2*i]=*min_element(Lines[i].begin(),Lines[i].end());
			SimBox[2*i+1]=*max_element(Lines[i].begin(),Lines[i].end());
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
	if (sorted) {Sort(0);Sort(1);Sort(2);}
	TiXmlElement grid("RectilinearGrid");

	grid.SetDoubleAttribute("DeltaUnit",dDeltaUnit);
	grid.SetAttribute("CoordSystem",(int)this->GetMeshType());

	TiXmlElement XLines("XLines");
	XLines.SetAttribute("Qty",(int)Lines[0].size());
	if (Lines[0].size()>0)
	{
		TiXmlText XText(CombineVector2String(Lines[0],','));
		XLines.InsertEndChild(XText);
	}
	grid.InsertEndChild(XLines);

	TiXmlElement YLines("YLines");
	YLines.SetAttribute("Qty",(int)Lines[1].size());
	if (Lines[1].size()>0)
	{
		TiXmlText YText(CombineVector2String(Lines[1],','));
		YLines.InsertEndChild(YText);
	}
	grid.InsertEndChild(YLines);

	TiXmlElement ZLines("ZLines");
	ZLines.SetAttribute("Qty",(int)Lines[2].size());
	if (Lines[2].size()>0)
	{
		TiXmlText ZText(CombineVector2String(Lines[2],','));
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
