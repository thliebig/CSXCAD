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

//#include "CSProperties.h"

class ModeFileParser
{
public:
	ModeFileParser(char delimiter = ',') : delimiter(delimiter) {clearData();};
	ModeFileParser(const std::string & fileName, char delimiter = ',') : fileName(fileName) , delimiter(delimiter) {clearData(); parseFile();};

	bool parseFile(const std::string & fileName) {this->fileName = fileName; return this->parseFile();};
	bool parseFile();
	bool isFileParsed() {return (m_Vx.size() != 0);};

	void clearData();

	std::vector<double> operator()(unsigned int i, unsigned int j) const;

	std::vector<double> getNearestNeighbor(double x, double y);
	void getNearestNeighbor(double x, double y, double * const  V);
	void getNearestNeighbor(double x, double y, double & Vx, double & Vy);

	std::vector<double> linInterp2(double x, double y);
	void linInterp2(double x, double y, double * const V);
	void linInterp2(double x, double y, double & Vx, double & Vy);

	unsigned int getM() const {return m_M;};
	unsigned int getN() const {return m_N;};

	// See if mode data was parsed
	bool hasModeData() {return (m_Vx.size() != 0);};

	// for debug
	void print();

private:
	std::string fileName;
	char 		delimiter;

	// Containers for arranged data
	std::vector<double>					m_X,m_Y;
	std::vector<std::vector<double>>	m_Vx,m_Vy;

	unsigned int 	m_M = 0,
					m_N = 0;

	// Helper functions to parse original CSV
	std::vector<double> parseCSVline(const std::string & line);
	std::vector<double> getCSVcolumn(const std::vector<std::vector<double>> & mat, unsigned int colIdx) const;

	// This function assumes that the input vector is sorted, for efficiency
	unsigned int minDistArg(std::vector<double> & A, double a0);

};
