/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberSmallSet.h"
#include "Physics/CollisionSpecification.h"
#include "Resource/Member.h"
#include "Shape/Editor/Spline/SplineComponentData.h"

namespace traktor::shape
{
	namespace
	{
		
const resource::Id< physics::CollisionSpecification > c_defaultCollision(Guid(L"{F9805131-50C2-504C-9421-13C99E44616C}"));
const resource::Id< physics::CollisionSpecification > c_interactableCollision(Guid(L"{09CB1141-1924-3349-934A-CEB9728D7A61}"));

	}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.SplineComponentData", 0, SplineComponentData, world::IEntityComponentData)

SplineComponentData::SplineComponentData()
{
	m_collisionGroup.insert(c_defaultCollision);
	m_collisionMask.insert(c_defaultCollision);
	m_collisionMask.insert(c_interactableCollision);
}

void SplineComponentData::setCollisionGroup(const SmallSet< resource::Id< physics::CollisionSpecification > >& collisionGroup)
{
	m_collisionGroup = collisionGroup;
}

const SmallSet< resource::Id< physics::CollisionSpecification > >& SplineComponentData::getCollisionGroup() const
{
	return m_collisionGroup;
}

void SplineComponentData::setCollisionMask(const SmallSet< resource::Id< physics::CollisionSpecification > >& collisionMask)
{
	m_collisionMask = collisionMask;
}

const SmallSet< resource::Id< physics::CollisionSpecification > >& SplineComponentData::getCollisionMask() const
{
	return m_collisionMask;
}

int32_t SplineComponentData::getOrdinal() const
{
	return 0;
}

void SplineComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void SplineComponentData::serialize(ISerializer& s)
{
	s >> MemberSmallSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"collisionGroup", m_collisionGroup);
	s >> MemberSmallSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"collisionMask", m_collisionMask);
}

}
