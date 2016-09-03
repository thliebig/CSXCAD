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

#ifndef CSTRANSFORM_H
#define CSTRANSFORM_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include "ParameterObjects.h"

class CSXCAD_EXPORT CSTransform
{
public:
	CSTransform();
	CSTransform(CSTransform* transform);
	CSTransform(ParameterSet* paraSet);
	virtual ~CSTransform();

	void SetParameterSet(ParameterSet* paraset) {m_ParaSet=paraset;}

	enum TransformType
	{
		SCALE, SCALE3, TRANSLATE, ROTATE_ORIGIN, ROTATE_X, ROTATE_Y, ROTATE_Z, MATRIX
	}; //Keep this in sync with GetNameByType and GetTypeByName and TransformByType methods!!!

	double* Transform(const double inCoords[3], double outCoords[3]) const;
	double* InvertTransform(const double inCoords[3], double outCoords[3]) const;

	void Invert();

	double* GetMatrix() {return m_TMatrix;}

	//! Apply a matrix directly
	void SetMatrix(const double matrix[16], bool concatenate=true);
	bool SetMatrix(std::string matrix, bool concatenate=true);

	//! Create and apply a translation matrix
	void Translate(const double translate[3], bool concatenate=true);
	bool Translate(std::string translate, bool concatenate=true);

	//! Create and apply a rotation matrix around the given vector and angle
	void RotateOrigin(const double vector[3], double angle, bool concatenate=true);
	bool RotateOrigin(std::string XYZ_A, bool concatenate=true);

	void RotateX(double angle, bool concatenate=true);
	bool RotateX(std::string angle, bool concatenate=true);

	void RotateY(double angle, bool concatenate=true);
	bool RotateY(std::string angle, bool concatenate=true);

	void RotateZ(double angle, bool concatenate=true);
	bool RotateZ(std::string angle, bool concatenate=true);

	void RotateXYZ(int dir, double angle, bool concatenate=true);
	bool RotateXYZ(int dir, std::string angle, bool concatenate=true);

	void Scale(double scale, bool concatenate=true);
	void Scale(const double scale[3], bool concatenate=true);
	bool Scale(std::string scale, bool concatenate=true);

	bool TransformByString(std::string operation, std::string argument, bool concatenate=true);

	void TransformByType(TransformType type, std::vector<double> args, bool concatenate=true);
	void TransformByType(TransformType type, const double* args, bool concatenate=true);
	bool TransformByType(TransformType type, std::string args, bool concatenate=true);

	void Reset();

	//! Check if this CSTransform has any transformations
	bool HasTransform();

	//! All subsequent operations will be occur before the previous operations (not the default).
	void SetPreMultiply() {m_PostMultiply=false;}
	//! All subsequent operations will be after the previous operations (default).
	void SetPostMultiply() {m_PostMultiply=true;}

	void SetAngleDegree() {m_AngleRadian=false;}
	void SetAngleRadian() {m_AngleRadian=true;}

	double* MakeUnitMatrix(double matrix[16]) const;

	std::string GetNameByType(TransformType type) const;
	std::string GetNameByType(TransformType type, unsigned int &numArgs) const;
	int GetTypeByName(std::string name, unsigned int &numArgs) const;

	void PrintMatrix(std::ostream& stream);

	void PrintTransformations(std::ostream& stream, std::string prefix="");

	//! Write this transformations to a xml-node. \param parameterised Use false if parameters should be written as values. Parameters are lost!
	virtual bool Write2XML(TiXmlNode* root, bool parameterised=true, bool sparse=false);
	//! Read transformations from xml-node. \return Successful read-operation.
	virtual bool ReadFromXML(TiXmlNode* root);

	static CSTransform* New(TiXmlNode* root, ParameterSet* paraSet=NULL);
	static CSTransform* New(CSTransform* cst, ParameterSet* paraSet=NULL);

protected:
	//transform matrix
	double m_TMatrix[16];
	//inverse transform matrix
	double m_Inv_TMatrix[16];

	void UpdateInverse();

	bool m_PostMultiply;
	bool m_AngleRadian;

	ParameterSet* m_ParaSet;

	void ApplyMatrix(const double matrix[16], bool concatenate);

	bool RotateOriginMatrix(double matrix[16], const double XYZ_A[4]);
	bool ScaleMatrix(double matrix[16], double scale);
	bool ScaleMatrix(double matrix[16], const double scale[3]);
	bool TranslateMatrix(double matrix[16], const double translate[3]);

	void AppendList(TransformType type, const double* args, size_t numArgs );
	void AppendList(TransformType type, const ParameterScalar* args, size_t numArgs );
	std::vector<TransformType> m_TransformList;
	std::vector<std::vector <ParameterScalar> > m_TransformArguments;
};

#endif // CSTRANSFORM_H
