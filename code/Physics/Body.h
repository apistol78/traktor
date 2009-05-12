#ifndef traktor_physics_Body_H
#define traktor_physics_Body_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Math/Matrix44.h"

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

class CollisionListener;
struct CollisionInfo;

/*! \brief Rigid body.
 * \ingroup Physics
 */
class T_DLLCLASS Body : public Object
{
	T_RTTI_CLASS(Body)

public:
	virtual void destroy() = 0;

	virtual void setTransform(const Matrix44& transform) = 0;

	virtual Matrix44 getTransform() const = 0;

	virtual void setEnable(bool enable) = 0;

	virtual bool getEnable() const = 0;

	void addCollisionListener(CollisionListener* collisionListener);

	void removeCollisionListener(CollisionListener* collisionListener);

	void notifyCollisionListeners(const CollisionInfo& collisionInfo);

	void setUserObject(Object* userObject);

	Object* getUserObject() const;

private:
	RefArray< CollisionListener > m_collisionListeners;
	Ref< Object > m_userObject;
};

	}
}

#endif	// traktor_physics_Body_H
