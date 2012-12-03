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

#include "tinyxml.h"
#include <hdf5.h>
#include <hdf5_hl.h>

#include "ParameterCoord.h"
#include "CSPropDiscMaterial.h"

CSPropDiscMaterial::CSPropDiscMaterial(ParameterSet* paraSet) : CSPropMaterial(paraSet)
{
	Type=(CSProperties::PropertyType)(DISCRETE_MATERIAL | MATERIAL);
	Init();
}

CSPropDiscMaterial::CSPropDiscMaterial(CSProperties* prop) : CSPropMaterial(prop)
{
	Type=(CSProperties::PropertyType)(DISCRETE_MATERIAL | MATERIAL);
	Init();
}

CSPropDiscMaterial::CSPropDiscMaterial(unsigned int ID, ParameterSet* paraSet) : CSPropMaterial(ID, paraSet)
{
	Type=(CSProperties::PropertyType)(DISCRETE_MATERIAL | MATERIAL);
	Init();
}

CSPropDiscMaterial::~CSPropDiscMaterial()
{
	for (int n=0;n<3;++n)
	{
		delete[] m_mesh[n];
		m_mesh[n]=NULL;
	}
	delete[] m_Disc_epsR;
	m_Disc_epsR=NULL;
	delete[] m_Disc_kappa;
	m_Disc_kappa=NULL;
	delete[] m_Disc_mueR;
	m_Disc_mueR=NULL;
	delete[] m_Disc_sigma;
	m_Disc_sigma=NULL;
	delete[] m_Disc_Density;
	m_Disc_Density=NULL;

	delete m_Transform;
	m_Transform=NULL;
}

unsigned int CSPropDiscMaterial::GetWeightingPos(const double* inCoords)
{
	double coords[3];
	TransformCoordSystem(inCoords, coords, coordInputType, CARTESIAN);
	if (m_Transform)
		m_Transform->InvertTransform(coords,coords);
	for (int n=0;n<3;++n)
		coords[n]/=m_Scale;
	unsigned int pos[3];
	if (!(m_mesh[0] && m_mesh[1] && m_mesh[2]))
		return -1;
	for (int n=0;n<3;++n)
	{
		if (coords[n]<m_mesh[n][0])
			return -1;
		if (coords[n]>m_mesh[n][m_Size[n]-1])
			return -1;
		pos[n]=0;
		for (unsigned int i=0;i<m_Size[n];++i)
		{
			if (coords[n]<m_mesh[n][i])
			{
				pos[n]=i;
				break;
			}
		}
	}
	return pos[0] + pos[1]*m_Size[0] + pos[2]*m_Size[0]*m_Size[1];
}

double CSPropDiscMaterial::GetEpsilonWeighted(int ny, const double* inCoords)
{
	if (m_Disc_epsR==NULL)
		return CSPropMaterial::GetEpsilonWeighted(ny,inCoords);
	unsigned int pos1 = GetWeightingPos(inCoords);
	if (pos1==(unsigned int)-1)
		return CSPropMaterial::GetEpsilonWeighted(ny,inCoords);
	return m_Disc_epsR[pos1];
}

double CSPropDiscMaterial::GetKappaWeighted(int ny, const double* inCoords)
{
	if (m_Disc_kappa==NULL)
		return CSPropMaterial::GetKappaWeighted(ny,inCoords);
	unsigned int pos1 = GetWeightingPos(inCoords);
	if (pos1==(unsigned int)-1)
		return CSPropMaterial::GetKappaWeighted(ny,inCoords);
	if (m_Disc_kappa[pos1]>3)
		cerr << "kappa to large? " << m_Disc_kappa[pos1] << endl;
	return m_Disc_kappa[pos1];
}

double CSPropDiscMaterial::GetMueWeighted(int ny, const double* inCoords)
{
	if (m_Disc_mueR==NULL)
		return CSPropMaterial::GetMueWeighted(ny,inCoords);
	unsigned int pos1 = GetWeightingPos(inCoords);
	if (pos1==(unsigned int)-1)
		return CSPropMaterial::GetMueWeighted(ny,inCoords);
	return m_Disc_mueR[pos1];
}

