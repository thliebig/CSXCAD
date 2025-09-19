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

#include "ModeFileParser.h"

std::vector<double> ModeFileParser::parseCSVline(const std::string & line)
{
	std::vector<double>	row;
	std::stringstream	ss(line);
	std::string			cell;
	std::string			temp;
	double				val;

	while (std::getline(ss, cell, delimiter))
	{
		// If cell is empty, push a zero and move forward
		if (cell.empty())
		{
			row.push_back(0.0);
			continue;
		}

		// Remove quotes
		temp = cell;
		if(temp.front() == '"')
			temp = temp.substr(1);
		if(temp.back() == '"')
			temp = temp.substr(0, temp.length() - 1);

		std::stringstream cellConvert(cell);

		// Use string stream to type-cast the value
		cellConvert >> val;

		// Put value into line
		row.push_back(val);
	}

	return row;
}

bool ModeFileParser::parseFile()
{
	std::ifstream file(fileName);
	if (!file.is_open())
		return false;

	std::vector<std::vector<double>> csvData;

	// Container for current line size
	unsigned int cN = 0;
	bool firstLine = true;
	std::string line;
	while (std::getline(file, line))
	{
		std::vector<double> row = parseCSVline(line);
		// If this is the first line, store the line length
		if (firstLine)
		{
			firstLine = false;
			cN = row.size();
		}
		else
			// If this line is shorter, stop reading.
			if (row.size() != cN)
				break;

		// Store row
		csvData.push_back(row);
	}





	if ((csvData.size() <= 4) || (cN != 4))
		return false;

	std::vector<double> Xcol = getCSVcolumn(csvData,0),
						Ycol = getCSVcolumn(csvData,1);

	/****************************************
	* Arrange data in matrices (or vectors) *
	****************************************/
	// Copy the column vector
	m_X = Xcol;
	m_Y = Ycol;

	// First, make vectors of only unique X and Y values. No need to store meshgrid matrices
	std::sort(m_X.begin(),m_X.end());
	std::sort(m_Y.begin(),m_Y.end());


	// Mark unique elements
	std::vector<double>::iterator 	Xcol_last = std::unique(m_X.begin(),m_X.end()),
									Ycol_last = std::unique(m_Y.begin(),m_Y.end());

	// Erase all non-unique elements
	m_X.erase(Xcol_last,m_X.end());
	m_Y.erase(Ycol_last,m_Y.end());

	// Determine if CSV file is arranged by X values or by Y values:
	// If the dx between two subsequent elements is 0, iterate as rows
	bool Xprimary = (Xcol[1] - Xcol[0]) != 0;

	unsigned int Ncluster;
	if (Xprimary)
		Ncluster = m_X.size();
	else
		Ncluster = m_Y.size();

	// In case this is a re-read, clear the containers
	m_Vx.clear();
	m_Vy.clear();

	// Get data to snip from
	std::vector<double>	Vx = getCSVcolumn(csvData,2),
						Vy = getCSVcolumn(csvData,3);

	// Snip rows one by one
	for (unsigned int m = 0 ; m < Xcol.size() ; m += Ncluster)
	{
		std::vector<double> Vx_temp(Vx.begin() + m,Vx.begin() + m + Ncluster - 1),
							Vy_temp(Vy.begin() + m,Vy.begin() + m + Ncluster - 1);

		m_Vx.push_back(Vx_temp);
		m_Vy.push_back(Vy_temp);
	}

	// Matrix size
	m_M = m_Y.size();
	m_N = m_X.size();

	// In case this is Y primary, transpose
	if (!Xprimary)
	{

		std::vector<std::vector<double>> 	swap_Vx(m_N,std::vector<double>(m_M)),
											swap_Vy(m_N,std::vector<double>(m_M));


		for (unsigned int m = 0 ; m < m_N ; m++)
			for (unsigned int n = 0 ; n < m_M ; n++)
			{
				swap_Vx[m][n] = m_Vx[n][m];
				swap_Vy[m][n] = m_Vy[n][m];
			}

		m_Vx = swap_Vx;
		m_Vy = swap_Vy;
	}



	file.close();
	return true;
}

void ModeFileParser::clearData()
{
	m_X.clear();
	m_Y.clear();
	m_Vx.clear();
	m_Vy.clear();

	fileName = "";
}

std::vector<double> ModeFileParser::getCSVcolumn(const std::vector<std::vector<double>> & mat, unsigned int colIdx) const
{
	std::vector<double> column;
	if (mat.size() > 0)
		if (colIdx < mat.size())
		{
			std::vector<double> colVect;

			std::for_each(mat.begin(), mat.end(),
				[&](const std::vector<double> & row)
				{
					column.push_back(row[colIdx]);
				});

			return column;
		}

	return {};
}

std::vector<double> ModeFileParser::operator() (unsigned int i, unsigned int j) const
{
	std::vector<double> Vij = {0.0,0.0};

	// Check that the coordinate is within bounds.
	if (i >= m_M) return Vij;
	if (j >= m_N) return Vij;

	Vij[0] = m_Vx[i][j];
	Vij[1] = m_Vy[i][j];

	return Vij;
}

