#include "Spray/Modifiers/SizeModifier.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SizeModifier", SizeModifier, Modifier)

SizeModifier::SizeModifier(float adjustRate)
:	m_adjustRate(adjustRate)
{
}

void SizeModifier::update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const
{
	float deltaSize = m_adjustRate * deltaTime;
	for (size_t i = first; i < last; ++i)
		points[i].size += deltaSize;
}

	}
}
