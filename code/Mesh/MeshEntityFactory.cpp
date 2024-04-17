/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/MeshComponent.h"
#include "Mesh/MeshComponentData.h"
#include "Mesh/MeshParameterComponent.h"
#include "Mesh/MeshParameterComponentData.h"
#include "Mesh/MeshEntityFactory.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshEntityFactory", MeshEntityFactory, world::AbstractEntityFactory)

MeshEntityFactory::MeshEntityFactory(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
{
}

const TypeInfoSet MeshEntityFactory::getEntityComponentTypes() const
{
	return makeTypeInfoSet< MeshComponentData, MeshParameterComponentData >();
}

Ref< world::IEntityComponent > MeshEntityFactory::createEntityComponent(const world::IEntityBuilder* builder, const world::IEntityComponentData& entityComponentData) const
{
	if (auto meshComponentData = dynamic_type_cast< const MeshComponentData* >(&entityComponentData))
		return meshComponentData->createComponent(m_resourceManager, m_renderSystem);
	else if (auto meshParameterComponentData = dynamic_type_cast< const MeshParameterComponentData* >(&entityComponentData))
		return meshParameterComponentData->createComponent(m_resourceManager);
	else
		return nullptr;
}

}
