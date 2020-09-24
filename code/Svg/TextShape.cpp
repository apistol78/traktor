#include "Svg/TextShape.h"

namespace traktor
{
	namespace svg
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.TextShape", TextShape, Shape)

TextShape::TextShape(const Vector2& position)
:	m_position(position)
{
}

const Vector2& TextShape::getPosition() const
{
	return m_position;
}

	}
}
