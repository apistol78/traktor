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

#include <algorithm>

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
	rp->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
		m_ranked.resize(0);
		m_ranked.reserve(renderables.size());

		for (Object* renderable : renderables)
		{
			SkinnedMeshComponent* meshComponent = static_cast< SkinnedMeshComponent* >(renderable);
			const Transform& worldTransform = meshComponent->getTransform().get();

			const Scalar distance = dot3(worldRenderView.getEyeDirection(), worldTransform.translation() - worldRenderView.getEyePosition());
			if (distance >= 0.0_simd)
				m_ranked.push_back({ distance, meshComponent });
			else
				m_ranked.push_back({ std::numeric_limits< float >::max(), meshComponent });
		}

		std::sort(m_ranked.begin(), m_ranked.end(), [](const Ranked& lh, const Ranked& rh) {
			return lh.distance < rh.distance;
		});

		// Setup all skinned meshes; skin for every component first, then the ray tracing acceleration structures.
		bool needSynchronization = false;
		for (int32_t i = 0; i < (int32_t)m_ranked.size(); ++i)
			needSynchronization |= m_ranked[i].meshComponent->setupSkin(worldRenderView, renderContext, i);
		for (int32_t i = 0; i < (int32_t)m_ranked.size(); ++i)
			needSynchronization |= m_ranked[i].meshComponent->setupAccelerationStructure(worldRenderView, renderContext, i);

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
