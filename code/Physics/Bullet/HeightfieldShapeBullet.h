/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <BulletCollision/CollisionShapes/btConcaveShape.h>
#include "Core/Misc/AutoPtr.h"
#include "Resource/Proxy.h"

namespace traktor::hf
{

class Heightfield;

}

namespace traktor::physics
{

/*!
 * \ingroup Bullet
 */
class HeightfieldShapeBullet : public btConcaveShape
{
public:
	explicit HeightfieldShapeBullet(const resource::Proxy< hf::Heightfield >& heightfield);

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
