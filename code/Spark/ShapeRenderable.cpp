#include "Spark/Shape.h"
#include "Spark/ShapeRenderable.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ShapeRenderable", ShapeRenderable, IRenderable)

ShapeRenderable::ShapeRenderable(const resource::Proxy< Shape >& shape)
:	m_shape(shape)
{
}

Aabb2 ShapeRenderable::getBounds() const
{
	return m_shape->getBounds();
}

void ShapeRenderable::render(render::RenderContext* renderContext, const Matrix33& transform, const ColorTransform& colorTransform) const
{
	m_shape->render(renderContext, transform, colorTransform);
}

	}
}
