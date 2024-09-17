/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberSmallSet.h"
#include "Physics/CollisionSpecification.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Shape/Editor/Solid/SolidComponent.h"
#include "Shape/Editor/Solid/SolidComponentData.h"
#include "World/IEntityBuilder.h"

namespace traktor::shape
{
	namespace
	{
		
const resource::Id< render::Shader > c_defaultShader(Guid(L"{F01DE7F1-64CE-4613-9A17-899B44D5414E}"));
const resource::Id< physics::CollisionSpecification > c_defaultCollision(Guid(L"{F9805131-50C2-504C-9421-13C99E44616C}"));
const resource::Id< physics::CollisionSpecification > c_interactableCollision(Guid(L"{09CB1141-1924-3349-934A-CEB9728D7A61}"));

	}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.SolidComponentData", 0, SolidComponentData, world::IEntityComponentData)

SolidComponentData::SolidComponentData()
{
	m_collisionGroup.insert(c_defaultCollision);
	m_collisionMask.insert(c_defaultCollision);
	m_collisionMask.insert(c_interactableCollision);
}

Ref< SolidComponent > SolidComponentData::createComponent(const world::IEntityBuilder* builder, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< render::Shader > shader;
	if (resourceManager->bind(c_defaultShader, shader))
		return new SolidComponent(renderSystem, shader);
	else
		return nullptr;
}

int32_t SolidComponentData::getOrdinal() const
{
	return 0;
}

void SolidComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void SolidComponentData::setCollisionGroup(const SmallSet< resource::Id< physics::CollisionSpecification > >& collisionGroup)
{
	m_collisionGroup = collisionGroup;
}

const SmallSet< resource::Id< physics::CollisionSpecification > >& SolidComponentData::getCollisionGroup() const
{
	return m_collisionGroup;
}

void SolidComponentData::setCollisionMask(const SmallSet< resource::Id< physics::CollisionSpecification > >& collisionMask)
{
	m_collisionMask = collisionMask;
}

const SmallSet< resource::Id< physics::CollisionSpecification > >& SolidComponentData::getCollisionMask() const
{
	return m_collisionMask;
}

void SolidComponentData::serialize(ISerializer& s)
{
	s >> MemberSmallSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"collisionGroup", m_collisionGroup);
	s >> MemberSmallSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"collisionMask", m_collisionMask);
}

}
