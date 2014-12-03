#ifndef traktor_physics_HeightfieldShapeBullet_H
#define traktor_physics_HeightfieldShapeBullet_H

#include <BulletCollision/CollisionShapes/btConcaveShape.h>
#include "Core/Misc/AutoPtr.h"
#include "Resource/Proxy.h"

namespace traktor
{
	namespace hf
	{

class Heightfield;

	}

	namespace physics
	{

/*!
 * \ingroup Bullet
 */
class HeightfieldShapeBullet : public btConcaveShape
{
public:
	HeightfieldShapeBullet(const resource::Proxy< hf::Heightfield >& heightfield);

	virtual ~HeightfieldShapeBullet();

	virtual void setLocalScaling(const btVector3& scaling);

	virtual const btVector3& getLocalScaling() const;

	virtual void getAabb(const btTransform& t, btVector3& aabbMin, btVector3& aabbMax) const;

	virtual void processAllTriangles(btTriangleCallback* callback, const btVector3& aabbMin, const btVector3& aabbMax) const;

#if !defined(_PS3)

	virtual void processRaycastAllTriangles(btTriangleRaycastCallback *callback, const btVector3 &raySource, const btVector3 &rayTarget);

	virtual void processConvexcastAllTriangles(btTriangleConvexcastCallback *callback, const btVector3 &boxSource, const btVector3 &boxTarget, const btVector3 &boxMin, const btVector3 &boxMax);

#endif

	virtual void calculateLocalInertia(btScalar mass, btVector3& inertia) const;

	virtual const char*	getName() const;

private:
	struct MinMax
	{
		float mn;
		float mx;
	};

	resource::Proxy< hf::Heightfield > m_heightfield;
	AutoArrayPtr< MinMax > m_gridMinMax;
	btVector3 m_localScaling;
};

	}
}

#endif	// traktor_physics_HeightfieldShapeBullet_H
