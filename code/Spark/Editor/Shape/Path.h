#ifndef traktor_spark_Path_H
#define traktor_spark_Path_H

#include <vector>
#include "Core/Object.h"
#include "Core/Math/Vector2.h"

namespace traktor
{
	namespace spark
	{

enum SubPathType
{
	SptUndefined,
	SptLinear,
	SptQuadric,
	SptCubic
};

struct SubPath
{
	SubPathType type;
	bool closed;
	std::vector< Vector2 > points;

	SubPath(SubPathType type_)
	:	type(type_)
	,	closed(false)
	{}
};

/*! \brief
 * \ingroup Spark
 */
class Path : public Object
{
	T_RTTI_CLASS;

public:
	Path();

	void moveTo(float x, float y, bool relative = false);

	void lineTo(float x, float y, bool relative = false);

	void quadricTo(float x1, float y1, float x, float y, bool relative = false);

	void quadricTo(float x, float y, bool relative = false);

	void cubicTo(float x1, float y1, float x2, float y2, float x, float y, bool relative = false);

	void cubicTo(float x2, float y2, float x, float y, bool relative = false);

	void close();

	const Vector2& getCursor() const;

	const std::vector< SubPath >& getSubPaths() const;

private:
	std::vector< SubPath > m_subPaths;
	Vector2 m_cursor;
	SubPath* m_current;

	void getAbsolute(float& x, float& y) const;
};

	}
}

#endif	// traktor_spark_Path_H
