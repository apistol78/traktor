/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"
#include "Physics/CollisionSpecification.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Shape/Editor/Solid/PrimitiveEntity.h"
#include "Shape/Editor/Solid/SolidEntity.h"
#include "Shape/Editor/Solid/SolidEntityData.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
    namespace shape
    {
		namespace
		{
		
const resource::Id< render::Shader > c_defaultShader(Guid(L"{F01DE7F1-64CE-4613-9A17-899B44D5414E}"));
const resource::Id< physics::CollisionSpecification > c_defaultCollision(Guid(L"{F9805131-50C2-504C-9421-13C99E44616C}"));
const resource::Id< physics::CollisionSpecification > c_interactableCollision(Guid(L"{09CB1141-1924-3349-934A-CEB9728D7A61}"));

		}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.SolidEntityData", 2, SolidEntityData, world::EntityData)

SolidEntityData::SolidEntityData()
{
	m_collisionGroup.insert(c_defaultCollision);
	m_collisionMask.insert(c_defaultCollision);
	m_collisionMask.insert(c_interactableCollision);
}

Ref< SolidEntity > SolidEntityData::createEntity(const world::IEntityBuilder* builder, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(c_defaultShader, shader))
		return nullptr;

    Ref< SolidEntity > solidEntity = new SolidEntity(renderSystem, shader, getTransform());

	// Instantiate all components.
	RefArray< world::IEntityComponent > components;
	for (auto componentData : getComponents())
	{
		Ref< world::IEntityComponent > component = builder->create(componentData);
		if (!component)
			continue;
		solidEntity->setComponent(component);
	}

	return solidEntity;
}

void SolidEntityData::setCollisionGroup(const std::set< resource::Id< physics::CollisionSpecification > >& collisionGroup)
{
	m_collisionGroup = collisionGroup;
}

const std::set< resource::Id< physics::CollisionSpecification > >& SolidEntityData::getCollisionGroup() const
{
	return m_collisionGroup;
}

void SolidEntityData::setCollisionMask(const std::set< resource::Id< physics::CollisionSpecification > >& collisionMask)
{
	m_collisionMask = collisionMask;
}

const std::set< resource::Id< physics::CollisionSpecification > >& SolidEntityData::getCollisionMask() const
{
	return m_collisionMask;
}

void SolidEntityData::serialize(ISerializer& s)
{
	T_FATAL_ASSERT(s.getVersion< SolidEntityData >() >= 2);

	world::EntityData::serialize(s);

    s >> MemberStlSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"collisionGroup", m_collisionGroup);
    s >> MemberStlSet< resource::Id< physics::CollisionSpecification >, resource::Member< physics::CollisionSpecification > >(L"collisionMask", m_collisionMask);
}

    }
}