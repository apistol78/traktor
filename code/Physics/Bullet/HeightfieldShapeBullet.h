#ifndef traktor_physics_HeightfieldShapeBullet_H
#define traktor_physics_HeightfieldShapeBullet_H

#include <BulletCollision/CollisionShapes/btConcaveShape.h>
#include "Resource/Proxy.h"

namespace traktor
{
	namespace physics
	{

class Heightfield;

/*!
 * \ingroup Bullet
 */
class HeightfieldShapeBullet : public btConcaveShape
{
public:
	HeightfieldShapeBullet(const resource::Proxy< Heightfield >& heightfield);

	virtual ~HeightfieldShapeBullet();

	virtual void setLocalScaling(const btVector3& scaling);

	virtual const btVector3& getLocalScaling() const;

	virtual void getAabb(const btTransform& t, btVector3& aabbMin, btVector3& aabbMax) const;

	virtual void processAllTriangles(btTriangleCallback* callback, const btVector3& aabbMin, const btVector3& aabbMax) const;

	virtual void calculateLocalInertia(btScalar mass, btVector3& inertia) const;

	virtual const char*	getName() const;

private:
	mutable resource::Proxy< Heightfield > m_heightfield;
	btVector3 m_localScaling;
};

	}
}

#endif	// traktor_physics_HeightfieldShapeBullet_H
