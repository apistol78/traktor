#include "Spray/Modifiers/DragModifier.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.DragModifier", DragModifier, Modifier)

DragModifier::DragModifier(float linearDrag, float angularDrag)
:	m_linearDrag(linearDrag)
,	m_angularDrag(angularDrag)
{
}

void DragModifier::update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const
{
	Scalar dv = Scalar(1.0f) - m_linearDrag * deltaTime;
	float da = 1.0f - m_angularDrag * deltaTime;

	for (size_t i = first; i < last; ++i)
	{
		points[i].velocity *= dv;
		points[i].angularVelocity *= da;
	}
}

	}
}
