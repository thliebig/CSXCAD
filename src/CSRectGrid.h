/*
*	Copyright (C) 2008,2009,2010 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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
/*
 * Author:	Thorsten Liebig
 * Date:	03-12-2008
 * Lib:		CSXCAD
 * Version:	0.1a
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include "ParameterObjects.h"
#include "CSXCAD_Global.h"
#include "CSObject.h"

class TiXmlNode;

//! CSRectGrid is managing a rectilinear graded mesh.
class CSXCAD_EXPORT CSRectGrid : public CSObject
{
public:
	//! \sa CSObject::ObjectKind
	ObjectKind GetObjectKind() const {return GRID;}

	//! Create an empty grid.
	CSRectGrid(void);
	//! Deconstruct the grid.
	~CSRectGrid(void);

	static CSRectGrid* Clone(CSRectGrid* original);

	//! Add a disc-line in the given direction.
	void AddDiscLine(int direct, double val);
	void AddDiscLines(int direct, int numLines, double* vals);
	std::string AddDiscLines(int direct, int numLines, double* vals, std::string DistFunction);

	//! Remove the disc-line at certain index and direction.
	bool RemoveDiscLine(int direct, int index);
	//! Remove the disc-line at certain value and direction.
	bool RemoveDiscLine(int direct, double val);

	//! Remove all lines and reset to an empty grid.
	void clear();
	//! Clear all lines in a given direction.
	void ClearLines(int direct);

	//! Set the drawing unit. e.g. 1e-3 for mm as drawing unit.
	void SetDeltaUnit(double val);
	//! Get the current drawing unit.
	double GetDeltaUnit();

	//! Set a disc-line in a certain direction at a given index. Will return true on success.
	/*! The value may break the ordering, the lines are re-sorted on the next
	 read access. Note that this can drop the line again if the new value
	 duplicates an already existing one. */
	bool SetLine(int direct, size_t Index, double value);

	//! Get an array of discretization lines in a certain direction.
	/*!
	\param direct The direction of interest.
	\param array The array in which the lines will be stored. Can be NULL. Caller has to delete the array.
	\param qty Methode will return the number of lines in this direction.
	\param sorted Ignored.
	\deprecated The \a sorted argument is ignored, the lines are always in
	 increasing order.
	 */
	double* GetLines(int direct, double *array, unsigned int &qty, bool sorted=true);
	//! Get quantity of lines in certain direction.
	size_t GetQtyLines(int direct);
	//! Get a disc-line in a certain direction an at given index.
	/*!
	The lines are in increasing order, index 0 is the smallest line.
	\return The line position, or 0 if \a direct or \a Index is out of range.
	Note: 0 is a valid line position, callers that need to detect an invalid
	index have to check it against GetQtyLines() themselves.
	 */
	double GetLine(int direct, size_t Index);
	//! Get disc-lines as a comma-seperated string for given direction
	std::string GetLinesAsString(int direct);

	//! Snap a given value to a grid line for the given direction
	unsigned int Snap2LineNumber(int ny, double value, bool &inside) const;

	//! Write the grid to a given XML-node.
	/*!
	\param sorted Ignored.
	\deprecated The \a sorted argument is ignored, the lines are always written
	 in increasing order.
	 */
	bool Write2XML(TiXmlNode &root, bool sorted=false);
	//! Read the grid from a given XML-node.
	bool ReadFromXML(TiXmlNode &root);

	//! Get the dimension of current grid. \return 0,1,2 or 3. Returns -1 if one or more directions have no disc-line at all.
	int GetDimension();

	//! Set the type of mesh (e.g. Cartesian or Cylindrical mesh)
	void SetMeshType(CoordinateSystem type);

	//! Get the type of mesh (e.g. Cartesian or Cylindrical mesh)
	CoordinateSystem GetMeshType();

	//! Increase the resolution in the specified direction by the given factor.
	void IncreaseResolution(int nu, int factor);

	//! Sort the lines in a given direction.
	/*! \deprecated The lines are sorted and unique on every read access, this
	 method has no effect. */
	void Sort(int direct);

	//! Get the bounding box of the area defined by the disc-lines.
	double* GetSimArea();

	//! This will check if the given mesh is a valid 3D mesh (at least 2 lines in all directions);
	bool isValid();

protected:
	//! The disc-lines of a single direction, sorted and unique on read access.
	/*!
	The vector itself is private to this class, all access has to go through
	the methods below. That is on purpose: sorting is deferred to the next read
	access, so code reaching the storage directly could observe an out of order
	or duplicated line. Making that impossible by construction is cheaper than
	remembering to sort in each of the accessors.

	Note that a read can sort and is therefore not thread-safe. The grid is only
	read while setting up an operator, the engines work on their own copy of the
	disc-lines.
	 */
	class LineVector
	{
	public:
		LineVector() : m_Dirty(false) {}

		//! Add a line, keeping the order for the usual increasing input.
		void Add(double val);
		//! Set the line at \a idx, may break the order. \sa CSRectGrid::SetLine
		bool Set(size_t idx, double val);
		//! Remove the line at \a idx.
		bool RemoveAt(size_t idx);
		//! Remove the line of the given value.
		bool RemoveValue(double val);
		//! Remove all lines.
		void Clear() {m_Lines.clear(); m_Dirty=false;}

		//! Get all lines, in increasing order and without duplicates.
		const std::vector<double>& Get() const {EnsureSorted(); return m_Lines;}
		//! Get the number of lines, duplicates excluded.
		size_t Size() const {EnsureSorted(); return m_Lines.size();}
		//! Get the line at \a idx, or 0 if out of range. \sa CSRectGrid::GetLine
		double At(size_t idx) const;

	protected:
		//! Sort and unique the lines if a previous Add or Set broke the order.
		void EnsureSorted() const;

		mutable std::vector<double> m_Lines;
		//! Set if a line was added out of order or set to an arbitrary value.
		mutable bool m_Dirty;
	};

	LineVector Lines[3];
	double dDeltaUnit;
	double SimBox[6];
	CoordinateSystem m_meshType;
};
