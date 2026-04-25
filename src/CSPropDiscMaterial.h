/*
*	Copyright (C) 2008-2012 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#include <hdf5.h>
#include "CSProperties.h"
#include "CSPropMaterial.h"

typedef unsigned char uint8;

class vtkPolyData;

//! Continuous Structure Discrete Material Property
/*!
  This Property reads a discrete material distribution from a file. (currently only HDF5)
  */
class CSXCAD_EXPORT CSPropDiscMaterial : public CSPropMaterial
{
public:
	CSPropDiscMaterial(ParameterSet* paraSet);
	CSPropDiscMaterial(CSPropDiscMaterial* prop, bool copyPrim=false);
	CSPropDiscMaterial(unsigned int ID, ParameterSet* paraSet);
	virtual ~CSPropDiscMaterial();

	//! Create a copy of this property. Optional: Copy all primitives assigned to this property too.
	virtual CSProperties* GetCopy(bool incl_prim=false) {return new CSPropDiscMaterial(this, incl_prim);}

	virtual const std::string GetTypeXMLString() const {return std::string("DiscMaterial");}

	virtual double GetEpsilonWeighted(int ny, const double* coords);
	virtual double GetMueWeighted(int ny, const double* coords);
	virtual double GetKappaWeighted(int ny, const double* coords);
	virtual double GetSigmaWeighted(int ny, const double* coords);

	virtual double GetDensityWeighted(const double* coords);

	//! Set true if database index 0 is used as background material (default), or false if CSPropMaterial should be used as index 0
	virtual void SetUseDataBaseForBackground(bool val) {m_DB_Background=val;}
	bool GetUseDataBaseForBackground() const {return m_DB_Background;}

	void SetFilename(const std::string& fn) {m_Filename=fn; m_FileRead=false;}
	std::string GetFilename() const {return m_Filename;}

	void SetFileType(int type) {m_FileType=type;}
	int GetFileType() const {return m_FileType;}

	//! Get the affine transform applied to lookup coordinates before the scale factor.
	/*! Returns NULL if no transform has been set.
	 *  Coordinate lookup order: InvertTransform(coord) -> divide by Scale -> look up in mesh. */
	CSTransform* GetTransform() {return m_Transform;}

	//! Set (and take ownership of) an affine transform applied to lookup coordinates.
	/*! The transform is applied before the scale factor: coords are first
	 *  inverse-transformed, then divided by Scale, then looked up in the mesh.
	 *  Any previously set transform is deleted. Pass NULL to remove the transform. */
	void SetTransform(CSTransform* transform);

	//! Set the isotropic scale factor applied to lookup coordinates after the transform.
	/*! Coordinates are divided by Scale before the mesh lookup, so Scale acts as
	 *  a unit conversion: e.g. Scale=1e-3 if the mesh is in mm but coords are in m.
	 *  Applied after the optional affine transform (see SetTransform). */
	void SetScale(double val) {m_Scale=val;}
	double GetScale() {return m_Scale;}

	virtual bool Write2XML(TiXmlNode& root, bool parameterised=true, bool sparse=false);
	virtual bool ReadFromXML(TiXmlNode &root);

	bool ReadFile();

	virtual void ShowPropertyStatus(std::ostream& stream);

	//! Create a vtkPolyData surface that separates the discrete material from background material
	virtual vtkPolyData* CreatePolyDataModel() const;

protected:
	unsigned int GetWeightingPos(const double* coords);
	int GetDBPos(const double* coords);
	virtual void Init();

	int m_FileType;
	std::string m_Filename;
	unsigned int m_Size[3];
	unsigned int m_DB_size;
	uint8* m_Disc_Ind;
	float *m_mesh[3];
	float *m_Disc_epsR;
	float *m_Disc_kappa;
	float *m_Disc_mueR;
	float *m_Disc_sigma;
	float *m_Disc_Density;
	double m_Scale;
	bool m_DB_Background;
	bool m_FileRead;
	CSTransform* m_Transform;

	void EnsureFileLoaded();
	bool ReadHDF5(std::string filename);
	void* ReadDataSet(std::string filename, std::string d_name, hid_t type_id, int &rank, unsigned int &size, bool debug=false);
};

