/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/WorldBuildContext.h"
#include "World/Entity/VolumetricFogComponent.h"
#include "World/Entity/VolumetricFogRenderer.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.VolumetricFogRenderer", VolumetricFogRenderer, IEntityRenderer)

const TypeInfoSet VolumetricFogRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< VolumetricFogComponent >();
}

void VolumetricFogRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void VolumetricFogRenderer::setup(
	const WorldSetupContext& context
)
{
}

void VolumetricFogRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto volumetricFogComponent = static_cast< VolumetricFogComponent* >(renderable);
	volumetricFogComponent->build(context, worldRenderView, worldRenderPass);
}

void VolumetricFogRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

}
