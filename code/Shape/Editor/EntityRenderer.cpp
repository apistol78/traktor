/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Shape/Editor/EntityRenderer.h"
#include "Shape/Editor/Solid/SolidEntity.h"
#include "Shape/Editor/Spline/SplineEntity.h"
#include "World/WorldGatherContext.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.EntityRenderer", EntityRenderer, world::IEntityRenderer)

const TypeInfoSet EntityRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet<
		SolidEntity,
		SplineEntity
	>();
}

void EntityRenderer::gather(
	const world::WorldGatherContext& context,
	Object* renderable
)
{
	context.include(this, renderable);
}

void EntityRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void EntityRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void EntityRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	if (auto solidEntity = dynamic_type_cast< SolidEntity* >(renderable))
	{
		solidEntity->build(
			context,
			worldRenderView,
			worldRenderPass
		);
	}
	else if (auto splineEntity = dynamic_type_cast< SplineEntity* >(renderable))
	{
		splineEntity->build(
			context,
			worldRenderView,
			worldRenderPass
		);
	}
}

void EntityRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
