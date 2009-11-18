#ifndef traktor_physics_StaticBodyPhysX_H
#define traktor_physics_StaticBodyPhysX_H

#include "Physics/StaticBody.h"
#include "Physics/PhysX/BodyPhysX.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_PHYSX_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*!
 * \ingroup PhysX
 */
class T_DLLCLASS StaticBodyPhysX : public BodyPhysX< StaticBody >
{
	T_RTTI_CLASS;

public:
	StaticBodyPhysX(DestroyCallbackPhysX* callback, NxActor* actor);

	virtual void setTransform(const Transform& transform);

	virtual Transform getTransform() const;

	virtual void setActive(bool active);

	virtual bool isActive() const;

	virtual void setEnable(bool enable);

	virtual bool isEnable() const;
};

	}
}

#endif	// traktor_physics_StaticBodyPhysX_H
