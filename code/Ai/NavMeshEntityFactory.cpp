/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/NavMesh.h"
#include "Ai/NavMeshComponent.h"
#include "Ai/NavMeshComponentData.h"
#include "Ai/NavMeshEntityFactory.h"
#include "Resource/IResourceManager.h"

namespace traktor::ai
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ai.NavMeshEntityFactory", NavMeshEntityFactory, world::IEntityFactory)

NavMeshEntityFactory::NavMeshEntityFactory(resource::IResourceManager* resourceManager, bool suppress)
:	m_resourceManager(resourceManager)
,	m_suppress(suppress)
{
}

const TypeInfoSet NavMeshEntityFactory::getEntityTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet NavMeshEntityFactory::getEntityEventTypes() const
{
	return TypeInfoSet();
}

const TypeInfoSet NavMeshEntityFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet< NavMeshComponentData >();
}

Ref< world::Entity > NavMeshEntityFactory::createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const
{
	return nullptr;
}

Ref< world::IEntityEvent > NavMeshEntityFactory::createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const
{
	return nullptr;
}

Ref< world::IEntityComponent > NavMeshEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (!m_suppress)
	{
		auto navMeshComponentData = checked_type_cast< const NavMeshComponentData* >(&entityComponentData);

		resource::Proxy< NavMesh > navMesh;
		if (!m_resourceManager->bind(navMeshComponentData->get(), navMesh))
			return nullptr;

		return new NavMeshComponent(navMesh);
	}
	else
		return new NavMeshComponent();
}

}
