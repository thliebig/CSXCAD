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
#include <vector>
#include "ParameterObjects.h"
#include "CSTransform.h"
#include "CSXCAD_Global.h"
#include "CSUseful.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>

class CSPrimitives;

class CSPropUnknown;
class CSPropMaterial;
	class CSPropDispersiveMaterial;
		class CSPropLorentzMaterial;
		class CSPropDebyeMaterial;
	class CSPropDiscMaterial;
class CSPropLumpedElement;
class CSPropMetal;
	class CSPropConductingSheet;
class CSPropExcitation;
class CSPropProbeBox;
	class CSPropDumpBox;
class CSPropResBox;

class TiXmlNode;

typedef struct
{
	unsigned char R,G,B,a;
} RGBa;

//! Continuous Structure (CS)Properties (layer)
/*!
 CSProperties is a class that contains geometrical primitive structures as boxes, spheres, cylinders etc. (CSPrimitives)
 All values in this class can contain parameters and mathematical equations.
 This absract base-class contains basic property methodes, e.g. set/get primitives, mesh-relations, color-information etc.
*/
class CSXCAD_EXPORT CSProperties
{
public:
	virtual ~CSProperties();
	//! Copy constructor
	CSProperties(CSProperties* prop);
	//! Enumeration of all possible sub-types of this base-class
	enum PropertyType
	{
		ANY = 0xffff, UNKNOWN = 0x001, MATERIAL = 0x002, METAL = 0x004, EXCITATION = 0x008, PROBEBOX = 0x010, RESBOX = 0x020, DUMPBOX = 0x040, /* unused = 0x080, */
		DISPERSIVEMATERIAL = 0x100, LORENTZMATERIAL = 0x200, DEBYEMATERIAL = 0x400,
		DISCRETE_MATERIAL = 0x1000, LUMPED_ELEMENT = 0x2000, CONDUCTINGSHEET = 0x4000
	};
	
	//! Get PropertyType \sa PropertyType
	int GetType();
	//! Get PropertyType as a xml element name \sa PropertyType and GetType
	virtual const std::string GetTypeXMLString() const {return std::string("Any");}

	//! Get Property Type as a string. (default is the xml element name)
	virtual const std::string GetTypeString() const {return GetTypeXMLString();}

	ParameterSet* GetParameterSet() {return clParaSet;}

	//! Check if Property is a physical material. Current PropertyType: MATERIAL & METAL
	bool GetMaterial() {return bMaterial;}
	//!Get ID of this property. Used for primitive-->property mapping. \sa SetID	
	unsigned int GetID();
	//!Set ID to this property. USE ONLY WHEN YOU KNOW WHAT YOU ARE DOING!!!! \sa GetID
	void SetID(unsigned int ID);

	//!Get unique ID of this property. Used internally. \sa SetUniqueID	
	unsigned int GetUniqueID();
	//!Set unique ID of this property. Used internally. USE ONLY WHEN YOU KNOW WHAT YOU ARE DOING!!!! \sa GetUniqueID	
	void SetUniqueID(unsigned int uID);

	//! Set Name for this Property. \sa GetName
	void SetName(const std::string name);
	//! Get Name for this Property. \sa SetName
	const std::string GetName();

	//! Check if given attribute exists
	bool ExistAttribute(std::string name);
	//! Get the value of a given attribute
	std::string GetAttributeValue(std::string name);
	//! Add a new attribute
	void AddAttribute(std::string name, std::string value);

	//! Add a primitive to this Propertie. Takes ownership of this primitive! \sa CSPrimitives, RemovePrimitive, TakePrimitive
	void AddPrimitive(CSPrimitives *prim);
	//! Check if primitive is owned by this Propertie. \sa CSPrimitives, AddPrimitive, RemovePrimitive, TakePrimitive
	bool HasPrimitive(CSPrimitives *prim);
	//! Removes a primitive of this Property. Caller must take ownership! \sa CSPrimitives, AddPrimitive, TakePrimitive
	void RemovePrimitive(CSPrimitives *prim);
	//! Removes and deletes a primitive of this Property. \sa CSPrimitives, RemovePrimitive, AddPrimitive, TakePrimitive
	void DeletePrimitive(CSPrimitives *prim);
	//! Take a primitive of this Propertie at index. Releases ownership of this primitive to caller! \sa CSPrimitives, RemovePrimitive, AddPrimitive \return NULL if not found!
	CSPrimitives* TakePrimitive(size_t index);

	//! Check whether the given coord is inside of a primitive assigned to this property and return the found primitive and \a priority.
	CSPrimitives* CheckCoordInPrimitive(const double *coord, int &priority, bool markFoundAsUsed=false, double tol=0);

	//! Get the quentity of primitives assigned to this property! \return Number of primitives in this property
	size_t GetQtyPrimitives();

	//! Get the Primitive at certain index position. \sa GetQtyPrimitives
	CSPrimitives* GetPrimitive(size_t index);

