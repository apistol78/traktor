/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <set>
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
{

class CollisionSpecification;

}

namespace traktor::shape
{

/*!
 * \ingroup Shape
 */
class T_DLLCLASS SplineComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	SplineComponentData();

	void setCollisionGroup(const std::set< resource::Id< physics::CollisionSpecification > >& collisionGroup);

	const std::set< resource::Id< physics::CollisionSpecification > >& getCollisionGroup() const;

	void setCollisionMask(const std::set< resource::Id< physics::CollisionSpecification > >& collisionMask);

	const std::set< resource::Id< physics::CollisionSpecification > >& getCollisionMask() const;

	virtual int32_t getOrdinal() const override final;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::set< resource::Id< physics::CollisionSpecification > > m_collisionGroup;
	std::set< resource::Id< physics::CollisionSpecification > > m_collisionMask;
};

}
