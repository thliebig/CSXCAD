/*
*	Copyright (C) 2011,2012 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#include "CSUseful.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>

std::string ConvertInt(int number)
{
   std::stringstream ss;
   ss << number;
   return ss.str();
}

int String2Int(std::string number)
{
	int val;
	std::stringstream ss(number);
	ss >> val;
	return val;
}

double String2Double(std::string number, bool &ok, int accurarcy)
{
	double val;
	std::stringstream ss(number);
	ss.precision(accurarcy);
	ss >> val;
	ok = ss.eof() && !ss.fail();
	return val;
}

std::vector<double> SplitString2Double(std::string str, const char delimiter)
{
	size_t pos=0;
	std::string sub;
	std::vector<double> values;
	bool ok;
	double val;
	do
	{
		pos=str.find_first_of(delimiter);
		if (pos==std::string::npos) pos=str.size();
		sub=str.substr(0,pos);
		if (sub.empty()==false)
		{
			val = String2Double(sub, ok);
			if (ok)
				values.push_back(val);
		}
		str.erase(0,pos+1);
	} while (str.size()>0);
	return values;
}

std::vector<std::string> SplitString2Vector(std::string str, const char delimiter)
{
	size_t pos=0;
	std::string sub;
	std::vector<std::string> values;
	do
	{
		pos=str.find_first_of(delimiter);
		if (pos==std::string::npos)
		{
			pos=str.size();
			sub=str.substr(0,pos);
		}
		else
		{
			sub=str.substr(0,pos);
			++pos;
		}
		if (sub.empty()==false) values.push_back(sub);
		str.erase(0,pos);
	} while (str.size()>0);
	return values;
}


std::string CombineVector2String(std::vector<double> values, const char delimiter, int accurarcy)
{
	std::stringstream ss;
	ss.precision( accurarcy );
	for (size_t i=0;i<values.size();++i)
	{
		if (i>0) ss << delimiter;
		ss<<values.at(i);
	}
	return ss.str();
}

std::string CombineArray2String(double* values, unsigned int numVal, const char delimiter, int accurarcy)
{
	std::stringstream ss;
	ss.precision( accurarcy );
	for (unsigned int i=0;i<numVal;++i)
	{
		if (i>0) ss << delimiter;
		ss<<values[i];
	}
	return ss.str();
}

std::string CombineArray2String(float* values, unsigned int numVal, const char delimiter, int accurarcy)
{
	std::stringstream ss;
	ss.precision( accurarcy );
	for (unsigned int i=0;i<numVal;++i)
	{
		if (i>0) ss << delimiter;
		ss<<values[i];
	}
	return ss.str();
}

std::string CombineArray2String(int* values, unsigned int numVal, const char delimiter, int accurarcy)
{
	std::stringstream ss;
	ss.precision( accurarcy );
	for (unsigned int i=0;i<numVal;++i)
	{
		if (i>0) ss << delimiter;
		ss<<values[i];
	}
	return ss.str();
}

std::vector<int> SplitString2Int(std::string str, const char delimiter)
{
	size_t pos=0;
	std::string sub;
	std::vector<int> values;
	do
	{
		pos=str.find_first_of(delimiter);
		if (pos==std::string::npos) pos=str.size();
		else ++pos;
		sub=str.substr(0,pos);
		if (sub.empty()==false) values.push_back(String2Int(sub));
		str.erase(0,pos);
	} while (str.size()>0);
	return values;
}

CSDebug::CSDebug()
{
	m_level = 0;
}

void CSDebug::Debug(int on_level, const char* message)
{
	Debug(on_level,std::string(message));
}

void CSDebug::Debug(int on_level, std::string message)
{
	if (on_level<=0) return; //no debug message for debug level smaller or equal zero
	if (on_level>=m_level)
		std::cerr << message << std::endl;
}

