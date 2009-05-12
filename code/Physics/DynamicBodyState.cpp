#include "Physics/DynamicBodyState.h"
#include "Core/Serialization/Serializer.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.DynamicBodyState", DynamicBodyState, Serializable)

DynamicBodyState::DynamicBodyState()
:	m_transform(Matrix44::identity())
,	m_linearVelocity(Vector4::zero())
,	m_angularVelocity(Vector4::zero())
{
}

void DynamicBodyState::setTransform(const Matrix44& transform)
{
	m_transform = transform;
}

const Matrix44& DynamicBodyState::getTransform() const
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
	const Matrix44& st = m_transform;
	const Matrix44& et = stateTarget.m_transform;

	Quaternion sq(st/*.transpose()*/), eq(et/*.transpose()*/);
	Vector4 sp(st.translation()), ep(et.translation());

	Quaternion q = slerp(sq, eq, interpolate).normalized();
	Vector4 p = lerp(sp, ep, interpolate);

	DynamicBodyState state;
	state.m_transform = q.inverse().toMatrix44() * translate(p);
	state.m_linearVelocity = lerp(m_linearVelocity, stateTarget.m_linearVelocity, interpolate);
	state.m_angularVelocity = lerp(m_angularVelocity, stateTarget.m_angularVelocity, interpolate);
	return state;
}

bool DynamicBodyState::serialize(Serializer& s)
{
	s >> Member< Matrix44 >(L"transform", m_transform);
	s >> Member< Vector4 >(L"linearVelocity", m_linearVelocity);
	s >> Member< Vector4 >(L"angularVelocity", m_angularVelocity);
	return true;
}

	}
}
