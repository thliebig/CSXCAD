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
#include <iostream>
#include <string>
#include <vector>
#include "CSXCAD_Global.h"
#include "CSProperties.h"
#include "CSPrimitives.h"
#include "CSRectGrid.h"
#include "CSBackgroundMaterial.h"
#include "ParameterObjects.h"
#include "CSUseful.h"

class TiXmlNode;

//! Continuous Structure containing properties (layer) and primitives.
/*!
 ContinuousStructure is a class that contains certain property objects containing geometrical primitive structures as boxes, spheres, cylinders etc.
 All values in this class can contain parameters and mathematical equations.
*/
class CSXCAD_EXPORT ContinuousStructure
{
public:
	//! Create an empty structure
	ContinuousStructure(void);
	//! Deconstructor. Will delete all properties and primitives it contains!
	virtual ~ContinuousStructure(void);

	//! Get the ParameterSet created by this Structure. Needed for creation of any property or primitive!
	/*!
	 \return ParameterSet owned by this class.
	 */
	ParameterSet* GetParameterSet();

	//! Get the Grid of this Structure.
	CSRectGrid* GetGrid();

	//! Get the background material
	CSBackgroundMaterial* GetBackgroundMaterial();

	//! Add an existing CSProperty. Class takes ownership!
	void AddProperty(CSProperties* prop);

	//! Replace an existing property with a new one. \sa AddProperty, DeleteProperty
	bool ReplaceProperty(CSProperties* oldProp, CSProperties* newProp);

	//! Remove and delete Property at known index
	void DeleteProperty(size_t index);

	//! Remove and delete a known Property
	void DeleteProperty(CSProperties* prop);

	//! Get a primitive by its unique ID.
	CSPrimitives* GetPrimitiveByID(unsigned int ID);

	//! Get all properties with the given name
	std::vector<CSProperties*> GetPropertiesByName(std::string name);

	//! Get a property by its internal index number. \sa GetQtyProperties
	CSProperties* GetProperty(size_t index);

	//! Define the input type for the CSProperties weighting coordinate system 0=cartesian, 1=cylindrical, 2=spherical
	void SetCoordInputType(CoordinateSystem type);

	CoordinateSystem GetCoordInputType();

	//! Set a drawing tolerance. /sa GetPropertyByCoordPriority /sa GetPropertiesByCoordsPriority
	void SetDrawingTolerance(double val);

	//! Get a property by its priority at a given coordinate and property type.
	/*!
	\param coord Give a 3-element array with a 3D-coordinate set (x,y,z).
	\param type Specify the type searched for. (Default is ANY-type)
	\param markFoundAsUsed Mark the found primitives as beeing used. \sa WarnUnusedPrimitves
	\param foundPrimitive return the found primitive, set to NULL if none was found
	\return Returns NULL if coordinate is outside the mesh, no mesh is defined or no property is found.
	 */
	CSProperties* GetPropertyByCoordPriority(const double* coord, CSProperties::PropertyType type=CSProperties::ANY, bool markFoundAsUsed=false, CSPrimitives** foundPrimitive=NULL);

	//! Get properties by its priority at given coordinates and property type.
	/*!
	\sa GetPropertyByCoordPriority
	\param coords Give a 3*n-element array with the 3D-coordinate set (e.g. x1,y1,z1,x2,y2,z2,...)
	\param type Specify the type searched for. (Default is ANY-type)
	\param markFoundAsUsed Mark the found primitives as beeing used. \sa WarnUnusedPrimitves
	\return Returns an array of n properties. NULL if coordinate is outside the mesh, no mesh is defined or no property is found.
	 */
	CSProperties** GetPropertiesByCoordsPriority(const double* coords, CSProperties::PropertyType type=CSProperties::ANY, bool markFoundAsUsed=false);

	CSProperties* GetPropertyByCoordPriority(const double* coord, std::vector<CSPrimitives*> primList, bool markFoundAsUsed=false, CSPrimitives** foundPrimitive=NULL);

