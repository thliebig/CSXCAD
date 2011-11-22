#include "CSUseful.h"
#include "CSUseful.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>

std::vector<double> SplitString2Double(std::string str, const char delimiter)
{
	size_t pos=0;
	std::string sub;
	std::vector<double> values;
	do
	{
		pos=str.find_first_of(delimiter);
		if (pos==std::string::npos) pos=str.size();
		else ++pos;
		sub=str.substr(0,pos);
		//fprintf(stderr,"%d -> %s\n",pos,sub.c_str());
		if (sub.empty()==false) values.push_back(atof(sub.c_str()));
		str.erase(0,pos);
	} while (str.size()>0);
	return values;
}

std::vector<string> SplitString2Vector(std::string str, const char delimiter)
{
	size_t pos=0;
	std::string sub;
	std::vector<string> values;
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
		//fprintf(stderr,"%d -> %s\n",pos,sub.c_str());
		if (sub.empty()==false) values.push_back(sub);
		str.erase(0,pos);
	} while (str.size()>0);
	return values;
}


string CombineVector2String(vector<double> values, const char delimiter)
{
	stringstream ss;
	for (size_t i=0;i<values.size();++i)
	{
		if (i>0) ss << delimiter;
		ss<<values.at(i);
	}
	return ss.str();
}

string CombineArray2String(double* values, unsigned int numVal, const char delimiter)
{
	stringstream ss;
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
//		fprintf(stderr,"%d -> %s\n",(int)pos,sub.c_str());
		if (sub.empty()==false) values.push_back(atoi(sub.c_str()));
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
	Debug(on_level,string(message));
}

void CSDebug::Debug(int on_level, string message)
{
	if (on_level<=0) return; //no debug message for debug level smaller or equal zero
	if (on_level>=m_level)
		cerr << message << endl;
}

