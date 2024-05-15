/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/NavMesh.h"
#include "Ai/NavMeshComponent.h"
#include "Ai/NavMeshComponentData.h"
#include "Ai/NavMeshEntityFactory.h"
#include "Core/Misc/ObjectStore.h"
#include "Resource/IResourceManager.h"

namespace traktor::ai
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.NavMeshEntityFactory", 0, NavMeshEntityFactory, world::AbstractEntityFactory)

NavMeshEntityFactory::NavMeshEntityFactory(bool suppress)
:	m_suppress(suppress)
{
}

bool NavMeshEntityFactory::initialize(const ObjectStore& objectStore)
{
	m_resourceManager = objectStore.get< resource::IResourceManager >();
	return true;
}

const TypeInfoSet NavMeshEntityFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet< NavMeshComponentData >();
}

Ref< world::IEntityComponent > NavMeshEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	auto navMeshComponentData = checked_type_cast<const NavMeshComponentData*>(&entityComponentData);

	resource::Proxy< NavMesh > navMesh;
	if (!m_resourceManager->bind(navMeshComponentData->get(), navMesh))
		return nullptr;

	return new NavMeshComponent(navMesh);
}

}
