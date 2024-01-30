/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Weather/Sky/SkyComponent.h"
#include "Weather/Sky/SkyRenderer.h"
#include "World/WorldBuildContext.h"

namespace traktor::weather
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.SkyRenderer", SkyRenderer, world::IEntityRenderer)

const TypeInfoSet SkyRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< SkyComponent >();
}

void SkyRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
	auto skyComponent = static_cast< SkyComponent* >(renderable);
	skyComponent->setup(context, worldRenderView);
}

void SkyRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void SkyRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto skyComponent = static_cast< SkyComponent* >(renderable);
	skyComponent->build(context.getRenderContext(), worldRenderView, worldRenderPass);
}

void SkyRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

}
