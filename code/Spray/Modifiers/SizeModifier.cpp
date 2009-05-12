#include "Spray/Modifiers/SizeModifier.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.spray.SizeModifier", SizeModifier, Modifier)

SizeModifier::SizeModifier()
:	m_adjustRate(0.0f)
{
}

void SizeModifier::update(const Scalar& deltaTime, const Matrix44& transform, PointVector& points, size_t first, size_t last) const
{
	for (size_t i = first; i < last; ++i)
		points[i].size += m_adjustRate * deltaTime;
}

bool SizeModifier::serialize(Serializer& s)
{
	return s >> Member< float >(L"adjustRate", m_adjustRate);
}

	}
}