double CSPropDiscMaterial::GetSigmaWeighted(int ny, const double* inCoords)
{
	if (m_Disc_sigma==NULL)
		return CSPropMaterial::GetSigmaWeighted(ny,inCoords);
	unsigned int pos1 = GetWeightingPos(inCoords);
	if (pos1==(unsigned int)-1)
		return CSPropMaterial::GetSigmaWeighted(ny,inCoords);
	return m_Disc_sigma[pos1];
}

double CSPropDiscMaterial::GetDensityWeighted(const double* inCoords)
{
	if (m_Disc_Density==NULL)
		return CSPropMaterial::GetDensityWeighted(inCoords);
	unsigned int pos1 = GetWeightingPos(inCoords);
	if (pos1==(unsigned int)-1)
		return CSPropMaterial::GetDensityWeighted(inCoords);
	return m_Disc_Density[pos1];
}

void CSPropDiscMaterial::Init()
{
	m_Filename.clear();
	m_FileType=-1;

	for (int n=0;n<3;++n)
		m_mesh[n]=NULL;
	m_Disc_epsR=NULL;
	m_Disc_kappa=NULL;
	m_Disc_mueR=NULL;
	m_Disc_sigma=NULL;
	m_Disc_Density=NULL;

	m_Scale=1;
	m_Transform=NULL;

	CSPropMaterial::Init();
}

bool CSPropDiscMaterial::Write2XML(TiXmlNode& root, bool parameterised, bool sparse)
{
	if (CSPropMaterial::Write2XML(root,parameterised,sparse) == false) return false;
	TiXmlElement* prop=root.ToElement();
	if (prop==NULL) return false;

	TiXmlElement filename("DiscFile");
	filename.SetAttribute("Type",m_FileType);
	filename.SetAttribute("File",m_Filename.c_str());

	filename.SetAttribute("Scale",m_Scale);

	if (m_Transform)
		m_Transform->Write2XML(prop);

	prop->InsertEndChild(filename);

	return true;
}

bool CSPropDiscMaterial::ReadFromXML(TiXmlNode &root)
{
	if (CSPropMaterial::ReadFromXML(root)==false) return false;
	TiXmlElement* prop=root.ToElement();

	if (prop==NULL) return false;

	m_FileType = 0;
	prop->QueryIntAttribute("Type",&m_FileType);
	const char* c_filename = prop->Attribute("File");

	delete m_Transform;
	m_Transform = CSTransform::New(prop, clParaSet);

	if (prop->QueryDoubleAttribute("Scale",&m_Scale)!=TIXML_SUCCESS)
		m_Scale=1;

	if (c_filename==NULL)
		return true;

	if ((m_FileType==0) && (c_filename!=NULL))
		return ReadHDF5(c_filename);
	else
		cerr << "CSPropDiscMaterial::ReadFromXML: Unknown file type or no filename given." << endl;

	return true;
}

