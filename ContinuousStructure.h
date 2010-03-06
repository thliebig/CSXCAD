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
#include "ParameterObjects.h"

using namespace std;

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
	ParameterSet* GetParameterSet() {return clParaSet;};

	//! Get the Grid of this Structure.
	CSRectGrid* GetGrid() {return &clGrid;};

	//! Add an existing CSPrimitive. Class takes ownership!
	void AddPrimitive(CSPrimitives* prim);
	//! Add an existing CSProperty. Class takes ownership!
	void AddProperty(CSProperties* prop);

	//! Replace an existing property with a new one. \sa AddProperty, DeleteProperty
	bool ReplaceProperty(CSProperties* oldProp, CSProperties* newProp);

	//! Remove and delete Primitive with known index 
	void DeletePrimitive(size_t index);

	//! Remove and delete Property with known index 
	void DeleteProperty(size_t index);

	//! Remove and delete a known Primitive
	void DeletePrimitive(CSPrimitives* prim);
	//! Remove and delete a known Property
	void DeleteProperty(CSProperties* prop);

	//! Get a Primitive by its internal index number. \sa GetQtyPrimitives
	CSPrimitives* GetPrimitive(size_t index);
	//! Get a primitive by its unique ID.
	CSPrimitives* GetPrimitiveByID(unsigned int ID);

	//! Get a property by its internal index number. \sa GetQtyProperties
	CSProperties* GetProperty(size_t index);

	//! Set a drawing tolerance. /sa GetPropertyByCoordPriority /sa GetPropertiesByCoordsPriority
	void SetDrawingTolerance(double val) {dDrawingTol=val;}

	//! Get a property by its priority at a given coordinate and property type.
	/*!
	/param coord Give a 3-element array with a 3D-coordinate set (x,y,z).
	/param type Specify the type searched for. (Default is ANY-type)
	/return Returns NULL if coordinate is outside the mesh, no mesh is defined or no property is found.
	 */
	CSProperties* GetPropertyByCoordPriority(double* coord, CSProperties::PropertyType type=CSProperties::ANY);

	//! Get properties by its priority at given coordinates and property type.
	/*!
	/sa GetPropertyByCoordPriority
	/param coords Give a 3*n-element array with the 3D-coordinate set (e.g. x1,y1,z1,x2,y2,z2,...)
	/param type Specify the type searched for. (Default is ANY-type)
	/return Returns an array of n properties. NULL if coordinate is outside the mesh, no mesh is defined or no property is found.
	 */
	CSProperties** GetPropertiesByCoordsPriority(double* coords, CSProperties::PropertyType type=CSProperties::ANY);

	//! Get the internal index of the primitive.
	int GetIndex(CSPrimitives* prim);
	//! Get the internal index of the property.
	int GetIndex(CSProperties* prop);

	//! Get the quantity of primitives included in this structure.
	size_t GetQtyPrimitives() {return vPrimitives.size();};
	//! Get the quantity of properties included in this structure.
	size_t GetQtyProperties() {return vProperties.size();};

	//! Get the quantity of properties of a certain type included in this structure.
	size_t GetQtyPropertyType(CSProperties::PropertyType type);

	//! Get a properties array of a certian type
	vector<CSProperties*>  GetPropertyByType(CSProperties::PropertyType type);

	//! Get the edges of all includes primitives and add to the desired grid direction. \param nu Direction of grid (x=0,y=1,z=2).
	bool InsertEdges2Grid(int nu);

	//! Check whether the structure is valid.
	virtual bool isGeometryValid();
	//! Update all primitives and properties e.g. with respect to changed parameter settings. \return Gives an error message in case of a found error.
	const char* Update();

	//! Get an array containing the absolute size of the current structure.
	double* GetObjectArea();

	//! Delete and clear all objects includes. This will result in an empty structure.
	void clear();

	//! Write this structure to an existing XML-node.
	/*!
	 \param rootNode XML-Node to write this structure into.
	 \param parameterised Include full parameters (default) or parameter-values only.
	 */
	virtual bool Write2XML(TiXmlNode* rootNode, bool parameterised=true);
	//! Write this structure to a file.
	/*!
	 \param file Filename to write this structure into. Will create a new file or overwrite an existing one!
	 \param parameterised Include full parameters (default) or parameter-values only.
	 */
	virtual bool Write2XML(const char* file, bool parameterised=true);

	//! Read a structure from file.
	/*!
	 \return Will return a string with possible error-messages!
	 \param file Filename to read this structure from.
	 */
	const char* ReadFromXML(const char* file);
	//! Read a structure from a given XML-node.
	/*!
	 \return Will return a string with possible error-messages!
	 \param rootNode XML-node to read this structure from.
	 */
	const char* ReadFromXML(TiXmlNode* rootNode);

	//! Get a Info-Line containing lib-name, -version etc. 
	static string GetInfoLine();

protected:
	ParameterSet* clParaSet;
	CSRectGrid clGrid;
	vector<CSPrimitives*> vPrimitives;
	vector<CSProperties*> vProperties;

	void UpdateIDs();

	unsigned int maxID;

	double ObjArea[6];
	double dDrawingTol;

	string ErrString;
	unsigned int UniqueIDCounter;
};


