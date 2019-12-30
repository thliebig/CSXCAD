/*
*	Copyright (C) 2011 Thorsten Liebig (Thorsten.Liebig@gmx.de)
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

#include "CSTransform.h"

#include "CSUseful.h"
#include "tinyxml.h"

#include <math.h>
#include <iostream>

#include "vtkMatrix4x4.h"

#define PI 3.141592653589793238462643383279

CSTransform::CSTransform()
{
	Reset();
	SetParameterSet(NULL);
}

CSTransform::CSTransform(CSTransform* transform)
{
	if (transform==NULL)
	{
		Reset();
		SetParameterSet(NULL);
		return;
	}
	m_PostMultiply = transform->m_PostMultiply;
	m_AngleRadian = transform->m_AngleRadian;
	m_TransformList = transform->m_TransformList;
	m_TransformArguments = transform->m_TransformArguments;
	SetParameterSet(transform->m_ParaSet);
	for (int n=0;n<16;++n)
	{
		m_TMatrix[n] = transform->m_TMatrix[n];
		m_Inv_TMatrix[n] = transform->m_Inv_TMatrix[n];
	}
}

CSTransform::CSTransform(ParameterSet* paraSet)
{
	Reset();
	SetParameterSet(paraSet);
}

CSTransform::~CSTransform()
{
}

void CSTransform::Reset()
{
	m_PostMultiply = true;
	m_AngleRadian=true;
	m_TransformList.clear();
	m_TransformArguments.clear();
	MakeUnitMatrix(m_TMatrix);
	MakeUnitMatrix(m_Inv_TMatrix);
}

bool CSTransform::HasTransform()
{
	return (m_TransformList.size()>0);
}

void CSTransform::Invert()
{
	//make sure the inverse matrix is up to date...
	UpdateInverse();
	//switch matrices
	double help;
	for (int n=0;n<16;++n)
	{
			help = m_TMatrix[n];
			m_TMatrix[n] = m_Inv_TMatrix[n];
			m_Inv_TMatrix[n]=help;
	}
}

void CSTransform::UpdateInverse()
{
	// use vtk to do the matrix inversion
	vtkMatrix4x4::Invert(m_TMatrix, m_Inv_TMatrix);
}

double* CSTransform::Transform(const double inCoords[3], double outCoords[3]) const
{
	double coords[4] = {inCoords[0],inCoords[1],inCoords[2],1};
	for (int m=0;m<3;++m)
	{
		outCoords[m] = 0;
		for (int n=0;n<4;++n)
		{
			outCoords[m] +=  m_TMatrix[4*m+n]*coords[n];
		}
	}
	return outCoords;
}

double* CSTransform::InvertTransform(const double inCoords[3], double outCoords[3]) const
{
	double coords[4] = {inCoords[0],inCoords[1],inCoords[2],1};
	for (int m=0;m<3;++m)
	{
		outCoords[m] = 0;
		for (int n=0;n<4;++n)
		{
			outCoords[m] +=  m_Inv_TMatrix[4*m+n]*coords[n];
		}
	}
	return outCoords;
}

void CSTransform::SetMatrix(const double matrix[16], bool concatenate)
{
	ApplyMatrix(matrix,concatenate);
	AppendList(MATRIX,matrix,16);
}

bool CSTransform::SetMatrix(std::string matrix, bool concatenate)
{
	std::vector<std::string> mat_vec = SplitString2Vector(matrix, ',');
	ParameterScalar ps_matrix[16];

	double d_matrix[16];
	if (mat_vec.size()>16)
		std::cerr << "CSTransform::SetMatrix: Warning: Number of arguments for operation: \"Matrix\" with arguments: \"" << matrix << "\" is larger than expected, skipping unneeded! " << std::endl;
	else if (mat_vec.size()<16)
	{
		std::cerr << "CSTransform::SetMatrix: Error: Number of arguments for operation: \"Matrix\" with arguments: \"" << matrix << "\" is invalid! Skipping" << std::endl;
		return false;
	}

	for (int n=0;n<16;++n)
	{
		ps_matrix[n].SetParameterSet(m_ParaSet);
		ps_matrix[n].SetValue(mat_vec.at(n));
		int EC = ps_matrix[n].Evaluate();
		if (EC!=0)
			return false;
		d_matrix[n]=ps_matrix[n].GetValue();
	}

	ApplyMatrix(d_matrix,concatenate);
	AppendList(MATRIX,ps_matrix,16);
	return true;
}

bool CSTransform::TranslateMatrix(double matrix[16], const double translate[3])
{
	MakeUnitMatrix(matrix);
	//put translate vector into the last column
	for (int n=0;n<3;++n)
		matrix[4*n+3] = translate[n];
	return true;
}

void CSTransform::Translate(const double translate[3], bool concatenate)
{
	double matrix[16];

	if (TranslateMatrix(matrix, translate)==false)
		return;

	ApplyMatrix(matrix,concatenate);
	AppendList(TRANSLATE,translate,3);
}

bool CSTransform::Translate(std::string translate, bool concatenate)
{
	double matrix[16];

	std::vector<std::string> tl_vec = SplitString2Vector(translate, ',');
	ParameterScalar ps_translate[3];
	double tl_double_vec[3];
	if (tl_vec.size()>3)
		std::cerr << "CSTransform::Translate: Warning: Number of arguments for operation: \"Translate\" with arguments: \"" << translate << "\" is larger than expected, skipping unneeded! " << std::endl;
	else if (tl_vec.size()<3)
	{
		std::cerr << "CSTransform::Translate: Error: Number of arguments for operation: \"Translate\" with arguments: \"" << translate << "\" is invalid! Skipping" << std::endl;
		return false;
	}

	for (int n=0;n<3;++n)
	{
		ps_translate[n].SetParameterSet(m_ParaSet);
		ps_translate[n].SetValue(tl_vec.at(n));
		int EC = ps_translate[n].Evaluate();
		if (EC!=0)
			return false;
		tl_double_vec[n]=ps_translate[n].GetValue();
	}

	if (TranslateMatrix(matrix, tl_double_vec)==false)
		return false;

	ApplyMatrix(matrix,concatenate);
	AppendList(TRANSLATE,ps_translate,3);
	return true;
}

bool CSTransform::RotateOriginMatrix(double matrix[16], const double XYZ_A[4])
{
	double length = sqrt(XYZ_A[0]*XYZ_A[0]+XYZ_A[1]*XYZ_A[1]+XYZ_A[2]*XYZ_A[2]);
	if (length==0)
	{
		std::cerr << "CSTransform::RotateOriginVector: Warning: vector length is zero! skipping" << std::endl;
		return false;
	}

	for (int n=0;n<16;++n)
		matrix[n]=0;
	matrix[15] = 1;

	double angle = XYZ_A[3];
	if (m_AngleRadian==false)
		angle *= PI/180;

	double unit_vec[3] = {XYZ_A[0]/length,XYZ_A[1]/length,XYZ_A[2]/length};

	for (int n=0;n<3;++n)
	{
		int nP = (n+1)%3;
		int nM = (n+2)%3;
		//diagonal
		matrix[4*n+n] += unit_vec[n]*unit_vec[n]+(1-unit_vec[n]*unit_vec[n])*cos(angle);
		//diagonal + 1
		matrix[4*n+nP] += unit_vec[n]*unit_vec[nP]*(1-cos(angle))-unit_vec[nM]*sin(angle);
		//diagonal + 2
		matrix[4*n+nM] += unit_vec[n]*unit_vec[nM]*(1-cos(angle))+unit_vec[nP]*sin(angle);
	}

	return true;
}

void CSTransform::RotateOrigin(const double vector[3], double angle, bool concatenate)
{
	double XYZ_A[4]={vector[0],vector[1],vector[2],angle};

	double matrix[16];
	if (RotateOriginMatrix(matrix, XYZ_A)==false)
		return;

	ApplyMatrix(matrix,concatenate);
	AppendList(ROTATE_ORIGIN,XYZ_A,4);
}

bool CSTransform::RotateOrigin(std::string XYZ_A, bool concatenate)
{
	double matrix[16];

	std::vector<std::string> rot_vec = SplitString2Vector(XYZ_A, ',');
	ParameterScalar ps_rotate[4];
	double rot_double_vec[4];
	if (rot_vec.size()>4)
		std::cerr << "CSTransform::RotateOrigin: Warning: Number of arguments for operation: \"RotateOrigin\" with arguments: \"" << XYZ_A << "\" is larger than expected, skipping unneeded! " << std::endl;
	else if (rot_vec.size()<4)
	{
		std::cerr << "CSTransform::RotateOrigin: Error: Number of arguments for operation: \"RotateOrigin\" with arguments: \"" << XYZ_A << "\" is invalid! Skipping" << std::endl;
		return false;
	}

	for (int n=0;n<4;++n)
	{
		ps_rotate[n].SetParameterSet(m_ParaSet);
		ps_rotate[n].SetValue(rot_vec.at(n));
		int EC = ps_rotate[n].Evaluate();
		if (EC!=0)
			return false;
		rot_double_vec[n]=ps_rotate[n].GetValue();
	}

	if (RotateOriginMatrix(matrix, rot_double_vec)==false)
		return false;

	ApplyMatrix(matrix,concatenate);
	AppendList(ROTATE_ORIGIN,ps_rotate,4);
	return true;
}

void CSTransform::RotateXYZ(int dir, double angle, bool concatenate)
{
	if ((dir<0) || (dir>3))
		return;

	double vec[4]={0,0,0,angle};
	vec[dir] = 1;

	double matrix[16];
	if (RotateOriginMatrix(matrix, vec)==false)
		return;

	ApplyMatrix(matrix,concatenate);
	TransformType type = (TransformType)((int)ROTATE_X + dir);
	AppendList(type,&angle,1);
}

bool CSTransform::RotateXYZ(int dir, std::string angle, bool concatenate)
{
	if ((dir<0) || (dir>3))
		return false;

	ParameterScalar ps_angle(m_ParaSet, angle);
	int EC = ps_angle.Evaluate();
	if (EC!=0)
		return false;
	double vec[4]={0,0,0,ps_angle.GetValue()};
	vec[dir] = 1;

	double matrix[16];
	if (RotateOriginMatrix(matrix, vec)==false)
		return false;

	ApplyMatrix(matrix,concatenate);
	TransformType type = (TransformType)((int)ROTATE_X + dir);
	AppendList(type,&ps_angle,1);
	return true;
}

void CSTransform::RotateX(double angle, bool concatenate)
{
	return RotateXYZ(0,angle,concatenate);
}

bool CSTransform::RotateX(std::string angle, bool concatenate)
{
	return RotateXYZ(0,angle,concatenate);
}

void CSTransform::RotateY(double angle, bool concatenate)
{
	return RotateXYZ(1,angle,concatenate);
}

bool CSTransform::RotateY(std::string angle, bool concatenate)
{
	return RotateXYZ(1,angle,concatenate);
}

void CSTransform::RotateZ(double angle, bool concatenate)
{
	return RotateXYZ(2,angle,concatenate);
}

bool CSTransform::RotateZ(std::string angle, bool concatenate)
{
	return RotateXYZ(2,angle,concatenate);
}

bool CSTransform::ScaleMatrix(double matrix[16], double scale)
{
	MakeUnitMatrix(matrix);
	for (int n=0;n<3;++n)
		matrix[4*n+n] = scale;
	return true;
}

bool CSTransform::ScaleMatrix(double matrix[16], const double scale[3])
{
	MakeUnitMatrix(matrix);
	for (int n=0;n<3;++n)
		matrix[4*n+n] = scale[n];
	return true;
}

void CSTransform::Scale(double scale, bool concatenate)
{
	double matrix[16];

	if (ScaleMatrix(matrix, scale)==false)
		return;

	ApplyMatrix(matrix,concatenate);
	AppendList(SCALE,&scale,1);
}

void CSTransform::Scale(const double scale[3], bool concatenate)
{
	double matrix[16];

	if (ScaleMatrix(matrix, scale)==false)
		return;

	ApplyMatrix(matrix,concatenate);
	AppendList(SCALE3,scale,3);
}

bool CSTransform::Scale(std::string scale, bool concatenate)
{
	double matrix[16];

	std::vector<std::string> scale_vec = SplitString2Vector(scale, ',');

	if ((scale_vec.size()>1) && (scale_vec.size()!=3))
		std::cerr << "CSTransform::Scale: Warning: Number of arguments for operation: \"Scale\" with arguments: \"" << scale << "\" is larger than expected, skipping unneeded! " << std::endl;
	else if (scale_vec.size()<1)
	{
		std::cerr << "CSTransform::Scale: Error: Number of arguments for operation: \"Scale\" with arguments: \"" << scale << "\" is invalid! Skipping" << std::endl;
		return false;
	}

	if (scale_vec.size()>=3)
	{
		ParameterScalar ps_scale[3];
		double scale_double_vec[3];
		for (int n=0;n<3;++n)
		{
			ps_scale[n].SetParameterSet(m_ParaSet);
			ps_scale[n].SetValue(scale_vec.at(n));
			int EC = ps_scale[n].Evaluate();
			if (EC!=0)
				return false;
			scale_double_vec[n]=ps_scale[n].GetValue();
		}

		if (ScaleMatrix(matrix, scale_double_vec)==false)
			return false;

		ApplyMatrix(matrix,concatenate);
		AppendList(SCALE3,ps_scale,3);
		return true;
	}

	if(scale_vec.size()>=1)
	{
		ParameterScalar ps_scale(m_ParaSet, scale);
		int EC = ps_scale.Evaluate();
		if (EC!=0)
			return false;

		if (ScaleMatrix(matrix, ps_scale.GetValue())==false)
			return false;

		ApplyMatrix(matrix,concatenate);
		AppendList(SCALE,&ps_scale,1);
		return true;
	}

	std::cerr << "CSTransform::Scale: Error: Number of arguments for operation: \"Scale\" with arguments: \"" << scale << "\" is invalid! Skipping" << std::endl;
	return false;
}

void CSTransform::ApplyMatrix(const double matrix[16], bool concatenate)
{
	if (concatenate)
	{
		double new_matrix[16];
		for (int n=0;n<16;++n)
			new_matrix[n]=0;
		for (int n=0;n<4;++n)
			for (int m=0;m<4;++m)
			{
				for (int k=0;k<4;++k)
					if (m_PostMultiply)
						new_matrix[4*m+n] += matrix[4*m+k]*m_TMatrix[4*k+n];
					else
						new_matrix[4*m+n] += m_TMatrix[4*m+k]*matrix[4*k+n];
			}
		for (int n=0;n<16;++n)
				m_TMatrix[n]=new_matrix[n];
	}
	else
	{
		m_TransformList.clear();
		m_TransformArguments.clear();
		for (int n=0;n<16;++n)
			m_TMatrix[n]=matrix[n];
	}
	UpdateInverse();
}

bool CSTransform::TransformByString(std::string operation, std::string argument, bool concatenate)
{
	unsigned int numArgs;
	int type = GetTypeByName(operation, numArgs);

	if (type<0)
	{
		std::cerr << "CSTransform::TransformByString: Error, unknown transformation: \"" << operation << "\"" << std::endl;
		return false;
	}

	return TransformByType((TransformType)type, argument, concatenate);
}

void CSTransform::TransformByType(TransformType type, std::vector<double> args, bool concatenate)
{
	unsigned int numArgs = args.size();
	double* arguments = new double[numArgs];
	for (unsigned int n=0;n<args.size();++n)
		arguments[n] = (double)args.at(n);
	TransformByType(type, arguments, concatenate);
	delete[] arguments; arguments=NULL;
}

bool CSTransform::TransformByType(TransformType type, std::string args, bool concatenate)
{
	//Keep this in sync with GetTypeByName and TransformType!!!
	switch (type)
	{
	case SCALE:
	case SCALE3:
		return Scale(args, concatenate);
	case TRANSLATE:
		return Translate(args,concatenate);
	case ROTATE_ORIGIN:
		return RotateOrigin(args,concatenate);
	case ROTATE_X:
		return RotateX(args,concatenate);
	case ROTATE_Y:
		return RotateY(args,concatenate);
	case ROTATE_Z:
		return RotateZ(args,concatenate);
	case MATRIX:
		return SetMatrix(args,concatenate);
	default:
		return false;
	}
}

void CSTransform::TransformByType(TransformType type, const double* args, bool concatenate)
{
	//Keep this in sync with GetTypeByName and TransformType!!!
	switch (type)
	{
	case SCALE:
		return Scale(args[0], concatenate);
	case SCALE3:
		return Scale(args, concatenate);
	case TRANSLATE:
		return Translate(args,concatenate);
	case ROTATE_ORIGIN:
		return RotateOrigin(args,args[3],concatenate);
	case ROTATE_X:
		return RotateX(args[0],concatenate);
	case ROTATE_Y:
		return RotateY(args[0],concatenate);
	case ROTATE_Z:
		return RotateZ(args[0],concatenate);
	case MATRIX:
		return SetMatrix(args,concatenate);
	default:
		return;
	}
}

std::string CSTransform::GetNameByType(TransformType type) const
{
	unsigned int numArgs;
	return GetNameByType(type, numArgs);
}

std::string CSTransform::GetNameByType(TransformType type, unsigned int &numArgs) const
{
	//Keep this in sync with GetTypeByName and TransformByType and TransformType!!!
	switch (type)
	{
	case SCALE:
		numArgs=1;
		return "Scale";
	case SCALE3:
		numArgs=3;
		return "Scale3";
	case TRANSLATE:
		numArgs=3;
		return "Translate";
	case ROTATE_ORIGIN:
		numArgs=4;
		return "Rotate_Origin";
	case ROTATE_X:
		numArgs=1;
		return "Rotate_X";
	case ROTATE_Y:
		numArgs=1;
		return "Rotate_Y";
	case ROTATE_Z:
		numArgs=1;
		return "Rotate_Z";
	case MATRIX:
		numArgs=16;
		return "Matrix";
	default:
		numArgs=0;
		return "Unknown";
	}
}

int CSTransform::GetTypeByName(std::string name, unsigned int &numArgs) const
{
	//Keep this in sync with GetNameByType and TransformByType and TransformType!!!
	if (name.compare("Scale")==0)
	{
		numArgs=1;
		return SCALE;
	}
	if (name.compare("Scale3")==0)
	{
		numArgs=3;
		return SCALE3;
	}
	if (name.compare("Translate")==0)
	{
		numArgs=3;
		return TRANSLATE;
	}
	if (name.compare("Rotate_Origin")==0)
	{
		numArgs=4;
		return ROTATE_ORIGIN;
	}
	if (name.compare("Rotate_X")==0)
	{
		numArgs=1;
		return ROTATE_X;
	}
	if (name.compare("Rotate_Y")==0)
	{
		numArgs=1;
		return ROTATE_Y;
	}
	if (name.compare("Rotate_Z")==0)
	{
		numArgs=1;
		return ROTATE_Z;
	}
	if (name.compare("Matrix")==0)
	{
		numArgs=16;
		return MATRIX;
	}
	numArgs = 0;
	return -1;
}

void CSTransform::PrintMatrix(ostream& stream)
{
	for (int m=0;m<4;++m)
	{
		stream << m_TMatrix[4*m+0] << "\t" << m_TMatrix[4*m+1] << "\t" << m_TMatrix[4*m+2] << "\t" << m_TMatrix[4*m+3] << std::endl;
	}
}

void CSTransform::PrintTransformations(ostream& stream, std::string prefix)
{
	for (size_t n=0;n<m_TransformList.size();++n)
	{
		stream << prefix << GetNameByType(m_TransformList.at(n)) << "(";
		for (size_t a=0;a<m_TransformArguments.at(n).size();++a)
		{
			stream << m_TransformArguments.at(n).at(a).GetValueString();
			if (a<m_TransformArguments.at(n).size()-1)
				stream << ",";
		}
		stream << ")" << std::endl;
	}
}

void CSTransform::AppendList(TransformType type, const double* args, size_t numArgs )
{
	m_TransformList.push_back(type);
	std::vector<ParameterScalar> argument;
	for (size_t n=0;n<numArgs;++n)
		argument.push_back(ParameterScalar(m_ParaSet,args[n]));
	m_TransformArguments.push_back(argument);
}

void CSTransform::AppendList(TransformType type, const ParameterScalar* args, size_t numArgs )
{
	m_TransformList.push_back(type);
	std::vector<ParameterScalar> argument;
	for (size_t n=0;n<numArgs;++n)
		argument.push_back(args[n]);
	m_TransformArguments.push_back(argument);
}

double* CSTransform::MakeUnitMatrix(double* matrix) const
{
	for (int n=0;n<4;++n)
		for (int m=0;m<4;++m)
		{
			if (n==m)
				matrix[4*m+n]=1;
			else
				matrix[4*m+n]=0;
		}
	return matrix;
}

bool CSTransform::Write2XML(TiXmlNode* root, bool parameterised, bool sparse)
{
	UNUSED(sparse);
	UNUSED(parameterised);

	TiXmlElement Transform("Transformation");

	for (size_t n=0;n<m_TransformList.size();++n)
	{
		TiXmlElement newTransform(GetNameByType(m_TransformList.at(n)).c_str());

		std::string args;
		for (size_t a=0;a<m_TransformArguments.at(n).size();++a)
		{
			args.append(m_TransformArguments.at(n).at(a).GetValueString());
			if (a<m_TransformArguments.at(n).size()-1)
				args.append(",");
		}
		newTransform.SetAttribute("Argument",args.c_str());
		Transform.InsertEndChild(newTransform);
	}

	root->InsertEndChild(Transform);
	return true;
}

bool CSTransform::ReadFromXML(TiXmlNode* root)
{
	TiXmlElement* prop=root->FirstChildElement("Transformation");
	if (prop==NULL) return false;

	TiXmlElement* PropNode = prop->FirstChildElement();
	while (PropNode!=NULL)
	{
		std::string argument(PropNode->Attribute("Argument"));
		if (TransformByString(PropNode->Value(),argument,true)==false)
			std::cerr << "CSTransform::ReadFromXML: Warning: Reading of \"" << PropNode->Value() << "\" with arguments: \""  << argument << "\" failed." << std::endl;
		PropNode=PropNode->NextSiblingElement();
	}
	return true;
}

CSTransform* CSTransform::New(TiXmlNode* root, ParameterSet* paraSet)
{
	CSTransform* newCST = new CSTransform(paraSet);
	if (newCST->ReadFromXML(root))
		return newCST;
	delete newCST;
	return NULL;
}

CSTransform* CSTransform::New(CSTransform* cst, ParameterSet* paraSet)
{
	if (cst==NULL)
		return NULL;
	CSTransform* newCST = new CSTransform(cst);
	if (paraSet)
		newCST->SetParameterSet(paraSet);
	return newCST;
}
