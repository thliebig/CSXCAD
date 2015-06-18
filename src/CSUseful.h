#ifndef CSUSEFUL_H
#define CSUSEFUL_H

class CSDebug;

#include "CSXCAD_Global.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

std::string CSXCAD_EXPORT ConvertInt(int number);
int CSXCAD_EXPORT String2Int(std::string number);
double CSXCAD_EXPORT String2Double(std::string number, bool &ok, int accurarcy=15);
std::vector<double> CSXCAD_EXPORT SplitString2Double(std::string str, const char delimiter);
std::vector<std::string> CSXCAD_EXPORT SplitString2Vector(std::string str, const char delimiter);
std::string CSXCAD_EXPORT CombineVector2String(std::vector<double> values, const char delimiter, int accurarcy=15);
std::string CSXCAD_EXPORT CombineArray2String(double* values, unsigned int numVal, const char delimiter, int accurarcy=15);
std::string CSXCAD_EXPORT CombineArray2String(float* values, unsigned int numVal, const char delimiter, int accurarcy=15);
std::string CSXCAD_EXPORT CombineArray2String(int* values, unsigned int numVal, const char delimiter, int accurarcy=15);

std::vector<int> CSXCAD_EXPORT SplitString2Int(std::string str, const char delimiter);

class CSXCAD_EXPORT CSDebug
{
public:
	CSDebug();

	void SetLevel(int level) {m_level=level;}

	void Debug(int on_level, const char* message);
	void Debug(int on_level, std::string message);

protected:
	int m_level;
};

static CSDebug g_CSDebug;

#endif // CSUSEFUL_H
