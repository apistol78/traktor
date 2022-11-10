/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Boids/BoidsComponent.h"
#include "Animation/Boids/BoidsRenderer.h"
#include "World/Entity.h"
#include "World/WorldGatherContext.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.BoidsRenderer", BoidsRenderer, world::IEntityRenderer)

const TypeInfoSet BoidsRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< BoidsComponent >();
}

void BoidsRenderer::gather(
	const world::WorldGatherContext& context,
	Object* renderable
)
{
	auto boidsComponent = static_cast< BoidsComponent* >(renderable);
	for (auto entity : boidsComponent->getEntities())
		context.gather(entity);
}

void BoidsRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void BoidsRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void BoidsRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
}

void BoidsRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
