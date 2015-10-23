#include "Spark/Editor/Svg/SvgShape.h"
#include "Spark/Editor/Svg/SvgShapeVisitor.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SvgShape", SvgShape, Object)

SvgShape::SvgShape()
:	m_transform(Matrix33::identity())
{
}

void SvgShape::setStyle(const SvgStyle* style)
{
	m_style = style;
}

const SvgStyle* SvgShape::getStyle() const
{
	return m_style;
}

void SvgShape::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

const Matrix33& SvgShape::getTransform() const
{
	return m_transform;
}

void SvgShape::addChild(SvgShape* shape)
{
	m_children.push_back(shape);
}

void SvgShape::visit(SvgShapeVisitor* shapeVisitor)
{
	shapeVisitor->enter(this);

	for (RefArray< SvgShape >::iterator i = m_children.begin(); i != m_children.end(); ++i)
		(*i)->visit(shapeVisitor);

	shapeVisitor->leave(this);
}

	}
}
