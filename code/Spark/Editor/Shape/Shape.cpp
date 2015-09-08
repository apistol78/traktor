#include "Spark/Editor/Shape/Shape.h"
#include "Spark/Editor/Shape/ShapeVisitor.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Shape", Shape, Object)

Shape::Shape()
:	m_transform(Matrix33::identity())
{
}

void Shape::setStyle(Style* style)
{
	m_style = style;
}

Style* Shape::getStyle() const
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

void Shape::visit(ShapeVisitor* shapeVisitor)
{
	shapeVisitor->enter(this);

	for (RefArray< Shape >::iterator i = m_children.begin(); i != m_children.end(); ++i)
		(*i)->visit(shapeVisitor);

	shapeVisitor->leave(this);
}

	}
}
