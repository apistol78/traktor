#ifndef traktor_spark_Canvas_H
#define traktor_spark_Canvas_H

#include <list>
#include "Core/Object.h"
#include "Core/Math/Color4f.h"
#include "Spark/Path.h"
#include "Spark/Triangulator.h"

namespace traktor
{
	namespace spark
	{

class Context;
class Shape;

/*! \brief
 * \ingroup Spark
 */
class Canvas : public Object
{
	T_RTTI_CLASS;

public:
	void clear();

	void moveTo(float x, float y);

	void lineTo(float x, float y);

	void quadricTo(float x1, float y1, float x, float y);

	void quadricTo(float x, float y);

	void cubicTo(float x1, float y1, float x2, float y2, float x, float y);

	void cubicTo(float x2, float y2, float x, float y);

	void close();

	void rect(float x, float y, float width, float height, float radius);

	void circle(float x, float y, float radius);

	void fill(const Color4f& fillColor);

	void stroke();

	Ref< Shape > createShape(const Context* context) const;

private:
	struct Batch
	{
		Color4f fillColor;
		AlignedVector< Triangulator::Triangle > triangles;
	};

	Path m_path;
	std::list< Batch > m_batches;
};

	}
}

#endif	// traktor_spark_Canvas_H
