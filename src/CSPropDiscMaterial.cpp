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

#include "vtkPolyData.h"
#include "vtkCellArray.h"
#include "vtkPoints.h"

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
	delete[] m_Disc_Ind;
	m_Disc_Ind=NULL;
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
		for (unsigned int i=1;i<m_Size[n];++i)
		{
			if (coords[n]<m_mesh[n][i])
			{
				pos[n]=i-1;
				break;
			}
		}
	}
	return pos[0] + pos[1]*(m_Size[0]-1) + pos[2]*(m_Size[0]-1)*(m_Size[1]-1);
}

int CSPropDiscMaterial::GetDBPos(const double* coords)
{
	if (m_Disc_Ind==NULL)
		return -1;
	unsigned int pos = GetWeightingPos(coords);
	if (pos==(unsigned int)-1)
		return -1;
	// material with index 0 is assumed to be background material
	if ((m_DB_Background==false) && (m_Disc_Ind[pos]==0))
			return -1;
	int db_pos = (int)m_Disc_Ind[pos];
	if (db_pos>=(int)m_DB_size)
	{
		//sanity check, this should not happen!!!
		std::cerr << __func__ << ": Error, false DB position!" << std::endl;
		return -1;
	}
	return db_pos;
}

double CSPropDiscMaterial::GetEpsilonWeighted(int ny, const double* inCoords)
{
	if (m_Disc_epsR==NULL)
		return CSPropMaterial::GetEpsilonWeighted(ny,inCoords);
	int pos = GetDBPos(inCoords);
	if (pos<0)
		return CSPropMaterial::GetEpsilonWeighted(ny,inCoords);
	return m_Disc_epsR[pos];
}

double CSPropDiscMaterial::GetKappaWeighted(int ny, const double* inCoords)
{
	if (m_Disc_kappa==NULL)
		return CSPropMaterial::GetKappaWeighted(ny,inCoords);
	int pos = GetDBPos(inCoords);
	if (pos<0)
		return CSPropMaterial::GetKappaWeighted(ny,inCoords);
	return m_Disc_kappa[pos];
}

double CSPropDiscMaterial::GetMueWeighted(int ny, const double* inCoords)
{
	if (m_Disc_mueR==NULL)
		return CSPropMaterial::GetMueWeighted(ny,inCoords);
	int pos = GetDBPos(inCoords);
	if (pos<0)
		return CSPropMaterial::GetMueWeighted(ny,inCoords);
	return m_Disc_mueR[pos];
}

double CSPropDiscMaterial::GetSigmaWeighted(int ny, const double* inCoords)
{
	if (m_Disc_sigma==NULL)
		return CSPropMaterial::GetSigmaWeighted(ny,inCoords);
	int pos = GetDBPos(inCoords);
	if (pos<0)
		return CSPropMaterial::GetSigmaWeighted(ny,inCoords);
	return m_Disc_sigma[pos];
}

double CSPropDiscMaterial::GetDensityWeighted(const double* inCoords)
{
	if (m_Disc_Density==NULL)
		return CSPropMaterial::GetDensityWeighted(inCoords);
	int pos = GetDBPos(inCoords);
	if (pos<0)
		return CSPropMaterial::GetDensityWeighted(inCoords);
	return m_Disc_Density[pos];
}

void CSPropDiscMaterial::Init()
{
	m_Filename.clear();
	m_FileType=-1;

	m_DB_size = 0;
	m_DB_Background = true;

	for (int n=0;n<3;++n)
		m_mesh[n]=NULL;
	m_Disc_Ind=NULL;
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
	filename.SetAttribute("UseDBBackground",m_DB_Background);
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

	int help;
	if (prop->QueryIntAttribute("UseDBBackground",&help)==TIXML_SUCCESS)
		SetUseDataBaseForBackground(help!=0);

	delete m_Transform;
	m_Transform = CSTransform::New(prop, clParaSet);

	if (prop->QueryDoubleAttribute("Scale",&m_Scale)!=TIXML_SUCCESS)
		m_Scale=1;

	if (c_filename==NULL)
		return true;

	if ((m_FileType==0) && (c_filename!=NULL))
		return ReadHDF5(c_filename);
	else
		std::cerr << "CSPropDiscMaterial::ReadFromXML: Unknown file type or no filename given." << std::endl;

	return true;
}