float *CSPropDiscMaterial::ReadDataSet(string filename, string d_name, int &rank, unsigned int &size, bool debug)
{
	herr_t status;
	H5T_class_t class_id;
	size_t type_size;
	rank = -1;

	// open hdf5 file
	hid_t file_id = H5Fopen( filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
	if (file_id < 0)
	{
		if (debug)
			cerr << __func__ << ": Failed to open file, skipping..." << endl;
		H5Fclose(file_id);
		return NULL;
	}

	if (H5Lexists(file_id, d_name.c_str(), H5P_DEFAULT)<=0)
	{
		if (debug)
			cerr << __func__ << ": Warning, dataset: \"" << d_name << "\" not found... skipping" << endl;
		H5Fclose(file_id);
		return NULL;
	}

	status = H5LTget_dataset_ndims(file_id, d_name.c_str(), &rank);
	if (status < 0)
	{
		if (debug)
			cerr << __func__ << ": Warning, failed to read dimension for dataset: \"" << d_name << "\" skipping..." << endl;
		H5Fclose(file_id);
		return NULL;
	}

	hsize_t dims[rank];
	status = H5LTget_dataset_info( file_id, d_name.c_str(), dims, &class_id, &type_size);
	if (status < 0)
	{
		if (debug)
			cerr << __func__ << ": Warning, failed to read dataset info: \"" << d_name << "\" skipping..." << endl;
		H5Fclose(file_id);
		return NULL;
	}

	size = 1;
	for (int n=0;n<rank;++n)
		size*=dims[n];

	float* data = new float[size];
	status = H5LTread_dataset_float( file_id, d_name.c_str(), data );
	if (status < 0)
	{
		if (debug)
			cerr << __func__ << ": Warning, failed to read dataset: \"" << d_name << "\" skipping..." << endl;
		delete[] data;
		H5Fclose(file_id);
		return NULL;
	}

	H5Fclose(file_id);
	return data;
}

bool CSPropDiscMaterial::ReadHDF5( string filename )
{
	cout << __func__ << ": Reading \"" << filename << "\"" << endl;

	// open hdf5 file
	hid_t file_id = H5Fopen( filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
	if (file_id < 0)
	{
		cerr << __func__ << ": Failed to open file, skipping..." << endl;
		return false;
	}

	unsigned int size;
	int rank;
	// read mesh
	unsigned int numCells = 1;
	string names[] = {"/mesh/x","/mesh/y","/mesh/z"};
	for (int n=0; n<3; ++n)
	{
		m_mesh[n] = ReadDataSet(filename, names[n], rank, size);
		if ((m_mesh[n]==NULL) || (rank!=1))
		{
			cerr << __func__ << ": Failed to read mesh, skipping..." << endl;
			return false;
		}
		m_Size[n]=size;
		numCells*=m_Size[n];
	}

	string name = "/epsR";
	m_Disc_epsR = ReadDataSet(filename, name, rank, size);
	if (m_Disc_epsR==NULL)
	{
		cerr << __func__ << ": Failed to read " << name << ", skipping..." << endl;
	}
	else if (rank!=3)
	{
		cerr << __func__ << ": Error, data dimension error!!! Found rank: " << rank << endl;
		return false;
	}
	else if (size != numCells)
	{
		cerr << __func__ << ": Error, data size doesn't match!!! " << size << " vs. " << m_Size[0]*m_Size[1]*m_Size[2] << endl;
		return false;
	}

	name = "/kappa";
	m_Disc_kappa = ReadDataSet(filename, name, rank, size);
	if (m_Disc_kappa==NULL)
	{
		cerr << __func__ << ": Failed to read " << name << ", skipping..." << endl;
	}
	else if (rank!=3)
	{
		cerr << __func__ << ": Error, data dimension error!!! Found rank: " << rank << endl;
		return false;
	}
	else if (size != numCells)
	{
		cerr << __func__ << ": Error, data size doesn't match!!! " << size << " vs. " << m_Size[0]*m_Size[1]*m_Size[2] << endl;
		return false;
	}

	name = "/mueR";
	m_Disc_mueR = ReadDataSet(filename, name, rank, size);
	if (m_Disc_mueR==NULL)
	{
		cerr << __func__ << ": Failed to read " << name << ", skipping..." << endl;
	}
	else if (rank!=3)
	{
		cerr << __func__ << ": Error, data dimension error!!! Found rank: " << rank << endl;
		return false;
	}
	else if (size != numCells)
	{
		cerr << __func__ << ": Error, data size doesn't match!!! " << size << " vs. " << m_Size[0]*m_Size[1]*m_Size[2] << endl;
		return false;
	}

	name = "/sigma";
	m_Disc_sigma = ReadDataSet(filename, name, rank, size);
	if (m_Disc_sigma==NULL)
	{
		cerr << __func__ << ": Failed to read " << name << ", skipping..." << endl;
	}
	else if (rank!=3)
	{
		cerr << __func__ << ": Error, data dimension error!!! Found rank: " << rank << endl;
		return false;
	}
	else if (size != numCells)
	{
		cerr << __func__ << ": Error, data size doesn't match!!! " << size << " vs. " << m_Size[0]*m_Size[1]*m_Size[2] << endl;
		return false;
	}

	name = "/density";
	m_Disc_Density = ReadDataSet(filename, name, rank, size);
	if (m_Disc_Density==NULL)
	{
		cerr << __func__ << ": Failed to read " << name << ", skipping..." << endl;
	}
	else if (rank!=3)
	{
		cerr << __func__ << ": Error, data dimension error!!! Found rank: " << rank << endl;
		return false;
	}
	else if (size != numCells)
	{
		cerr << __func__ << ": Error, data size doesn't match!!! " << size << " vs. " << m_Size[0]*m_Size[1]*m_Size[2] << endl;
		return false;
	}

	return true;
}
