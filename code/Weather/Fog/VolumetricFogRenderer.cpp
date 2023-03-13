/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Weather/Fog/VolumetricFogComponent.h"
#include "Weather/Fog/VolumetricFogRenderer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldGatherContext.h"

namespace traktor::weather
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.VolumetricFogRenderer", VolumetricFogRenderer, world::IEntityRenderer)

const TypeInfoSet VolumetricFogRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< VolumetricFogComponent >();
}

void VolumetricFogRenderer::gather(
	const world::WorldGatherContext& context,
	Object* renderable
)
{
	context.include(this, renderable);
}

void VolumetricFogRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
	auto volumetricFogComponent = static_cast< VolumetricFogComponent* >(renderable);
	volumetricFogComponent->setup(context, worldRenderView);
}

void VolumetricFogRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void VolumetricFogRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto volumetricFogComponent = static_cast< VolumetricFogComponent* >(renderable);
	volumetricFogComponent->build(context, worldRenderView, worldRenderPass);
}

void VolumetricFogRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

}
