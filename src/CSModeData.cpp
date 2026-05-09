/*
*	Copyright (C) 2023-2025 Gadi Lahav (gadi@rfwithcare.com)
*	Copyright (C) 2026 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#include <algorithm>
#include <iostream>

#include <hdf5.h>
#include <hdf5_hl.h>

#include "CSModeData.h"

namespace {

// Read a 1-D double dataset directly into a pre-allocated vector.
// Returns false if the dataset is missing, not 1-D, or shorter than 2 points.
bool ReadVec(hid_t fid, const char* name, std::vector<double>& out)
{
	if (H5Lexists(fid, name, H5P_DEFAULT) <= 0)
		return false;

	int rank = 0;
	if (H5LTget_dataset_ndims(fid, name, &rank) < 0 || rank != 1)
		return false;

	hsize_t dims[1] = {0};
	if (H5LTget_dataset_info(fid, name, dims, nullptr, nullptr) < 0 || dims[0] < 2)
		return false;

	out.resize(dims[0]);
	return H5LTread_dataset_double(fid, name, out.data()) >= 0;
}

// Read a 2-D double dataset (shape nx × ny) directly into a flat vector.
// Returns false if missing, not 2-D, or shape does not match expected_nx × expected_ny.
bool ReadMat(hid_t fid, const char* name, std::vector<double>& out,
             size_t expected_nx, size_t expected_ny)
{
	if (H5Lexists(fid, name, H5P_DEFAULT) <= 0)
		return false;

	int rank = 0;
	if (H5LTget_dataset_ndims(fid, name, &rank) < 0 || rank != 2)
		return false;

	hsize_t dims[2] = {0, 0};
	if (H5LTget_dataset_info(fid, name, dims, nullptr, nullptr) < 0)
		return false;

	if (dims[0] != expected_nx || dims[1] != expected_ny)
	{
		std::cerr << "CSModeData: dataset '" << name << "' has shape ("
		          << dims[0] << ", " << dims[1] << "), expected ("
		          << expected_nx << ", " << expected_ny << ")\n";
		return false;
	}

	out.resize(expected_nx * expected_ny);
	return H5LTread_dataset_double(fid, name, out.data()) >= 0;
}

} // namespace

bool CSModeData::ReadFromHDF5(const std::string& filename)
{
	Clear();

	hid_t fid = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
	if (fid < 0)
	{
		std::cerr << "CSModeData: failed to open '" << filename << "'\n";
		return false;
	}

	double version = 0.0;
	H5LTget_attribute_double(fid, "/", "Version", &version);
	if (version < 1.0)
	{
		std::cerr << "CSModeData: unsupported or missing Version attribute in '" << filename << "'\n";
		H5Fclose(fid);
		return false;
	}

	if (!ReadVec(fid, "x", m_X) || !ReadVec(fid, "y", m_Y))
	{
		std::cerr << "CSModeData: missing or invalid 'x' or 'y' dataset in '" << filename << "'\n";
		H5Fclose(fid);
		return false;
	}
	m_nx = m_X.size();
	m_ny = m_Y.size();

	if (!ReadMat(fid, "Vx", m_Vx, m_nx, m_ny) || !ReadMat(fid, "Vy", m_Vy, m_nx, m_ny))
	{
		std::cerr << "CSModeData: missing or invalid 'Vx' or 'Vy' dataset in '" << filename << "'\n";
		H5Fclose(fid);
		Clear();
		return false;
	}

	H5Fclose(fid);
	return true;
}

void CSModeData::Clear()
{
	m_X.clear();
	m_Y.clear();
	m_Vx.clear();
	m_Vy.clear();
	m_nx = 0;
	m_ny = 0;
}

std::array<double, 2> CSModeData::LinInterp2(double x, double y) const
{
	// Clamp to grid extent
	x = std::max(m_X.front(), std::min(m_X.back(), x));
	y = std::max(m_Y.front(), std::min(m_Y.back(), y));

	// Left-edge x index (first dimension)
	auto it = std::lower_bound(m_X.begin(), m_X.end(), x);
	if (it != m_X.begin()) --it;
	size_t i = std::min((size_t)(it - m_X.begin()), m_nx - 2);

	// Left-edge y index (second dimension)
	auto jt = std::lower_bound(m_Y.begin(), m_Y.end(), y);
	if (jt != m_Y.begin()) --jt;
	size_t j = std::min((size_t)(jt - m_Y.begin()), m_ny - 2);

	const double x1 = m_X[i],     x2 = m_X[i + 1];
	const double y1 = m_Y[j],     y2 = m_Y[j + 1];
	const double D  = 1.0 / ((x2 - x1) * (y2 - y1));

	auto interp = [&](const std::vector<double>& A) -> double
	{
		// A[i, j] = A[i * m_ny + j]  where i=x-index, j=y-index
		const double f11 = A[ i      * m_ny + j    ];  // (x[i],   y[j]  )
		const double f21 = A[(i + 1) * m_ny + j    ];  // (x[i+1], y[j]  )
		const double f12 = A[ i      * m_ny + j + 1];  // (x[i],   y[j+1])
		const double f22 = A[(i + 1) * m_ny + j + 1];  // (x[i+1], y[j+1])
		return D * (  f11 * (x2 - x) * (y2 - y)
		            + f21 * (x - x1) * (y2 - y)
		            + f12 * (x2 - x) * (y - y1)
		            + f22 * (x - x1) * (y - y1));
	};

	return { interp(m_Vx), interp(m_Vy) };
}
