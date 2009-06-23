#ifndef traktor_physics_StaticBodyHavok_H
#define traktor_physics_StaticBodyHavok_H

#include "Physics/StaticBody.h"
#include "Physics/Havok/BodyHavok.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_HAVOK_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup Havok
 */
class T_DLLCLASS StaticBodyHavok : public BodyHavok< StaticBody >
{
	T_RTTI_CLASS(StaticBodyHavok)

public:
	StaticBodyHavok(DestroyCallbackHavok* callback, const HvkRef< hkpRigidBody >& rigidBody);

	virtual void setTransform(const Matrix44& transform);

	virtual Matrix44 getTransform() const;

	virtual void setActive(bool active);

	virtual bool isActive() const;

	virtual void setEnable(bool enable);

	virtual bool isEnable() const;
};

	}
}

#endif	// traktor_physics_StaticBodyHavok_H
