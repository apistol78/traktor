/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Static/StaticMeshComponentRenderer.h"

#include "Mesh/Static/StaticMeshComponent.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.StaticMeshComponentRenderer", 0, StaticMeshComponentRenderer, world::IEntityRenderer)

bool StaticMeshComponentRenderer::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet StaticMeshComponentRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< StaticMeshComponent >();
}

void StaticMeshComponentRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	const AlignedVector< Object* >& renderables)
{
	for (Object* renderable : renderables)
	{
		StaticMeshComponent* meshComponent = static_cast< StaticMeshComponent* >(renderable);
		meshComponent->setup(context, worldRenderView);
	}
}

void StaticMeshComponentRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	const AlignedVector< Object* >& renderables)
{
	for (Object* renderable : renderables)
	{
		StaticMeshComponent* meshComponent = static_cast< StaticMeshComponent* >(renderable);
		meshComponent->build(context, worldRenderView, worldRenderPass);
	}
}

}
