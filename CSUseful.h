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

vector<double> SplitString2Double(string str, const char delimiter);

vector<int> SplitString2Int(string str, const char delimiter);

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
