#ifndef CSUSEFUL_H
#define CSUSEFUL_H

class CSDebug;

#include "CSXCAD_Global.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

using namespace std;

string CSXCAD_EXPORT ConvertInt(int number);
int CSXCAD_EXPORT String2Int(string number);
double CSXCAD_EXPORT String2Double(string number, int accurarcy=15);
vector<double> CSXCAD_EXPORT SplitString2Double(string str, const char delimiter);
vector<string> CSXCAD_EXPORT SplitString2Vector(string str, const char delimiter);
string CSXCAD_EXPORT CombineVector2String(vector<double> values, const char delimiter, int accurarcy=15);
string CSXCAD_EXPORT CombineArray2String(double* values, unsigned int numVal, const char delimiter, int accurarcy=15);
string CSXCAD_EXPORT CombineArray2String(float* values, unsigned int numVal, const char delimiter, int accurarcy=15);
string CSXCAD_EXPORT CombineArray2String(int* values, unsigned int numVal, const char delimiter, int accurarcy=15);

vector<int> CSXCAD_EXPORT SplitString2Int(string str, const char delimiter);

class CSXCAD_EXPORT CSDebug
{
public:
	CSDebug();

	void SetLevel(int level) {m_level=level;}

	void Debug(int on_level, const char* message);
	void Debug(int on_level, string message);

protected:
	int m_level;
};

static CSDebug g_CSDebug;

#endif // CSUSEFUL_H
