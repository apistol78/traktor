/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/MeshComponentRenderer.h"

#include "Mesh/MeshComponent.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderView.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshComponentRenderer", 0, MeshComponentRenderer, world::IEntityRenderer)

bool MeshComponentRenderer::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet MeshComponentRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< MeshComponent >();
}

void MeshComponentRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	const AlignedVector< Object* >& renderables)
{
	Ref< render::RenderPass > rp = new render::RenderPass(L"Mesh setup");
	rp->addInput(render::RGDependency::First);
	rp->addBuild([=](const render::RenderGraph&, render::RenderContext* renderContext) {
		for (Object* renderable : renderables)
		{
			MeshComponent* meshComponent = static_cast< MeshComponent* >(renderable);
			meshComponent->setup(worldRenderView, renderContext);
		}

		// Synchronize the async compute skinning and RT jobs with the graphics queue.
		render::ComputeHandle* handle = renderContext->alloc< render::ComputeHandle >();

		renderContext->compute< render::LambdaRenderBlock >([=](render::IRenderView* renderView) {
			*handle = renderView->signalAsynchronousCompute();
		});

		// #todo This should be moved closer to the consumer pass to increase overlap.
		renderContext->compute< render::LambdaRenderBlock >([=](render::IRenderView* renderView) {
			renderView->waitAsynchronousCompute(*handle);
		});

		//renderContext->compute< render::SynchronizeRenderBlock >();
	});
	context.getRenderGraph().addPass(rp);
}

void MeshComponentRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	const AlignedVector< Object* >& renderables)
{
	for (Object* renderable : renderables)
	{
		MeshComponent* meshComponent = static_cast< MeshComponent* >(renderable);
		meshComponent->build(context, worldRenderView, worldRenderPass);
	}
}

}
