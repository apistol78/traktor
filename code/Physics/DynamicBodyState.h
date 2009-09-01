#ifndef traktor_physics_DynamicBodyState_H
#define traktor_physics_DynamicBodyState_H

#include "Core/Serialization/Serializable.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector4.h"

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

/*! \brief Dynamic rigid body state.
 * \ingroup Physics
 */
class T_DLLCLASS DynamicBodyState : public Serializable
{
	T_RTTI_CLASS(DynamicBodyState)

public:
	DynamicBodyState();

	void setTransform(const Transform& transform);

	const Transform& getTransform() const;

	void setLinearVelocity(const Vector4& velocity);

	const Vector4& getLinearVelocity() const;

	void setAngularVelocity(const Vector4& velocity);

	const Vector4& getAngularVelocity() const;

	DynamicBodyState interpolate(const DynamicBodyState& stateTarget, const Scalar& interpolate) const;

	virtual bool serialize(Serializer& s);

private:
	Transform m_transform;
	Vector4 m_linearVelocity;
	Vector4 m_angularVelocity;
};

	}
}

#endif	// traktor_physics_DynamicBodyState_H
