/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Physics/CollisionSpecification.h"
#include "Resource/Member.h"
#include "Shape/Editor/Spline/SplineEntityData.h"

namespace traktor
{
	namespace shape
	{
		namespace
		{
		
const resource::Id< physics::CollisionSpecification > c_defaultCollision(Guid(L"{F9805131-50C2-504C-9421-13C99E44616C}"));
const resource::Id< physics::CollisionSpecification > c_interactableCollision(Guid(L"{09CB1141-1924-3349-934A-CEB9728D7A61}"));

		}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.SplineEntityData", 2, SplineEntityData, world::EntityData)

SplineEntityData::SplineEntityData()
{
	m_collisionGroup.insert(c_defaultCollision);
	m_collisionMask.insert(c_defaultCollision);
	m_collisionMask.insert(c_interactableCollision);
}

void SplineEntityData::setCollisionGroup(const std::set< resource::Id< physics::CollisionSpecification > >& collisionGroup)
{
	m_collisionGroup = collisionGroup;
}

const std::set< resource::Id< physics::CollisionSpecification > >& SplineEntityData::getCollisionGroup() const
{
	return m_collisionGroup;
}

void SplineEntityData::setCollisionMask(const std::set< resource::Id< physics::CollisionSpecification > >& collisionMask)
{
	m_collisionMask = collisionMask;
}

const std::set< resource::Id< physics::CollisionSpecification > >& SplineEntityData::getCollisionMask() const
{
	return m_collisionMask;
}

void SplineEntityData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion() >= 2);

	world::EntityData::serialize(s);

	s >> MemberStlSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"collisionGroup", m_collisionGroup);
	s >> MemberStlSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"collisionMask", m_collisionMask);
}

	}
}
