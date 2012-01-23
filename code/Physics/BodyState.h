#ifndef traktor_physics_BodyState_H
#define traktor_physics_BodyState_H

#include "Core/Serialization/ISerializable.h"
#include "Core/Math/Transform.h"
#include "Core/Math/Vector4.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! \brief Rigid body state.
 * \ingroup Physics
 */
class T_DLLCLASS BodyState : public ISerializable
{
	T_RTTI_CLASS;

public:
	BodyState();

	void setTransform(const Transform& transform);

	const Transform& getTransform() const;

	void setLinearVelocity(const Vector4& velocity);

	const Vector4& getLinearVelocity() const;

	void setAngularVelocity(const Vector4& velocity);

	const Vector4& getAngularVelocity() const;

	BodyState interpolate(const BodyState& stateTarget, const Scalar& interpolate) const;

	virtual bool serialize(ISerializer& s);

private:
	Transform m_transform;
	Vector4 m_linearVelocity;
	Vector4 m_angularVelocity;
};

	}
}

#endif	// traktor_physics_BodyState_H
