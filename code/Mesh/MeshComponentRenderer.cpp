/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/MeshComponent.h"
#include "Mesh/MeshComponentRenderer.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshComponentRenderer", 0, MeshComponentRenderer, world::IEntityRenderer)

bool MeshComponentRenderer::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet MeshComponentRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< MeshComponent >();
}

void MeshComponentRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void MeshComponentRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void MeshComponentRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto meshComponent = static_cast< MeshComponent* >(renderable);
	meshComponent->build(context, worldRenderView, worldRenderPass);
}

void MeshComponentRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

}
