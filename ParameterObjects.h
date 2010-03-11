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

#ifndef _PARAMETEROBJECTS_H_
#define _PARAMETEROBJECTS_H_
/*
 * Author:	Thorsten Liebig
 * Date:	03-12-2008
 * Lib:		CSXCAD
 * Version:	0.1a
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <math.h>
#include "CSXCAD_Global.h"
#include "fparser.hh"

using namespace std;

class Parameter;
class LinearParameter;
class ParameterSet;
class ParameterScalar;
class TiXmlNode;
class TiXmlElement;

bool ReadTerm(ParameterScalar &PS, TiXmlElement &elem, const char* attr);
void WriteTerm(ParameterScalar &PS, TiXmlElement &elem, const char* attr, bool mode, bool scientific=true);

class CSXCAD_EXPORT Parameter
{
public:
	Parameter();
	Parameter(const string Paraname, double val);
	Parameter(const Parameter* parameter) {sName=string(parameter->sName);dValue=parameter->dValue;bModified=true;Type=parameter->Type;bSweep=parameter->bSweep;};
	virtual ~Parameter();
	enum ParameterType
	{
		Const, Linear
	};
	ParameterType GetType() {return Type;};

	const string GetName() {return sName;};
	void SetName(const string Paraname) {sName=string(Paraname);bModified=true;};

	virtual double GetValue() {return dValue;};
	virtual void SetValue(double val) {dValue=val;bModified=true;};

	bool GetModified() {return bModified;};
	void SetModified(bool mod) {bModified=mod;};

	virtual bool GetSweep() {return false;};  ///const parameter can't sweep
	void SetSweep(bool mod) {bSweep=mod;};

	virtual void InitSweep() {};
	void Save() {dValueSaved=dValue;};
	void Restore() {dValue=dValueSaved;};

	virtual bool IncreaseStep() {return false;}; ///return false if no more sweep step available
	virtual int CountSteps() {return 1;};

	virtual void PrintSelf(FILE* out=stdout);

	virtual bool Write2XML(TiXmlNode& root);
	virtual bool ReadFromXML(TiXmlNode &root);
	Parameter* GetParameterFromXML(TiXmlNode &root);

	virtual Parameter* Clone() {return new Parameter(this);};

//	void ParameterSet(ParameterSet* set) {Set=set;};

	Parameter* ToConst() { return ( this && Type == Const ) ? this : 0; } /// Cast Parameter to a more defined type. Will return null if not of the requested type.
	LinearParameter* ToLinear() { return ( this && Type == Linear ) ? (LinearParameter*) this : 0; } /// Cast Parameter to a more defined type. Will return null if not of the requested type.

protected:
	string sName;
	double dValue;
	double dValueSaved;
	bool bModified;
	bool bSweep;
	ParameterType Type;
//	ParameterSet *Set;
};

class CSXCAD_EXPORT LinearParameter :  public Parameter
{
public:
	LinearParameter();
	LinearParameter(const string Paraname, double val, double min, double max, double step);
	//copy-constructor
	LinearParameter(const LinearParameter *parameter);
	virtual ~LinearParameter(void) {};

	virtual void SetValue(double val);

	virtual bool GetSweep() {return bSweep;};

	virtual void InitSweep() {dValue=dMin;};

	virtual bool IncreaseStep();
	virtual int CountSteps() {return (int)((dMax-dMin)/dStep)+1;};

	double GetMin() {return dMin;};
	void SetMin(double min) {dMin=min; if (dMax<dMin) dMax=dMin; SetValue(dValue);};

	double GetMax() {return dMax;};
	void SetMax(double max) {dMax=max; if (dMax<dMin) dMax=dMin; SetValue(dValue);};

	double GetStep() {return dStep;};
	void SetStep(double step) {dStep=step; if (dStep<0) dStep=0; SetValue(dValue);};

	void PrintSelf(FILE* out=stdout);

	virtual bool Write2XML(TiXmlNode& root);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual Parameter* Clone() {return new LinearParameter(this);};

protected:
	double dMin;
	double dMax;
	double dStep;
};


class CSXCAD_EXPORT ParameterSet
{
public:
	ParameterSet(void);
	virtual ~ParameterSet(void);

	virtual size_t InsertParameter(Parameter* newPara) {return LinkParameter(newPara->Clone());};
	virtual size_t LinkParameter(Parameter* newPara);
	virtual size_t DeleteParameter(size_t index);
	virtual size_t DeleteParameter(Parameter* para);
	Parameter* GetParameter(size_t index) {if (index<vParameter.size()) return vParameter.at(index); else return NULL;}

	bool GetModified();
	virtual void SetModified(bool mod=true);

	bool GetParaSetModified() {return bModified;};
	bool SetParaSetModified(bool val) {bModified=val;};

	const string GetParameterString(const string spacer=",");
	const string GetParameterValueString(const string spacer=",", bool ValuesOnly=false);

	size_t GetQtyParameter() {return vParameter.size();};
	double* GetValueArray(double *array);

	int CountSweepSteps(int SweepMode);  ///Modes: 1 sweep all; 2 sweep independently;
	void InitSweep();
	void EndSweep();
	bool NextSweepPos(int SweepMode);

	virtual void clear();

	void PrintSelf(FILE* out=stdout);
	bool Write2XML(TiXmlNode& root);
	bool ReadFromXML(TiXmlNode &root);

protected:
	vector<Parameter* > vParameter;
	string ParameterString;
	string ParameterValueString;
	bool bModified;
	int SweepPara;
};

void PSErrorCode2Msg(int code, string* msg);

class FunctionParser;

class CSXCAD_EXPORT ParameterScalar
{
public:
    enum EvaluateErrorType
    {
        NO_ERROR
    };
	ParameterScalar();
	ParameterScalar(ParameterSet* ParaSet, double value);
	ParameterScalar(ParameterSet* ParaSet, const string value);
	ParameterScalar(ParameterScalar* ps);
	~ParameterScalar();

	int SetParameterSet(ParameterSet *paraSet);

	int SetValue(const string value, bool Eval=true); ///returns eval-error-code
	void SetValue(double value);

	bool GetMode() {return ParameterMode;};
	const string GetString() {return sValue;}; //if (sValue.empty()) return NULL; else return sValue.c_str();};

	double GetValue();

	//returns error-code
	int Evaluate();

protected:
	ParameterSet* clParaSet;
	FunctionParser fParse;
	bool bModified;
	bool ParameterMode;
	string sValue;
	double dValue;
};

class CSXCAD_EXPORT ParameterVector
{
public:
	ParameterVector(ParameterSet* ParaSet);
	~ParameterVector();

	void SetX(double val) {XCoord.SetValue(val);};
	void SetX(const string val) {XCoord.SetValue(val);};
	void SetY(double val) {YCoord.SetValue(val);};
	void SetY(const string val) {YCoord.SetValue(val);};
	void SetZ(double val) {ZCoord.SetValue(val);};
	void SetZ(const string val) {ZCoord.SetValue(val);};
	void SetCoords(double x, double y, double z) {XCoord.SetValue(x);YCoord.SetValue(y);ZCoord.SetValue(z);};

	int Evaluate() {return XCoord.Evaluate()+YCoord.Evaluate()+ZCoord.Evaluate();};
	double GetX() {return XCoord.GetValue();};
	double GetY() {return YCoord.GetValue();};
	double GetZ() {return ZCoord.GetValue();};

protected:
	ParameterScalar XCoord,YCoord,ZCoord;
};

#endif
