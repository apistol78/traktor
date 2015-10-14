#ifndef traktor_spark_IRenderable_H
#define traktor_spark_IRenderable_H

#include "Core/Object.h"
#include "Core/Math/Aabb2.h"

namespace traktor
{

class Matrix33;

	namespace render
	{

class RenderContext;

	}

	namespace spark
	{

class ColorTransform;

/*! \brief
 * \ingroup Spark
 */
class IRenderable : public Object
{
	T_RTTI_CLASS;

public:
	virtual Aabb2 getBounds() const = 0;

	virtual void render(render::RenderContext* renderContext, const Matrix33& transform, const ColorTransform& colorTransform) const = 0;
};

	}
}

#endif	// traktor_spark_IRenderable_H
