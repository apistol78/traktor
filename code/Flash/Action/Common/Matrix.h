/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_Matrix_H
#define traktor_flash_Matrix_H

#include "Core/Math/Matrix33.h"
#include "Flash/Action/ActionObjectRelay.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class Point;

/*! \brief ActionScript Matrix class.
 * \ingroup Flash
 */
class T_DLLCLASS Matrix : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	Matrix33 m_v;

	Matrix();

	Matrix(const float v[6]);

	Ref< Matrix > clone();

	void concat(const Matrix* rh);

	void createBox(float scaleX, float scaleY, float rotation, float tx, float ty);

	void createGradientBox(float width, float height, float rotation, float tx, float ty);

	Ref< Point > deltaTransformPoint(const Point* pt);

	void identity();

	void invert();

	void rotate(float angle);

	void scale(float scaleX, float scaleY);

	std::wstring toString();

	Ref< Point > transformPoint(const Point* pt);

	void translate(float x, float y);
};

	}
}

#endif	// traktor_flash_Matrix_H