void *CSPropDiscMaterial::ReadDataSet(std::string filename, std::string d_name, int type_id, int &rank, unsigned int &size, bool debug)
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
			std::cerr << __func__ << ": Failed to open file, skipping..." << std::endl;
		H5Fclose(file_id);
		return NULL;
	}

	if (H5Lexists(file_id, d_name.c_str(), H5P_DEFAULT)<=0)
	{
		if (debug)
			std::cerr << __func__ << ": Warning, dataset: \"" << d_name << "\" not found... skipping" << std::endl;
		H5Fclose(file_id);
		return NULL;
	}

	status = H5LTget_dataset_ndims(file_id, d_name.c_str(), &rank);
	if (status < 0)
	{
		if (debug)
			std::cerr << __func__ << ": Warning, failed to read dimension for dataset: \"" << d_name << "\" skipping..." << std::endl;
		H5Fclose(file_id);
		return NULL;
	}

	hsize_t* dims = new hsize_t[rank];
	status = H5LTget_dataset_info( file_id, d_name.c_str(), dims, &class_id, &type_size);
	if (status < 0)
	{
		if (debug)
			std::cerr << __func__ << ": Warning, failed to read dataset info: \"" << d_name << "\" skipping..." << std::endl;
		H5Fclose(file_id);
		return NULL;
	}

	size = 1;
	for (int n=0;n<rank;++n)
		size*=dims[n];
	delete[] dims; dims = NULL;

	void* data;
	if (type_id==H5T_NATIVE_FLOAT)
		data = (void*) new float[size];
	else if (type_id==H5T_NATIVE_INT)
		data = (void*) new int[size];
	else if (type_id==H5T_NATIVE_UINT8)
		data = (void*) new uint8[size];
	else
	{
		std::cerr << __func__ << ": Error, unknown data type" << std::endl;
		H5Fclose(file_id);
		return NULL;
	}

	status = H5LTread_dataset( file_id, d_name.c_str(), type_id, data );
	if (status < 0)
	{
		if (debug)
			std::cerr << __func__ << ": Warning, failed to read dataset: \"" << d_name << "\" skipping..." << std::endl;
		if (type_id==H5T_NATIVE_FLOAT)
			delete[] (float*)data;
		else if (type_id==H5T_NATIVE_INT)
			delete[] (int*)data;
		else if (type_id==H5T_NATIVE_UINT8)
			delete[] (uint8*)data;
		H5Fclose(file_id);
		return NULL;
	}

	H5Fclose(file_id);
	return data;
}

