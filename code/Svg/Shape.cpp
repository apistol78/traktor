#include "Svg/IShapeVisitor.h"
#include "Svg/Shape.h"

namespace traktor
{
	namespace svg
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.Shape", Shape, Object)

Shape::Shape()
:	m_transform(Matrix33::identity())
{
}

void Shape::setStyle(const Style* style)
{
	m_style = style;
}

const Style* Shape::getStyle() const
{
	return m_style;
}

void Shape::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

const Matrix33& Shape::getTransform() const
{
	return m_transform;
}

void Shape::addChild(Shape* shape)
{
	m_children.push_back(shape);
}

void Shape::visit(IShapeVisitor* shapeVisitor)
{
	shapeVisitor->enter(this);

	for (auto child : m_children)
		child->visit(shapeVisitor);

	shapeVisitor->leave(this);
}

	}
}