	//! Get all Primitives \sa GetPrimitive
	std::vector<CSPrimitives*> GetAllPrimitives() {return vPrimitives;}
	
	//! Set a fill-color for this property. \sa GetFillColor
	void SetFillColor(RGBa color);
	void SetFillColor(unsigned char R, unsigned char G, unsigned char B, unsigned char a);
	//! Get a fill-color for this property. \sa SetFillColor \return RGBa color object.
	RGBa GetFillColor();

	//! Set a edge-color for this property. \sa SetEdgeColor
	void SetEdgeColor(RGBa color);
	void SetEdgeColor(unsigned char R, unsigned char G, unsigned char B, unsigned char a);
	//! Get a fill-color for this property. \sa GetEdgeColor \return RGBa color object.
	RGBa GetEdgeColor();

	//! Get visibility for this property. \sa SetVisibility
	bool GetVisibility();
	//! Set visibility for this property. \sa GetVisibility
	void SetVisibility(bool val);

	//! Convert to Unknown Property, returns NULL if type is different! \return Returns a CSPropUnknown* or NULL if type is different!
	CSPropUnknown* ToUnknown();
	//! Convert to Material Property, returns NULL if type is different! \return Returns a CSPropMaterial* or NULL if type is different!
	CSPropMaterial* ToMaterial();
	//! Convert to Lorentzs-Material Property, returns NULL if type is different! \return Returns a CSPropLorentzMaterial* or NULL if type is different!
	CSPropLorentzMaterial* ToLorentzMaterial();
	//! Convert to Debye-Material Property, returns NULL if type is different! \return Returns a CSPropDebyeMaterial* or NULL if type is different!
	CSPropDebyeMaterial* ToDebyeMaterial();
	//! Convert to Discrete-Material Property, returns NULL if type is different! \return Returns a CSPropDiscMaterial* or NULL if type is different!
	CSPropDiscMaterial* ToDiscMaterial();
	//! Convert to Metal Property, returns NULL if type is different! \return Returns a CSPropMetal* or NULL if type is different!
	CSPropMetal* ToMetal();
	//! Convert to Conducting Sheet Property, returns NULL if type is different! \return Returns a CSPropConductingSheet* or NULL if type is different!
	CSPropConductingSheet* ToConductingSheet();
	//! Convert to Excitation Property, returns NULL if type is different! \return Returns a CSPropExcitation* or NULL if type is different!
	CSPropExcitation* ToExcitation();
	//! Convert to ProbeBox Property, returns NULL if type is different! \return Returns a CSPropProbeBox* or NULL if type is different!
	CSPropProbeBox* ToProbeBox();
	//! Convert to ResBox Property, returns NULL if type is different! \return Returns a CSPropResBox* or NULL if type is different!
	CSPropResBox* ToResBox();
	//! Convert to DumpBox Property, returns NULL if type is different! \return Returns a CSPropDumpBox* or NULL if type is different!
	CSPropDumpBox* ToDumpBox();

	//! Update all parameters. Nothing to do in this base class. \param ErrStr Methode writes error messages to this string! \return Update success
	virtual bool Update(std::string *ErrStr=NULL);

	//! Write this property to a xml-node. \param parameterised Use false if parameters should be written as values. Parameters are lost!
	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	//! Read property from xml-node. \return Successful read-operation. 
	virtual bool ReadFromXML(TiXmlNode &root);

	//! Define the input type for the weighting coordinate system 0=cartesian, 1=cylindrical, 2=spherical
	void SetCoordInputType(CoordinateSystem type, bool CopyToPrimitives=true);
	//! Get the input type for the weighting coordinate system 0=cartesian, 1=cylindrical, 2=spherical
	int GetCoordInputType() const {return coordInputType;}

	//! Check and warn for unused primitives
	void WarnUnusedPrimitves(std::ostream& stream);

	//! Show status of this property, incl. all primitives
	virtual void ShowPropertyStatus(std::ostream& stream);

protected:
	CSProperties(ParameterSet* paraSet);
	CSProperties(unsigned int ID, ParameterSet* paraSet);
	ParameterSet* clParaSet;
	ParameterSet* coordParaSet;
	//! x,y,z,rho,r,a,t one for all coord-systems (rho distance to z-axis (cylinder-coords), r for distance to origin)
	void InitCoordParameter();
	Parameter* coordPara[7];
	CoordinateSystem coordInputType;
	PropertyType Type;
	bool bMaterial;
	unsigned int uiID;
	unsigned int UniqueID;
	std::string sName;
	std::string sType;
	RGBa FillColor;
	RGBa EdgeColor;

	bool bVisisble;

	std::vector<CSPrimitives*> vPrimitives;

	//! List of additional attribute names
	std::vector<std::string> m_Attribute_Name;
	//! List of additional attribute values
	std::vector<std::string> m_Attribute_Value;
};
