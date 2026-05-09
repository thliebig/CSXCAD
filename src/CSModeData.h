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

#pragma once

#include <array>
#include <string>
#include <vector>

#include "CSXCAD_Global.h"

//! Holds a sampled 2-D waveguide mode and provides bilinear interpolation.
/*!
  The mode is stored on a regular (but not necessarily uniform) rectangular
  grid.  The HDF5 file must contain:
    - 1-D datasets  /x  and  /y  (double, at least 2 points each)
    - 2-D datasets  /Vx and /Vy  (double, shape nx x ny, row-major: data[i,j] at (x[i],y[j]))
    - a scalar root attribute  Version  (double, currently 1.0)

  Vx/Vy are the two transverse components of the mode field in the coordinate
  directions of the mode plane.  Whether they represent E, H etc. is determined
  by the caller (excitation or probe).
*/
class CSXCAD_EXPORT CSModeData
{
public:
	CSModeData() = default;

	bool ReadFromHDF5(const std::string& filename);
	void Clear();
	bool IsLoaded() const { return !m_Vx.empty(); }

	//! Bilinear interpolation at (x, y).
	//! Returns {Vx, Vy}. Coordinates outside the grid are clamped to the boundary.
	std::array<double, 2> LinInterp2(double x, double y) const;

private:
	std::vector<double> m_X;   // x-axis, length m_nx
	std::vector<double> m_Y;   // y-axis, length m_ny
	std::vector<double> m_Vx;  // row-major (m_nx x m_ny)
	std::vector<double> m_Vy;
	size_t m_nx = 0;
	size_t m_ny = 0;
};
