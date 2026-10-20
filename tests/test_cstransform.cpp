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
  Tests for the inverse of CSTransform, including Invert(), which the python
  test suite cannot reach.

  Build with -DCSXCAD_BUILD_TESTS=ON and run it through ctest:

    cmake -DCSXCAD_BUILD_TESTS=ON <the usual options> ..
    make && ctest --output-on-failure

  Exits non-zero and prints "FAIL: ..." per failed check.
*/

#include "CSTransform.h"

#include <cmath>
#include <iostream>

static int fails = 0;
static const double PI = 3.14159265358979323846;   // M_PI is not standard, MSVC lacks it
#define CHECK(cond, msg) do { if (!(cond)) { std::cout << "FAIL: " << msg << "\n"; ++fails; } } while (0)

// largest deviation of a*b from the unit matrix
static double UnitDeviation(const double* a, const double* b)
{
	double dev = 0;
	for (int r=0;r<4;++r)
		for (int c=0;c<4;++c)
		{
			double s = 0;
			for (int k=0;k<4;++k)
				s += a[4*r+k]*b[4*k+c];
			dev = std::max(dev, std::fabs(s - (r==c ? 1.0 : 0.0)));
		}
	return dev;
}

int main()
{
	// a general matrix: no zero entries, a non-trivial bottom row
	const double full[16] = { 3.0,  0.4, -0.7,  1.0,
	                          0.2,  2.5,  0.9, -2.0,
	                         -0.6,  0.3,  4.0,  0.5,
	                          0.1, -0.2,  0.3,  2.0};
	// an affine transform built from all kinds of steps
	CSTransform composite;
	{
		const double axis[3] = {1,2,3};
		const double scale[3] = {2,0.5,3};
		const double shift[3] = {-1,4,2};
		composite.RotateOrigin(axis, 37*PI/180);
		composite.Scale(scale);
		composite.Translate(shift);
		composite.RotateY(-70*PI/180);
	}

	// ---- 1. Invert() makes the inverse the forward matrix: M*Minv = I
	{
		CSTransform tr;
		tr.SetMatrix(full);
		double m[16];
		for (int n=0;n<16;++n)
			m[n] = full[n];
		tr.Invert();
		CHECK(UnitDeviation(m, tr.GetMatrix()) < 1e-12, "Invert() of a full matrix is no inverse");

		CSTransform tr2(&composite);
		for (int n=0;n<16;++n)
			m[n] = composite.GetMatrix()[n];
		tr2.Invert();
		CHECK(UnitDeviation(m, tr2.GetMatrix()) < 1e-12, "Invert() of a composite transform is no inverse");

		// inverting twice gives the original back
		tr2.Invert();
		double dev = 0;
		for (int n=0;n<16;++n)
			dev = std::max(dev, std::fabs(tr2.GetMatrix()[n]-m[n]));
		CHECK(dev < 1e-12, "Invert() twice is not the original");
	}

	// ---- 2. InvertTransform() undoes Transform() of an affine transform
	{
		const double p[3] = {1.5, -2.0, 0.25};
		double q[3], r[3];
		composite.Transform(p, q);
		composite.InvertTransform(q, r);
		double dev = 0;
		for (int n=0;n<3;++n)
			dev = std::max(dev, std::fabs(r[n]-p[n]));
		CHECK(dev < 1e-12, "InvertTransform() does not undo Transform()");
	}

	if (fails)
		std::cout << fails << " check(s) failed\n";
	return fails ? 1 : 0;
}
