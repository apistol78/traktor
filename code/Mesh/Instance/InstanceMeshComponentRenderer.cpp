/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Instance/InstanceMeshComponentRenderer.h"

#include "Mesh/Instance/InstanceMeshComponent.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.InstanceMeshComponentRenderer", 0, InstanceMeshComponentRenderer, world::IEntityRenderer)

bool InstanceMeshComponentRenderer::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet InstanceMeshComponentRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< InstanceMeshComponent >();
}

void InstanceMeshComponentRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	const AlignedVector< Object* >& renderables)
{
	for (Object* renderable : renderables)
	{
		InstanceMeshComponent* meshComponent = static_cast< InstanceMeshComponent* >(renderable);
		meshComponent->setup(context, worldRenderView);
	}
}

void InstanceMeshComponentRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	const AlignedVector< Object* >& renderables)
{
}

}
