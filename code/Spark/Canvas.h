#ifndef traktor_spark_Canvas_H
#define traktor_spark_Canvas_H

#include "Spark/IRenderable.h"

namespace traktor
{
	namespace spark
	{

/*! \brief
 * \ingroup Spark
 */
class Canvas : public IRenderable
{
	T_RTTI_CLASS;

public:
	void clear();

	void moveTo(float x, float y);

	void lineTo(float x, float y);

	void quadraticTo(float x, float y);

	void cubicTo(float x, float y);

	void close();

	void fill();

	void stroke();

	virtual Aabb2 getBounds() const T_OVERRIDE T_FINAL;

	virtual void render(render::RenderContext* renderContext, const Matrix33& transform, const ColorTransform& colorTransform) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_spark_Canvas_H
