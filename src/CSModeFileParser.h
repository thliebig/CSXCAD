/*
*	Copyright (C) 2023-2025 Gadi Lahav (gadi@rfwithcare.com)
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

#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <limits>
#include <algorithm>
#include <cmath>
#include <functional>

#include "CSXCAD_Global.h"

class CSXCAD_EXPORT CSModeFileParser
{
public:
	CSModeFileParser(char delimiter = ',') : m_delimiter(delimiter) {ClearData();};
	CSModeFileParser(const std::string & fileName, char delimiter = ',') : m_fileName(fileName) , m_delimiter(delimiter) {ParseFile();};

	bool ParseFile(const std::string & fileName) {this->m_fileName = fileName; return this->ParseFile();};
	bool ParseFile();
	bool IsFileParsed() {return (m_Vx.size() != 0);};

	void ClearData();

	std::vector<double> NearestNeighbor(double x, double y);
	void NearestNeighbor(double x, double y, double * const  V);
	void NearestNeighbor(double x, double y, double & Vx, double & Vy);

	std::vector<double> LinInterp2(double x, double y);
	void LinInterp2(double x, double y, double * const V);
	void LinInterp2(double x, double y, double & Vx, double & Vy);

	// See if mode data was parsed
	bool HasModeData() {return (m_Vx.size() != 0);};

	// Directly obtain the array elements
	std::vector<double> operator()(unsigned int i, unsigned int j) const;

	// for debug
	void Print();

private:
	std::string m_fileName;
	char 		m_delimiter;

	// Containers for arranged data
	std::vector<double>					m_X,m_Y;
	std::vector<std::vector<double>>	m_Vx,m_Vy;

	unsigned int 	m_M = 0,
					m_N = 0;

	// Helper functions to parse original CSV
	std::vector<double> ParseCSVline(const std::string & line);
	std::vector<double> GetCSVcolumn(const std::vector<std::vector<double>> & mat, unsigned int colIdx) const;

	// This function assumes that the input vector is sorted, for efficiency
	unsigned int MinDistArg(std::vector<double> & x, double x0);
	unsigned int MinDistArgLHS(std::vector<double> & x, double x0);

	// Numbers of rows and columns
	unsigned int GetM() const {return m_M;};
	unsigned int GetN() const {return m_N;};


};
