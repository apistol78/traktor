#include "Spark/Canvas.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Canvas", Canvas, IRenderable)

void Canvas::clear()
{
}

void Canvas::moveTo(float x, float y)
{
}

void Canvas::lineTo(float x, float y)
{
}

void Canvas::quadraticTo(float x, float y)
{
}

void Canvas::cubicTo(float x, float y)
{
}

void Canvas::close()
{
}

void Canvas::fill()
{
}

void Canvas::stroke()
{
}

Aabb2 Canvas::getBounds() const
{
	return Aabb2();
}

void Canvas::render(render::RenderContext* renderContext, const Matrix33& transform, const ColorTransform& colorTransform) const
{
}

	}
}
