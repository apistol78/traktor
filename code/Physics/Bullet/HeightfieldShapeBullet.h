/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	virtual void setLocalScaling(const btVector3& scaling) override final;

	virtual const btVector3& getLocalScaling() const override final;

	virtual void getAabb(const btTransform& t, btVector3& aabbMin, btVector3& aabbMax) const override final;

	virtual void processAllTriangles(btTriangleCallback* callback, const btVector3& aabbMin, const btVector3& aabbMax) const override final;

	virtual void calculateLocalInertia(btScalar mass, btVector3& inertia) const override final;

	virtual const char*	getName() const override final;

private:
	resource::Proxy< hf::Heightfield > m_heightfield;
	btVector3 m_localScaling;
	bool m_haveCuts;
};

	}
}

#endif	// traktor_physics_HeightfieldShapeBullet_H
