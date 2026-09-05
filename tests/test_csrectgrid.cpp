/*
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

/*
  Tests for CSRectGrid::Clone, which the python test suite cannot reach.

  Build with -DCSXCAD_BUILD_TESTS=ON and run it through ctest:

    cmake -DCSXCAD_BUILD_TESTS=ON <the usual options> ..
    make && ctest --output-on-failure

  Exits non-zero and prints "FAIL: ..." per failed check.
*/

#include "CSRectGrid.h"

#include <iostream>

static int fails = 0;
#define CHECK(cond, msg) do { if (!(cond)) { std::cout << "FAIL: " << msg << "\n"; ++fails; } } while (0)

int main()
{
	// ---- 1. a clone keeps every member of the original
	{
		CSRectGrid grid;
		grid.SetMeshType(CYLINDRICAL);
		grid.SetDeltaUnit(1e-3);
		grid.AddDiscLine(0,0.0);
		grid.AddDiscLine(0,1.0);
		grid.AddDiscLine(1,0.0);
		grid.AddDiscLine(2,-3.0);
		grid.GetSimArea();

		CSRectGrid* clone = CSRectGrid::Clone(&grid);
		CHECK(clone->GetMeshType()  == CYLINDRICAL, "Clone lost the mesh type");
		CHECK(clone->GetDeltaUnit() == 1e-3,        "Clone lost the delta unit");
		CHECK(clone->GetQtyLines(0) == 2,           "Clone lost the x-lines");
		CHECK(clone->GetLine(0,1)   == 1.0,         "Clone lost an x-line value");
		CHECK(clone->GetLine(2,0)   == -3.0,        "Clone lost a z-line value");
		CHECK(clone->GetSimArea()[4]== -3.0,        "Clone lost the sim area");
		delete clone;
	}

	// ---- 2. a cartesian grid stays cartesian, the default of a fresh grid
	{
		CSRectGrid grid;
		grid.SetMeshType(CARTESIAN);
		CSRectGrid* clone = CSRectGrid::Clone(&grid);
		CHECK(clone->GetMeshType() == CARTESIAN, "Clone changed the mesh type");
		delete clone;
	}

	// ---- 3. the clone owns its lines, modifying it must not touch the original
	{
		CSRectGrid grid;
		grid.AddDiscLine(0,0.0);
		grid.AddDiscLine(0,1.0);

		CSRectGrid* clone = CSRectGrid::Clone(&grid);
		clone->AddDiscLine(0,2.0);
		CHECK(clone->GetQtyLines(0) == 3, "the clone was not modified");
		CHECK(grid.GetQtyLines(0)   == 2, "modifying the clone changed the original");
		delete clone;
	}

	// ---- 4. a pending sort is carried over, not lost or applied twice
	{
		CSRectGrid grid;
		grid.AddDiscLine(0,5.0);
		grid.AddDiscLine(0,0.0);  // out of order, sorting is deferred
		grid.AddDiscLine(0,5.0);  // duplicate, dropped by the deferred sort

		CSRectGrid* clone = CSRectGrid::Clone(&grid);
		CHECK(clone->GetQtyLines(0) == 2,   "the clone did not sort and unique its lines");
		CHECK(clone->GetLine(0,0)   == 0.0, "the clone is not sorted");
		CHECK(clone->GetLine(0,1)   == 5.0, "the clone is not sorted");
		delete clone;
	}

	std::cout << (fails ? "FAILED" : "all CSRectGrid tests passed") << std::endl;
	return fails != 0;
}
