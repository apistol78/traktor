#include "Svg/Gradient.h"

namespace traktor
{
	namespace svg
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.Gradient", Gradient, Object)

Gradient::Gradient(GradientType gradientType)
:	m_gradientType(gradientType)
{
}

Gradient::GradientType Gradient::getGradientType() const
{
	return m_gradientType;
}

void Gradient::addStop(float offset, const Color4f& color)
{
	m_stops.push_back({ offset, color });
}

const AlignedVector< Gradient::Stop >& Gradient::getStops() const
{
	return m_stops;
}

	}
}
