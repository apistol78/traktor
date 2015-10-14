#ifndef traktor_spark_ShapeRenderable_H
#define traktor_spark_ShapeRenderable_H

#include "Resource/Proxy.h"
#include "Spark/IRenderable.h"

namespace traktor
{
	namespace spark
	{

class Shape;

class ShapeRenderable : public IRenderable
{
	T_RTTI_CLASS;

public:
	ShapeRenderable(const resource::Proxy< Shape >& shape);

	virtual Aabb2 getBounds() const T_OVERRIDE T_FINAL;

	virtual void render(render::RenderContext* renderContext, const Matrix33& transform, const ColorTransform& colorTransform) const T_OVERRIDE T_FINAL;

private:
	resource::Proxy< Shape > m_shape;
};

	}
}

#endif	// traktor_spark_ShapeRenderable_H
