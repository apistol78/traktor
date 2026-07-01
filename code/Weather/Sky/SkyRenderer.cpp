/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.weather.SkyRenderer", 0, SkyRenderer, world::IEntityRenderer)

bool SkyRenderer::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet SkyRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< SkyComponent >();
}

void SkyRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	const AlignedVector< Object* >& renderables
)
{
	for (Object* renderable : renderables)
	{
		auto skyComponent = static_cast< SkyComponent* >(renderable);
		skyComponent->setup(context, worldRenderView);
	}
}

void SkyRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	const AlignedVector< Object* >& renderables
)
{
	for (Object* renderable : renderables)
	{
		auto skyComponent = static_cast< SkyComponent* >(renderable);
		skyComponent->build(context.getRenderContext(), worldRenderView, worldRenderPass);
	}
}

}
