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
	render::RenderGraph& renderGraph = context.getRenderGraph();

	// Skin all components on the asynchronous compute queue; the render graph
	// synchronizes the graphics queue before the first pass drawing the results.
	const render::RGDependency skinDependency = renderGraph.addDependency();

	Ref< render::RenderPass > skinPass = new render::RenderPass(L"Skinned mesh skin", render::RenderPass::Queue::AsyncCompute);
	skinPass->addInput(render::RGDependency::First);
	skinPass->setOutput(skinDependency);
	skinPass->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
		m_ranked.resize(0);
		m_ranked.reserve(renderables.size());

		for (Object* renderable : renderables)
		{
			SkinnedMeshComponent* meshComponent = static_cast< SkinnedMeshComponent* >(renderable);
			const Transform& worldTransform = meshComponent->getTransform().get();
			const Scalar radius = meshComponent->getBoundingBox().getExtent().length() * 0.75_simd;
			const Scalar distance = dot3(worldRenderView.getEyeDirection(), worldTransform.translation() - worldRenderView.getEyePosition());
			if (distance >= -radius)
				m_ranked.push_back({ distance, meshComponent });
			else
				m_ranked.push_back({ std::numeric_limits< float >::max(), meshComponent });
		}

		std::sort(m_ranked.begin(), m_ranked.end(), [](const Ranked& lh, const Ranked& rh) {
			return lh.distance < rh.distance;
		});

		for (int32_t i = 0; i < (int32_t)m_ranked.size(); ++i)
			m_ranked[i].meshComponent->setupSkin(worldRenderView, renderContext, i);
	});
	renderGraph.addPass(skinPass);

	// All passes drawing the gathered entities consume the skinning results.
	context.addSetupAttachment(skinDependency);

	// Update the ray tracing acceleration structures from the skinning results,
	// also on the asynchronous compute queue. The top level structure build
	// consumes the acceleration structure dependency.
	bool haveAccelerationStructures = false;
	for (Object* renderable : renderables)
		if (static_cast< SkinnedMeshComponent* >(renderable)->haveAccelerationStructure())
		{
			haveAccelerationStructures = true;
			break;
		}

	if (haveAccelerationStructures)
	{
		Ref< render::RenderPass > blasPass = new render::RenderPass(L"Skinned mesh AS", render::RenderPass::Queue::AsyncCompute);
		blasPass->addInput(render::RGDependency::First);
		blasPass->addInput(skinDependency);
		blasPass->setOutput(context.getAccelerationStructureDependency());
		blasPass->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
			for (int32_t i = 0; i < (int32_t)m_ranked.size(); ++i)
				m_ranked[i].meshComponent->setupAccelerationStructure(worldRenderView, renderContext, i);
		});
		renderGraph.addPass(blasPass);
	}
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
