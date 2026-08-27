/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/DisplacementRenderer.h"

#include "World/Entity/DisplacementEntityComponent.h"
#include "World/Entity/DisplacementWorldComponent.h"
#include "World/World.h"
#include "World/WorldSetupContext.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.DisplacementRenderer", 0, DisplacementRenderer, IEntityRenderer)

bool DisplacementRenderer::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet DisplacementRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< DisplacementEntityComponent >();
}

void DisplacementRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	const AlignedVector< Object* >& renderables)
{
	DisplacementWorldComponent* displacement = context.getWorld()->getComponent< DisplacementWorldComponent >();
	if (!displacement)
		return;

	displacement->setup(context.getRenderGraph(), worldRenderView, renderables);
}

void DisplacementRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	const AlignedVector< Object* >& renderables)
{
}

}
