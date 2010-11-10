#ifndef CSFUNCTIONPARSER_H
#define CSFUNCTIONPARSER_H

#include "CSXCAD_Global.h"
#include "fparser.hh"

//! Extended FunctionParser using some additional constants (pi,e) and functions
/*!
 This is an extended FunctionParser with the additional constants, "pi" and "e", as well as severel bessel functions of first (j0, j1, jn) and second kind (y0, y1, yn).
*/
class CSXCAD_EXPORT CSFunctionParser : public FunctionParser
{
public:
    CSFunctionParser();
};

#endif // CSFUNCTIONPARSER_H
