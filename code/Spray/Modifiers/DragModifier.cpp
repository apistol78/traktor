#include "Spray/Modifiers/DragModifier.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.DragModifier", 0, DragModifier, Modifier)

DragModifier::DragModifier()
:	m_linearDrag(0.0f)
,	m_angularDrag(0.0f)
{
}

void DragModifier::update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const
{
	for (size_t i = first; i < last; ++i)
	{
		points[i].velocity *= Scalar(1.0f) - m_linearDrag * deltaTime;
		points[i].angularVelocity *= 1.0f - m_angularDrag * deltaTime;
	}
}

bool DragModifier::serialize(ISerializer& s)
{
	s >> Member< Scalar >(L"linearDrag", m_linearDrag);
	s >> Member< float >(L"angularDrag", m_angularDrag);
	return true;
}

	}
}