bool CSPropDiscMaterial::ReadHDF5( std::string filename )
{
	cout << __func__ << ": Reading \"" << filename << "\"" << std::endl;

	// open hdf5 file
	hid_t file_id = H5Fopen( filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
	if (file_id < 0)
	{
		std::cerr << __func__ << ": Error, failed to open file, abort..." << std::endl;
		return false;
	}

	double ver;
	herr_t status = H5LTget_attribute_double(file_id, "/", "Version", &ver);
	if (status < 0)
		ver = 1.0;

	if (ver<2.0)
	{
		std::cerr << __func__ << ": Error, older file versions are no longer supported, abort..." << std::endl;
		H5Fclose(file_id);
		return false;
	}

	int db_size;
	status = H5LTget_attribute_int(file_id, "/DiscData", "DB_Size", &db_size);
	if (status<0)
	{
		std::cerr << __func__ << ": Error, can't read database size, abort..." << std::endl;
		H5Fclose(file_id);
		return false;
	}

	m_DB_size = db_size;
	if (H5Lexists(file_id, "/DiscData", H5P_DEFAULT)<=0)
	{
		std::cerr << __func__ << ": Error, can't read database, abort..." << std::endl;
		H5Fclose(file_id);
		return false;
	}

	hid_t dataset = H5Dopen2(file_id, "/DiscData", H5P_DEFAULT);
	if (dataset<0)
	{
		std::cerr << __func__ << ": Error, can't open database" << std::endl;
		H5Fclose(file_id);
		return 0;
	}

	// read database
	if (H5LTfind_attribute(dataset, "epsR")==1)
	{
		m_Disc_epsR = new float[db_size];
		status = H5LTget_attribute_float(file_id, "/DiscData", "epsR", m_Disc_epsR);
	}
	else
	{
		std::cerr << __func__ << ": No \"/DiscData/epsR\" found, skipping..." << std::endl;
		m_Disc_epsR=NULL;
	}

	delete[] m_Disc_kappa;
	if (H5LTfind_attribute(dataset, "kappa")==1)
	{
		m_Disc_kappa = new float[db_size];
		status = H5LTget_attribute_float(file_id, "/DiscData", "kappa", m_Disc_kappa);
	}
	else
	{
		std::cerr << __func__ << ": No \"/DiscData/kappa\" found, skipping..." << std::endl;
		m_Disc_kappa=NULL;
	}

	delete[] m_Disc_mueR;
	if (H5LTfind_attribute(dataset, "mueR")==1)
	{
		m_Disc_mueR = new float[db_size];
		status = H5LTget_attribute_float(file_id, "/DiscData", "mueR", m_Disc_mueR);
	}
	else
	{
		std::cerr << __func__ << ": No \"/DiscData/mueR\" found, skipping..." << std::endl;
		m_Disc_mueR=NULL;
	}

	delete[] m_Disc_sigma;
	if (H5LTfind_attribute(dataset, "sigma")==1)
	{
		m_Disc_sigma = new float[db_size];
		status = H5LTget_attribute_float(file_id, "/DiscData", "sigma", m_Disc_sigma);
	}
	else
	{
		std::cerr << __func__ << ": No \"/DiscData/sigma\" found, skipping..." << std::endl;
		m_Disc_sigma=NULL;
	}

	delete[] m_Disc_Density;
	if (H5LTfind_attribute(dataset, "density")==1)
	{
		m_Disc_Density = new float[db_size];
		status = H5LTget_attribute_float(file_id, "/DiscData", "density", m_Disc_Density);
	}
	else
	{
		std::cerr << __func__ << ": no \"/DiscData/density\" found, skipping..." << std::endl;
		m_Disc_Density=NULL;
	}

	H5Fclose(file_id);

	// read mesh
	unsigned int size;
	int rank;
	unsigned int numCells = 1;
	std::string names[] = {"/mesh/x","/mesh/y","/mesh/z"};
	for (int n=0; n<3; ++n)
	{
		m_mesh[n] = (float*)ReadDataSet(filename, names[n], H5T_NATIVE_FLOAT, rank, size);
		if ((m_mesh[n]==NULL) || (rank!=1) || (size<=1))
		{
			std::cerr << __func__ << ": Error, failed to read or invalid mesh, abort..." << std::endl;
			H5Fclose(file_id);
			return false;
		}
		m_Size[n]=size;
		numCells*=(m_Size[n]-1);
	}

	delete[] m_Disc_Ind;
	m_Disc_Ind = (uint8*)ReadDataSet(filename, "/DiscData", H5T_NATIVE_UINT8, rank, size, true);

	if ((m_Disc_Ind==NULL) || (rank!=3) || (size!=numCells))
	{
		std::cerr << __func__ << ": Error, can't read database indizies or size/rank is invalid, abort..." << std::endl;
		delete[] m_Disc_Ind;
		m_Disc_Ind = NULL;
		return false;
	}
	return true;
}

void CSPropDiscMaterial::ShowPropertyStatus(std::ostream& stream)
{
	CSProperties::ShowPropertyStatus(stream);
	stream << " --- Discrete Material Properties --- " << std::endl;
	stream << "  Data-Base Size:\t: " << m_DB_size << std::endl;
	stream << "  Number of Voxels:\t: " << m_Size[0] << "x" << m_Size[1] << "x" << m_Size[2] << std::endl;
	stream << " Background Material Properties: " << std::endl;
	stream << "  Isotropy\t: " << bIsotropy << std::endl;
	stream << "  Epsilon_R\t: " << Epsilon[0].GetValueString() << ", "  << Epsilon[1].GetValueString() << ", "  << Epsilon[2].GetValueString()  << std::endl;
	stream << "  Kappa\t\t: " << Kappa[0].GetValueString() << ", "  << Kappa[1].GetValueString() << ", "  << Kappa[2].GetValueString()  << std::endl;
	stream << "  Mue_R\t\t: " << Mue[0].GetValueString() << ", "  << Mue[1].GetValueString() << ", "  << Mue[2].GetValueString()  << std::endl;
	stream << "  Sigma\t\t: " << Sigma[0].GetValueString() << ", "  << Sigma[1].GetValueString() << ", "  << Sigma[2].GetValueString()  << std::endl;
	stream << "  Density\t: " << Density.GetValueString() << std::endl;
}

vtkPolyData* CSPropDiscMaterial::CreatePolyDataModel() const
{
	vtkPolyData* polydata = vtkPolyData::New();
	vtkCellArray *poly = vtkCellArray::New();
	vtkPoints *points = vtkPoints::New();


	int* pointIdx[2];
	pointIdx[0] = new int[m_Size[0]*m_Size[1]];
	pointIdx[1] = new int[m_Size[0]*m_Size[1]];
	// init point idx
	for (unsigned int n=0;n<m_Size[0]*m_Size[1];++n)
	{
		pointIdx[0][n]=-1;
		pointIdx[1][n]=-1;
	}

	unsigned int mat_idx, mat_idx_down;
	unsigned int mesh_idx=0;
	bool bd, bu;
	int nP, nPP;
	unsigned int pos[3],rpos[3];
	for (pos[2]=0;pos[2]<m_Size[2]-1;++pos[2])
	{ // each xy-plane
		for (unsigned int n=0;n<m_Size[0]*m_Size[1];++n)
		{
			pointIdx[0][n]=pointIdx[1][n];
			pointIdx[1][n]=-1;
		}
		for (pos[0]=0;pos[0]<m_Size[0]-1;++pos[0])
			for (pos[1]=0;pos[1]<m_Size[1]-1;++pos[1])
			{
				mat_idx = pos[0] + pos[1]*(m_Size[0]-1) + pos[2]*(m_Size[0]-1)*(m_Size[1]-1);
				for (int n=0;n<3;++n)
				{
					// reset relative pos
					rpos[0]=pos[0];
					rpos[1]=pos[1];
					rpos[2]=pos[2];
					bd = false;
					bu = false;
					if (pos[n]==0)
					{
						if (m_Disc_Ind[mat_idx]>0)
							bd=true;
					}
					else if (pos[n]==m_Size[n]-2)
					{
						if (m_Disc_Ind[mat_idx]>0)
							bu=true;
					}
					else
					{
						rpos[n] = pos[n]-1; // set relative pos
						mat_idx_down  = rpos[0] + rpos[1]*(m_Size[0]-1) + rpos[2]*(m_Size[0]-1)*(m_Size[1]-1);
						rpos[n] = pos[n]; // reset relative pos
						if ((m_Disc_Ind[mat_idx]>0) && (m_Disc_Ind[mat_idx_down]==0))
							bd=true;
						else if (m_Disc_Ind[mat_idx]==0 && (m_Disc_Ind[mat_idx_down]>0))
							bu=true;
					}

					rpos[0]=pos[0];
					rpos[1]=pos[1];
					rpos[2]=0;

					if (bu) // draw poly for surface up
					{
						nP = (n+1)%3;
						nPP = (n+2)%3;
						poly->InsertNextCell(4);

						mesh_idx = rpos[0] + rpos[1]*m_Size[0];
						if (pointIdx[rpos[2]][mesh_idx]<0)
							pointIdx[rpos[2]][mesh_idx] = (int)points->InsertNextPoint(m_mesh[0][rpos[0]],m_mesh[1][rpos[1]],m_mesh[2][pos[2]+rpos[2]]);
						poly->InsertCellPoint(pointIdx[rpos[2]][mesh_idx]);

						++rpos[nP];
						mesh_idx = rpos[0] + rpos[1]*m_Size[0];
						if (pointIdx[rpos[2]][mesh_idx]<0)
							pointIdx[rpos[2]][mesh_idx] = (int)points->InsertNextPoint(m_mesh[0][rpos[0]],m_mesh[1][rpos[1]],m_mesh[2][pos[2]+rpos[2]]);
						poly->InsertCellPoint(pointIdx[rpos[2]][mesh_idx]);

						++rpos[nPP];
						mesh_idx = rpos[0] + rpos[1]*m_Size[0];
						if (pointIdx[rpos[2]][mesh_idx]<0)
							pointIdx[rpos[2]][mesh_idx] = (int)points->InsertNextPoint(m_mesh[0][rpos[0]],m_mesh[1][rpos[1]],m_mesh[2][pos[2]+rpos[2]]);
						poly->InsertCellPoint(pointIdx[rpos[2]][mesh_idx]);

						--rpos[nP];
						mesh_idx = rpos[0] + rpos[1]*m_Size[0];
						if (pointIdx[rpos[2]][mesh_idx]<0)
							pointIdx[rpos[2]][mesh_idx] = (int)points->InsertNextPoint(m_mesh[0][rpos[0]],m_mesh[1][rpos[1]],m_mesh[2][pos[2]+rpos[2]]);
						poly->InsertCellPoint(pointIdx[rpos[2]][mesh_idx]);
					}
					else if (bd) // draw poly for surface down
					{
						nP = (n+1)%3;
						nPP = (n+2)%3;
						poly->InsertNextCell(4);
						mesh_idx = rpos[0] + rpos[1]*m_Size[0];
						if (pointIdx[rpos[2]][mesh_idx]<0)
							pointIdx[rpos[2]][mesh_idx] = (int)points->InsertNextPoint(m_mesh[0][rpos[0]],m_mesh[1][rpos[1]],m_mesh[2][pos[2]+rpos[2]]);
						poly->InsertCellPoint(pointIdx[rpos[2]][mesh_idx]);

						++rpos[nPP];
						mesh_idx = rpos[0] + rpos[1]*m_Size[0];
						if (pointIdx[rpos[2]][mesh_idx]<0)
							pointIdx[rpos[2]][mesh_idx] = (int)points->InsertNextPoint(m_mesh[0][rpos[0]],m_mesh[1][rpos[1]],m_mesh[2][pos[2]+rpos[2]]);
						poly->InsertCellPoint(pointIdx[rpos[2]][mesh_idx]);

						++rpos[nP];
						mesh_idx = rpos[0] + rpos[1]*m_Size[0];
						if (pointIdx[rpos[2]][mesh_idx]<0)
							pointIdx[rpos[2]][mesh_idx] = (int)points->InsertNextPoint(m_mesh[0][rpos[0]],m_mesh[1][rpos[1]],m_mesh[2][pos[2]+rpos[2]]);
						poly->InsertCellPoint(pointIdx[rpos[2]][mesh_idx]);

						--rpos[nPP];
						mesh_idx = rpos[0] + rpos[1]*m_Size[0];
						if (pointIdx[rpos[2]][mesh_idx]<0)
							pointIdx[rpos[2]][mesh_idx] = (int)points->InsertNextPoint(m_mesh[0][rpos[0]],m_mesh[1][rpos[1]],m_mesh[2][pos[2]+rpos[2]]);
						poly->InsertCellPoint(pointIdx[rpos[2]][mesh_idx]);
					}
				}
			}
	}
	delete[] pointIdx[0];
	delete[] pointIdx[1];

	polydata->SetPoints(points);
	points->Delete();
	polydata->SetPolys(poly);
	poly->Delete();

	return polydata;
}
