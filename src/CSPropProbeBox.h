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


//! Continuous Structure Probe Property for calculating integral properties
/*!
 CSProbProbeBox is a class for calculating integral properties such as (static) charges, voltages or currents.
*/
class CSXCAD_EXPORT CSPropProbeBox : public CSProperties
{
public:
	CSPropProbeBox(ParameterSet* paraSet);
	CSPropProbeBox(CSProperties* prop);
	CSPropProbeBox(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropProbeBox();

	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const std::string GetTypeXMLString() const {return std::string("ProbeBox");}

	//! Define a number for this probe property \sa GetNumber
	void SetNumber(unsigned int val);
	//! Get the number designated to this probe property \sa SetNumber
	unsigned int GetNumber();

	//! Get the normal direction of this probe box (required by some types of probes)
	int GetNormalDir() const {return m_NormDir;}
	//! Set the normal direction of this probe box (required by some types of probes)
	void SetNormalDir(unsigned int ndir) {m_NormDir=ndir;}

	//! Define/Set the probe weighting \sa GetWeighting
	void SetWeighting(double weight) {m_weight=weight;}
	//! Get the probe weighting \sa GetWeighting
	double GetWeighting() {return m_weight;}

	//! Define the probe type (e.g. type=0 for a charge integration, can/must be defined by the user interface) \sa GetProbeType
	void SetProbeType(int type) {ProbeType=type;}
	//! Get the probe type \sa SetProbeType
	int GetProbeType() {return ProbeType;}

	//! Set the probe start time
	void SetStartTime(float value) {startTime=value;}
	//! Get the probe start time
	double GetStartTime() {return startTime;}

	//! Set the probe stop time
	void SetStopTime(float value) {stopTime=value;}
	//! Get the probe stop time
	double GetStopTime() {return stopTime;}

	size_t CountFDSamples() {return m_FD_Samples.size();}
	std::vector<double> *GetFDSamples()	{return &m_FD_Samples;}
	void ClearFDSamples() {m_FD_Samples.clear();}
	void AddFDSample(double freq) {m_FD_Samples.push_back(freq);}
	void AddFDSample(std::vector<double> *freqs);
	void AddFDSample(std::string freqs);

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

protected:
	unsigned int uiNumber;
	int m_NormDir;
	double m_weight;
	int ProbeType;
	std::vector<double> m_FD_Samples;
	double startTime, stopTime;
};

