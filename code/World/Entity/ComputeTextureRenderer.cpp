/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/ComputeTextureRenderer.h"

#include "Render/Compute/ComputeTexture.h"
#include "Render/Frame/RenderGraph.h"
#include "World/World.h"
#include "World/WorldSetupContext.h"
#include "World/Entity/ComputeTextureComponent.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.ComputeTextureRenderer", 0, ComputeTextureRenderer, IEntityRenderer)

bool ComputeTextureRenderer::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet ComputeTextureRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< ComputeTextureComponent >();
}

void ComputeTextureRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void ComputeTextureRenderer::setup(
	const WorldSetupContext& context
)
{
	const auto* computeTextureComponent = context.getWorld()->getComponent< ComputeTextureComponent >();
	if (!computeTextureComponent)
		return;

	for (auto texture : computeTextureComponent->getTextures())
	{
		const render::RGDependency dependency = texture->setup(context.getRenderGraph());
		if (dependency != render::RGDependency::Invalid)
			context.addVisualAttachment(dependency.get());
	}
}

void ComputeTextureRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
}

void ComputeTextureRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
}

}
