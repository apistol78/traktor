#include "Physics/DynamicBodyState.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.DynamicBodyState", DynamicBodyState, ISerializable)

DynamicBodyState::DynamicBodyState()
:	m_transform(Transform::identity())
,	m_linearVelocity(Vector4::zero())
,	m_angularVelocity(Vector4::zero())
{
}

void DynamicBodyState::setTransform(const Transform& transform)
{
	m_transform = transform;
}

const Transform& DynamicBodyState::getTransform() const
{
	return m_transform;
}

void DynamicBodyState::setLinearVelocity(const Vector4& velocity)
{
	m_linearVelocity = velocity;
}

const Vector4& DynamicBodyState::getLinearVelocity() const
{
	return m_linearVelocity;
}

void DynamicBodyState::setAngularVelocity(const Vector4& velocity)
{
	m_angularVelocity = velocity;
}

const Vector4& DynamicBodyState::getAngularVelocity() const
{
	return m_angularVelocity;
}

DynamicBodyState DynamicBodyState::interpolate(const DynamicBodyState& stateTarget, const Scalar& interpolate) const
{
	DynamicBodyState state;
	state.m_transform = lerp(m_transform, stateTarget.m_transform, interpolate);
	state.m_linearVelocity = lerp(m_linearVelocity, stateTarget.m_linearVelocity, interpolate);
	state.m_angularVelocity = lerp(m_angularVelocity, stateTarget.m_angularVelocity, interpolate);
	return state;
}

bool DynamicBodyState::serialize(ISerializer& s)
{
	s >> MemberComposite< Transform >(L"transform", m_transform);
	s >> Member< Vector4 >(L"linearVelocity", m_linearVelocity);
	s >> Member< Vector4 >(L"angularVelocity", m_angularVelocity);
	return true;
}

	}
}
