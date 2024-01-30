/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Weather/Precipitation/PrecipitationComponent.h"
#include "Weather/Precipitation/PrecipitationRenderer.h"
#include "World/WorldBuildContext.h"

namespace traktor::weather
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.PrecipitationRenderer", PrecipitationRenderer, world::IEntityRenderer)

const TypeInfoSet PrecipitationRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< PrecipitationComponent >();
}

void PrecipitationRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void PrecipitationRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void PrecipitationRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto precipitationComponent = static_cast< PrecipitationComponent* >(renderable);
	precipitationComponent->build(context, worldRenderView, worldRenderPass);
}

void PrecipitationRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

}
