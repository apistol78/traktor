/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/FogComponent.h"
#include "World/Entity/FogRenderer.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.FogRenderer", 0, FogRenderer, IEntityRenderer)

bool FogRenderer::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet FogRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< FogComponent >();
}

void FogRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void FogRenderer::setup(
	const WorldSetupContext& context
)
{
}

void FogRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto fogComponent = static_cast< FogComponent* >(renderable);
	fogComponent->build(context, worldRenderView, worldRenderPass);
}

void FogRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

}
