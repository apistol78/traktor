#pragma once

#include "Core/Math/Matrix33.h"
#include "Spark/Action/ActionObjectRelay.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class Point;

/*! ActionScript Matrix class.
 * \ingroup Spark
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

