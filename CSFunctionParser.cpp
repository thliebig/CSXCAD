#include "CSFunctionParser.h"
#include <math.h>
#include <iostream>

double bessel_first_kind_0(const double* p)
{
	return j0(p[0]);
}
double bessel_first_kind_1(const double* p)
{
	return j1(p[0]);
}
double bessel_first_kind_n(const double* p)
{
	int n=p[0];
	if (n<0)
	{
		std::cerr << "CSFunctionParser::bessel_first_kind_n (jn): first argument must be integer larger than zero! found: " << p[0] << std::endl;
		return 0;
	}
	return jn(n,p[1]);
}

double bessel_second_kind_0(const double* p)
{
	return y0(p[0]);
}
double bessel_second_kind_1(const double* p)
{
	return y1(p[0]);
}
double bessel_second_kind_n(const double* p)
{
	int n=p[0];
	if (n<0)
	{
		std::cerr << "CSFunctionParser::bessel_second_kind_n (yn): first argument must be integer larger than zero! found: " << p[0] << std::endl;
		return 0;
	}
	return yn(n,p[1]);
}


CSFunctionParser::CSFunctionParser()
{
	//some usefull constants
	AddConstant("pi", 3.14159265358979323846);
	AddConstant("e", 2.71828182845904523536);

	//some bessel functions of first kind
	AddFunction("j0",bessel_first_kind_0,1);
	AddFunction("j1",bessel_first_kind_1,1);
	AddFunction("jn",bessel_first_kind_n,2);

	//some bessel functions of second kind
	AddFunction("y0",bessel_second_kind_0,1);
	AddFunction("y1",bessel_second_kind_1,1);
	AddFunction("yn",bessel_second_kind_n,2);
}
