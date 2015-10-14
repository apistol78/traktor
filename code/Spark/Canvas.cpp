#include "Spark/Canvas.h"
#include "Spark/Triangulator.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Canvas", Canvas, IRenderable)

void Canvas::clear()
{
	m_path = Path();
}

void Canvas::moveTo(float x, float y)
{
	m_path.moveTo(x, y);
}

void Canvas::lineTo(float x, float y)
{
	m_path.lineTo(x, y);
}

void Canvas::quadricTo(float x1, float y1, float x, float y)
{
	m_path.quadricTo(x1, y1, x, y);
}

void Canvas::quadricTo(float x, float y)
{
	m_path.quadricTo(x, y);
}

void Canvas::cubicTo(float x1, float y1, float x2, float y2, float x, float y)
{
	m_path.cubicTo(x1, y1, x2, y2, x, y);
}

void Canvas::cubicTo(float x2, float y2, float x, float y)
{
	m_path.cubicTo(x2, y2, x, y);
}

void Canvas::close()
{
	m_path.close();
}

void Canvas::fill()
{
	AlignedVector< Triangulator::Triangle > triangles;
	Triangulator().triangulate(&m_path, triangles);
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
