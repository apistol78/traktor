/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Skinned/SkinnedMeshComponentRenderer.h"

#include "Mesh/Skinned/SkinnedMeshComponent.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderView.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.SkinnedMeshComponentRenderer", 0, SkinnedMeshComponentRenderer, world::IEntityRenderer)

bool SkinnedMeshComponentRenderer::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet SkinnedMeshComponentRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< SkinnedMeshComponent >();
}

void SkinnedMeshComponentRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	const AlignedVector< Object* >& renderables)
{
	Ref< render::RenderPass > rp = new render::RenderPass(L"Skinned mesh setup");
	rp->addInput(render::RGDependency::First);
	rp->addBuild([=](const render::RenderGraph&, render::RenderContext* renderContext) {
		bool needSynchronization = false;

		// Setup all skinned meshes; compute new skin and BLAS if necessary.
		for (Object* renderable : renderables)
		{
			SkinnedMeshComponent* meshComponent = static_cast< SkinnedMeshComponent* >(renderable);
			needSynchronization |= meshComponent->setup(worldRenderView, renderContext);
		}

		// Synchronize the async compute skinning and RT jobs with the graphics queue.
		if (needSynchronization)
		{
			render::ComputeHandle* handle = renderContext->alloc< render::ComputeHandle >();

			renderContext->compute< render::LambdaRenderBlock >([=](render::IRenderView* renderView) {
				*handle = renderView->signalAsynchronousCompute();
			});

			// #todo This should be moved closer to the consumer pass to increase overlap.
			renderContext->compute< render::LambdaRenderBlock >([=](render::IRenderView* renderView) {
				renderView->waitAsynchronousCompute(*handle);
			});

			// renderContext->compute< render::SynchronizeRenderBlock >();
		}
	});
	context.getRenderGraph().addPass(rp);
}

void SkinnedMeshComponentRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	const AlignedVector< Object* >& renderables)
{
	for (Object* renderable : renderables)
	{
		SkinnedMeshComponent* meshComponent = static_cast< SkinnedMeshComponent* >(renderable);
		meshComponent->build(context, worldRenderView, worldRenderPass);
	}
}

}
