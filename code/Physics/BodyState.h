#ifndef traktor_physics_BodyState_H
#define traktor_physics_BodyState_H

#include "Core/Math/Transform.h"
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace physics
	{

/*! \brief Rigid body state.
 * \ingroup Physics
 */
struct BodyState
{
	BodyState()
	:	m_transform(Transform::identity())
	,	m_linearVelocity(Vector4::zero())
	,	m_angularVelocity(Vector4::zero())
	{
	}

	void setTransform(const Transform& transform) {
		m_transform = transform;
	}

	const Transform& getTransform() const {
		return m_transform;
	}

	void setLinearVelocity(const Vector4& velocity) {
		m_linearVelocity = velocity;
	}

	const Vector4& getLinearVelocity() const {
		return m_linearVelocity;
	}

	void setAngularVelocity(const Vector4& velocity) {
		m_angularVelocity = velocity;
	}

	const Vector4& getAngularVelocity() const {
		return m_angularVelocity;
	}

	BodyState interpolate(const BodyState& stateTarget, const Scalar& interpolate) const
	{
		BodyState state;
		state.m_transform = lerp(m_transform, stateTarget.m_transform, interpolate);
		state.m_linearVelocity = lerp(m_linearVelocity, stateTarget.m_linearVelocity, interpolate);
		state.m_angularVelocity = lerp(m_angularVelocity, stateTarget.m_angularVelocity, interpolate);
		return state;
	}

private:
	Transform m_transform;
	Vector4 m_linearVelocity;
	Vector4 m_angularVelocity;
};

	}
}

#endif	// traktor_physics_BodyState_H
