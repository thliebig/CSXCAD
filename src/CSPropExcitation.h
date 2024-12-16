/*
*	Copyright (C) 2008-2012 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#pragma once

#include "CSProperties.h"

//! Continuous Structure Excitation Property
/*!
  This Property defines an excitation which can be location and direction dependent.
  */
class CSXCAD_EXPORT CSPropExcitation : public CSProperties
{
public:
	CSPropExcitation(ParameterSet* paraSet,unsigned int number=0);
	CSPropExcitation(CSProperties* prop);
	CSPropExcitation(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropExcitation();

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const std::string GetTypeXMLString() const {return std::string("Excitation");}

	//! Set the number for this excitation
	void SetNumber(unsigned int val);
	//! Get the number for this excitation
	unsigned int GetNumber();

	//! Set the excitation type
	void SetExcitType(int val);
	//! Get the excitation type
	int GetExcitType();

	//! Set the active direction for the source, only necessary with hard sources (by default all active)
	void SetActiveDir(bool active, int Component=0);
	//! Get the active direction for the source, only necessary with hard sources (by default all active)
	bool GetActiveDir(int Component=0);

	//! Set the excitation frequency
	void SetFrequency(double val) {m_Frequency.SetValue(val);}
	//! Set the excitation frequency
	void SetFrequency(const std::string val) {m_Frequency.SetValue(val);}
	//! Get the excitation frequency
	double GetFrequency()  {return m_Frequency.GetValue();}
	//! Get the excitation frequency as a string
	const std::string GetFrequencyString()  {return m_Frequency.GetValueString();}

	//! Set the excitation amplitude for a given component
	void SetExcitation(double val, int Component=0);
	//! Set the excitation amplitude for a given component
	void SetExcitation(const std::string val, int Component=0);
	//! Get the excitation amplitude for a given component
	double GetExcitation(int Component=0);
	//! Get the excitation amplitude as a string for a given component
	const std::string GetExcitationString(int Comp=0);

	//! Set a weighting factor for the given component. This will override the weighting function!
	void SetWeight(double val, int ny);
	//! Set a weighting function for the given excitation component
	int SetWeightFunction(const std::string fct, int ny);
	//! Get the weighting function for the given excitation component
	const std::string GetWeightFunction(int ny);

	double GetWeightedExcitation(int ny, const double* coords);

	//! Set the propagation direction for a given component
	void SetPropagationDir(double val, int Component=0);
	//! Set the propagation direction for a given component
	void SetPropagationDir(const std::string val, int Component=0);
	//! Get the propagation direction for a given component
	double GetPropagationDir(int Component=0);
	//! Get the propagation direction as a string for a given component
	const std::string GetPropagationDirString(int Comp=0);

	//! Set the excitation delay
	void SetDelay(double val);
	//! Set the excitation delay
	void SetDelay(const std::string val);
	//! Get the excitation delay
	double GetDelay();
	//! Get the excitation delay as a string
	const std::string GetDelayString();

	virtual void Init();
	virtual bool Update(std::string *ErrStr=NULL);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

	virtual void ShowPropertyStatus(std::ostream& stream);

protected:
	unsigned int uiNumber;
	int iExcitType;
	bool ActiveDir[3];
	ParameterScalar m_Frequency;
	ParameterScalar Excitation[3];		// excitation amplitude vector
	ParameterScalar WeightFct[3];		// excitation amplitude weighting function
	ParameterScalar PropagationDir[3];	// direction of propagation (should be a unit vector), needed for plane wave excitations
	ParameterScalar Delay;				// excitation delay only, for time-domain solver e.g. FDTD
};
