#include "Physics/DynamicBody.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.DynamicBody", DynamicBody, Body)

void DynamicBody::setPreviousState(const DynamicBodyState& state)
{
	m_previousState = state;
}

const DynamicBodyState& DynamicBody::getPreviousState() const
{
	return m_previousState;
}

	}
}
