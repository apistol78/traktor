#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Physics/BodyState.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.BodyState", 0, BodyState, ISerializable)

BodyState::BodyState()
:	m_transform(Transform::identity())
,	m_linearVelocity(Vector4::zero())
,	m_angularVelocity(Vector4::zero())
{
}

void BodyState::setTransform(const Transform& transform)
{
	m_transform = transform;
}

const Transform& BodyState::getTransform() const
{
	return m_transform;
}

void BodyState::setLinearVelocity(const Vector4& velocity)
{
	m_linearVelocity = velocity;
}

const Vector4& BodyState::getLinearVelocity() const
{
	return m_linearVelocity;
}

void BodyState::setAngularVelocity(const Vector4& velocity)
{
	m_angularVelocity = velocity;
}

const Vector4& BodyState::getAngularVelocity() const
{
	return m_angularVelocity;
}

BodyState BodyState::interpolate(const BodyState& stateTarget, const Scalar& interpolate) const
{
	BodyState state;
	state.m_transform = lerp(m_transform, stateTarget.m_transform, interpolate);
	state.m_linearVelocity = lerp(m_linearVelocity, stateTarget.m_linearVelocity, interpolate);
	state.m_angularVelocity = lerp(m_angularVelocity, stateTarget.m_angularVelocity, interpolate);
	return state;
}

bool BodyState::serialize(ISerializer& s)
{
	s >> MemberComposite< Transform >(L"transform", m_transform);
	s >> Member< Vector4 >(L"linearVelocity", m_linearVelocity);
	s >> Member< Vector4 >(L"angularVelocity", m_angularVelocity);
	return true;
}

	}
}