	//! Check and warn for unused primitives in properties of given type
	void WarnUnusedPrimitves(std::ostream& stream, CSProperties::PropertyType type=CSProperties::ANY);

	//! Show status of all properties with given type
	void ShowPropertyStatus(std::ostream& stream, CSProperties::PropertyType type=CSProperties::ANY);

	//! Find the property owning the given primitive or return NULL if primitive is not to be found
	CSProperties* HasPrimitive(CSPrimitives* prim);

	//! Delete the given primitive
	void DeletePrimitive(CSPrimitives* prim);

	//! Get the quantity of primitives included in this structure.
	size_t GetQtyPrimitives(CSProperties::PropertyType type=CSProperties::ANY);

	//! Get a primitives array
	std::vector<CSPrimitives*>  GetAllPrimitives(bool sorted=false, CSProperties::PropertyType type=CSProperties::ANY);

	//! Get a primitives array of a certian type
	std::vector<CSPrimitives*>  GetPrimitivesByType(CSPrimitives::PrimitiveType type);

	//! Get a primitives array inside a bounding box and with a certian property type (default is any)
	std::vector<CSPrimitives*>  GetPrimitivesByBoundBox(const double* boundbox, bool sorted=false, CSProperties::PropertyType type=CSProperties::ANY);

	//! Get the internal index of the property.
	int GetIndex(CSProperties* prop);

	//! Get the quantity of properties included in this structure.
	size_t GetQtyProperties();

	//! Get the quantity of properties of a certain type included in this structure.
	size_t GetQtyPropertyType(CSProperties::PropertyType type);

	//! Get a properties array of a certian type
	std::vector<CSProperties*>  GetPropertyByType(CSProperties::PropertyType type);

	//! Get the edges of all includes primitives and add to the desired grid direction. \param nu Direction of grid (x=0,y=1,z=2).
	bool InsertEdges2Grid(int nu);

	//! Check whether the structure is valid.
	virtual bool isGeometryValid();
	//! Update all primitives and properties e.g. with respect to changed parameter settings. \return Gives an error message in case of a found error.
	std::string Update();

	//! Get an array containing the absolute size of the current structure.
	double* GetObjectArea(CSProperties::PropertyType type=CSProperties::ANY);

	//! Delete and clear all objects includes. This will result in an empty structure.
	void clear();

	//! Write this structure to an existing XML-node.
	/*!
	 \param rootNode XML-Node to write this structure into.
	 \param parameterised Include full parameters (default) or parameter-values only.
	 */
	virtual bool Write2XML(TiXmlNode* rootNode, bool parameterised=true, bool sparse=false);
	//! Write this structure to a file.
	/*!
	 \param file Filename to write this structure into. Will create a new file or overwrite an existing one!
	 \param parameterised Include full parameters (default) or parameter-values only.
	 */
	virtual bool Write2XML(const char* file, bool parameterised=true, bool sparse=false);
	virtual bool Write2XML(std::string file, bool parameterised=true, bool sparse=false);

	//! Read a structure from file.
	/*!
	 \return Will return a string with possible error-messages!
	 \param file Filename to read this structure from.
	 */
	const char* ReadFromXML(const char* file);
	std::string ReadFromXML(std::string file);

    //! Read a structure from a given XML-node.
	/*!
	 \return Will return a string with possible error-messages!
	 \param rootNode XML-node to read this structure from.
	 */
	const char* ReadFromXML(TiXmlNode* rootNode);

	//! Get a Info-Line containing lib-name, -version etc. 
	static std::string GetInfoLine(bool shortInfo=false);

protected:
	ParameterSet* clParaSet;
	CSRectGrid clGrid;
	CSBackgroundMaterial m_BG_Mat;
	std::vector<CSProperties*> vProperties;
	bool ReadPropertyPrimitives(TiXmlElement* PropNode, CSProperties* prop);

	void UpdateIDs();

	CoordinateSystem m_MeshType;

	unsigned int maxID;

	double ObjArea[6];
	double dDrawingTol;

	std::string ErrString;
	unsigned int UniqueIDCounter;
};