std::vector<double> ModeFileParser::getNearestNeighbor(double x, double y)
{

	// Nearest neighbor values found
	std::vector<double> Vnn = {0.0,0.0};

	unsigned int 	min_n = minDistArg(m_X,x),
					min_m = minDistArg(m_Y,y);

	// Store nearest neighbor
	Vnn[0] = m_Vx[min_m][min_n];
	Vnn[1] = m_Vy[min_m][min_n];

	return Vnn;
}
void ModeFileParser::getNearestNeighbor(double x, double y, double * const V)
{
	std::vector<double> Vnn = getNearestNeighbor(x,y);

	V[0] = Vnn[0];
	V[1] = Vnn[1];
}
void ModeFileParser::getNearestNeighbor(double x, double y, double & Vx, double & Vy)
{
	std::vector<double> Vnn = getNearestNeighbor(x,y);

	Vx = Vnn[0];
	Vy = Vnn[1];
}

std::vector<double> ModeFileParser::linInterp2(double x, double y)
{

	// Container for interpolated value
	std::vector<double> Vi2 = {0.0,0.0};

	unsigned int 	n1 = minDistArg(m_X, x),
					m1 = minDistArg(m_Y, y);

	// If the corner is past the coordinate, it was closer to the RHS. Shift back one.
	if (m_Y[m1] > y) m1--;
	if (m_X[n1] > x) n1--;

	// Check if this is the end of the matrix. If so,
	// clamp to the edge (nearest neighbor)
	if (x <= m_X[0])		x = m_X[0];
	if (x >= *m_X.end())	x = *m_X.end();
	if (y <= m_Y[0])		x = m_Y[0];
	if (y >= *m_Y.end())	x = *m_Y.end();

	std::function<double(const std::vector<std::vector<double>> &)> locInterp2 = [&](const std::vector<std::vector<double>> & A)
		{
			double 	X1 = m_X[n1],
					X2 = m_X[n1 + 1],
					Y1 = m_Y[m1],
					Y2 = m_Y[m1 + 1],
					f11 = A[m1][n1],
					f21 = A[m1][n1 + 1],
					f12 = A[m1 + 1][n1],
					f22 = A[m1 + 1][n1 + 1];

			double	D = (1/((X2 - X1)*(Y2 - Y1))),
					I11 = f11*(X2 - x)*(Y2 - y),
					I12 = f12*(X2 - x)*(y - Y1),
					I21 = f21*(x - X1)*(Y2 - y),
					I22 = f22*(x - X1)*(y - Y1);

			return D*(I11 + I12 + I21 + I22);
		};

	// finally, linear interpolation
	Vi2[0] = locInterp2(m_Vx);
	Vi2[1] = locInterp2(m_Vy);

	return Vi2;
}
void ModeFileParser::linInterp2(double x, double y, double * const V)
{
	std::vector<double> Vi2 = linInterp2(x,y);

	V[0] = Vi2[0];
	V[1] = Vi2[1];
}
void ModeFileParser::linInterp2(double x, double y, double & Vx, double & Vy)
{
	std::vector<double> Vi2 = linInterp2(x,y);

	Vx = Vi2[0];
	Vy = Vi2[1];
}

unsigned int ModeFileParser::minDistArg(std::vector<double> & A, double a0)
{
	unsigned int i;
	double minDist = std::numeric_limits<double>::infinity();

	for(i = 0 ; i < A.size() ; i++)
	{
		double cDist = std::abs(A[i] - a0);

		if(cDist < minDist)
			minDist = cDist;
		else
			break;

	}
	return i;
}

void ModeFileParser::print()
{
	std::cout << std::endl << "X = " << std::endl << std::endl;
	for (unsigned int m = 0 ; m < m_M ; m++)
	{
		for (std::vector<double>::const_iterator valIt = m_X.begin(); valIt != m_X.end(); ++valIt)
			std::cout << *valIt << "\t";

		std::cout << std::endl;
	}


	std::cout << std::endl << "Y = " << std::endl << std::endl;
	for (std::vector<double>::const_iterator valIt = m_Y.begin(); valIt != m_Y.end(); ++valIt)
	{
		for (unsigned int n = 0 ; n < m_N ; n++)
			std::cout << *valIt << "\t";

		std::cout << std::endl;
	}

	std::function<void(const std::vector<std::vector<double>> &)> printMat = [](const std::vector<std::vector<double>> & mat)
	{
		for (std::vector<std::vector<double>>::const_iterator rowIt = mat.begin(); rowIt != mat.end(); ++rowIt)
	    {
	        for (std::vector<double>::const_iterator valIt = rowIt->begin(); valIt != rowIt->end(); ++valIt)
	            std::cout << *valIt << "\t";

	        std::cout << std::endl;
	    }
	};
    std::cout << std::endl << "Vx = " << std::endl << std::endl;
    printMat(m_Vx);
    std::cout << std::endl << "Vy = " << std::endl << std::endl;
    printMat(m_Vy);

}
