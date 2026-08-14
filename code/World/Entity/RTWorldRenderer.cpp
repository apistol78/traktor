/*
 * TRAKTOR
 * Copyright (c) 2024-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/RTWorldRenderer.h"

#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IAccelerationStructure.h"
#include "Render/IRenderView.h"
#include "World/Entity/RTWorldComponent.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

#include <utility>

namespace traktor::world
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.RTWorldRenderer", 0, RTWorldRenderer, IEntityRenderer)

bool RTWorldRenderer::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet RTWorldRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< RTWorldComponent >();
}

void RTWorldRenderer::setup(
	const WorldSetupContext& context,
	const WorldRenderView& worldRenderView,
	const AlignedVector< Object* >& renderables)
{
	const Vector4 eyePosition = worldRenderView.getEyePosition();
	const float farDistance = worldRenderView.getViewFrustum().getFarZ();

	// Build the top level structure on the asynchronous compute queue. It reads
	// the bottom level structures whose producers output the acceleration
	// structure dependency; passes tracing rays against the world consume the
	// RT world dependency and the render graph synchronizes the graphics queue
	// before the first of them.
	Ref< render::RenderPass > rp = new render::RenderPass(L"RT world setup", render::RenderPass::Queue::AsyncCompute);
	rp->addInput(render::RGDependency::First);
	rp->addInput(context.getAccelerationStructureDependency());
	rp->setOutput(context.getRTWorldDependency());
	rp->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
		const bool asynchronous = renderContext->isAsyncCompute();
		for (Object* renderable : renderables)
		{
			auto rtWorldComponent = static_cast< RTWorldComponent* >(renderable);

			// Gather here, while the frame is being built, and not from the render block.
			AlignedVector< render::IAccelerationStructure::Instance > instances;
			Ref< render::IAccelerationStructure > tlas = rtWorldComponent->gatherTopLevelInstances(eyePosition, farDistance, instances);
			if (!tlas)
				continue;

			auto rb = renderContext->allocNamed< render::LambdaRenderBlock >(L"RTWorldRenderer");
			rb->lambda = [tlas, instances = std::move(instances), asynchronous](render::IRenderView* renderView) {
				renderView->writeAccelerationStructure(tlas, instances, asynchronous);
			};
			renderContext->compute(rb);
		}
	});
	context.getRenderGraph().addPass(rp);
}

void RTWorldRenderer::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass,
	const AlignedVector< Object* >& renderables)
{
}

}
