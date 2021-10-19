#include "Svg/ImageShape.h"

namespace traktor
{
	namespace svg
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.ImageShape", ImageShape, Shape)

ImageShape::ImageShape(const Vector2& position, const Vector2& size, const drawing::Image* image)
:	m_position(position)
,   m_size(size)
,   m_image(image)
{
}

const Vector2& ImageShape::getPosition() const
{
	return m_position;
}

const Vector2& ImageShape::getSize() const
{
	return m_size;
}

const drawing::Image* ImageShape::getImage() const
{
	return m_image;
}

	}
}
