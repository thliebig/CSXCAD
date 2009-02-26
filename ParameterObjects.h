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

using namespace std;

class Parameter;
class LinearParameter;
class ParameterSet;
class ParameterScalar;
class TiXmlNode;
class TiXmlElement;

bool ReadTerm(ParameterScalar &PS, TiXmlElement &elem, const char* attr);
void WriteTerm(ParameterScalar &PS, TiXmlElement &elem, const char* attr, bool mode);

class CSXCAD_EXPORT Parameter
{
public:
	Parameter();
	Parameter(const char* Paraname, double val);
	Parameter(const Parameter* parameter) {sName=string(parameter->sName);dValue=parameter->dValue;bModified=true;Type=parameter->Type;bSweep=parameter->bSweep;};
	virtual ~Parameter();
	enum ParameterType
	{
		Const, Linear
	};
	ParameterType GetType() {return Type;};

	const char* GetName() {return sName.c_str();};
	void SetName(const char* Paraname) {sName=string(Paraname);bModified=true;};

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
	LinearParameter(const char* Paraname, double val, double min, double max, double step);
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
	Parameter* GetParameter(size_t index) {if ((index>=0) && (index<vParameter.size())) return vParameter.at(index); else return NULL;}

	bool GetModified();
	virtual void SetModified(bool mod=true);

	const char* GetParameterString(const char* spacer=",");
	const char* GetParameterValueString(const char* spacer=",", bool ValuesOnly=false);

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

class CSXCAD_EXPORT ParameterScalar
{
public:
    enum EvaluateErrorType
    {
        NO_ERROR
    };
	ParameterScalar();
	ParameterScalar(ParameterSet* ParaSet, double value);
	ParameterScalar(ParameterSet* ParaSet, const char* value);
	ParameterScalar(ParameterScalar* ps);
	~ParameterScalar();

	void SetParameterSet(ParameterSet *paraSet) {clParaSet=paraSet;}

	int SetValue(const char* value, bool Eval=true); ///returns eval-error-code
	void SetValue(double value);

	bool GetMode() {return ParameterMode;};
	const char* GetString() {if (sValue.empty()) return NULL; else return sValue.c_str();};

	double GetValue();

	//returns error-code
	int Evaluate();

protected:
	ParameterSet* clParaSet;
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
	void SetX(const char* val) {XCoord.SetValue(val);};
	void SetY(double val) {YCoord.SetValue(val);};
	void SetY(const char* val) {YCoord.SetValue(val);};
	void SetZ(double val) {ZCoord.SetValue(val);};
	void SetZ(const char* val) {ZCoord.SetValue(val);};
	void SetCoords(double x, double y, double z) {XCoord.SetValue(x);YCoord.SetValue(y);ZCoord.SetValue(z);};

	int Evaluate() {return XCoord.Evaluate()+YCoord.Evaluate()+ZCoord.Evaluate();};
	double GetX() {return XCoord.GetValue();};
	double GetY() {return YCoord.GetValue();};
	double GetZ() {return ZCoord.GetValue();};

protected:
	ParameterScalar XCoord,YCoord,ZCoord;
};

#endif
