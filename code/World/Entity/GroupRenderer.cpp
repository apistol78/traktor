/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/WorldBuildContext.h"
#include "World/WorldSetupContext.h"
#include "World/Entity.h"
#include "World/Entity/GroupComponent.h"
#include "World/Entity/GroupRenderer.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.GroupRenderer", GroupRenderer, IEntityRenderer)

const TypeInfoSet GroupRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< GroupComponent >();
}

void GroupRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void GroupRenderer::setup(
	const WorldSetupContext& context
)
{
}

void GroupRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
}

void GroupRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

}
