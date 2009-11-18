#ifndef traktor_physics_DynamicBody_H
#define traktor_physics_DynamicBody_H

#include "Physics/Body.h"
#include "Physics/DynamicBodyState.h"
#include "Core/Math/Matrix33.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class DynamicBodyState;

/*! \brief Dynamic rigid body.
 * \ingroup Physics
 */
class T_DLLCLASS DynamicBody : public Body
{
	T_RTTI_CLASS;

public:
	virtual void reset() = 0;

	virtual void setMass(float mass, const Vector4& inertiaTensor) = 0;

	virtual float getInverseMass() const = 0;

	virtual Matrix33 getInertiaTensorInverseWorld() const = 0;

	virtual void addForceAt(const Vector4& at, const Vector4& force, bool localSpace) = 0;

	virtual void addTorque(const Vector4& torque, bool localSpace) = 0;

	virtual void addLinearImpulse(const Vector4& linearImpulse, bool localSpace) = 0;

	virtual void addAngularImpulse(const Vector4& angularImpulse, bool localSpace) = 0;

	virtual void addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace) = 0;

	virtual void setLinearVelocity(const Vector4& linearVelocity) = 0;

	virtual Vector4 getLinearVelocity() const = 0;

	virtual void setAngularVelocity(const Vector4& angularVelocity) = 0;

	virtual Vector4 getAngularVelocity() const = 0;

	virtual Vector4 getVelocityAt(const Vector4& at, bool localSpace) const = 0;

	virtual bool setState(const DynamicBodyState& state) = 0;

	virtual DynamicBodyState getState() const = 0;

	/*! \brief State management. */
	//@{

	void setPreviousState(const DynamicBodyState& state);

	const DynamicBodyState& getPreviousState() const;

	//@}

private:
	DynamicBodyState m_previousState;
};

	}
}

#endif	// traktor_physics_DynamicBody_H
