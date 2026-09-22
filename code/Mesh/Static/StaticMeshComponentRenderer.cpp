/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Static/StaticMeshComponentRenderer.h"

#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshComponent.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

#include <algorithm>

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.StaticMeshComponentRenderer", 0, StaticMeshComponentRenderer, world::IEntityRenderer)

bool StaticMeshComponentRenderer::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet StaticMeshComponentRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< StaticMeshComponent >();
}

void StaticMeshComponentRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	const AlignedVector< Object* >& renderables)
{
	m_deform.resize(0);
	m_deformMeshes.resize(0);

	// Deform slots are acquired and released here, on the CPU, as components cross
	// the deform distance.
	bool haveAccelerationStructures = false;
	for (Object* renderable : renderables)
	{
		StaticMeshComponent* meshComponent = static_cast< StaticMeshComponent* >(renderable);
		meshComponent->setup(context, worldRenderView);

		StaticMesh* mesh = meshComponent->getMesh();
		if (mesh != nullptr && mesh->haveDeform() && meshComponent->setupDeform(worldRenderView))
		{
			m_deform.push_back(meshComponent);
			if (std::find(m_deformMeshes.begin(), m_deformMeshes.end(), mesh) == m_deformMeshes.end())
				m_deformMeshes.push_back(mesh);
			haveAccelerationStructures |= meshComponent->haveAccelerationStructure();
		}
	}

	if (m_deform.empty())
		return;

	render::RenderGraph& renderGraph = context.getRenderGraph();

	// Deform all components holding slots on the asynchronous compute queue; the render
	// graph synchronizes the graphics queue before the first pass drawing the results.
	const render::RGDependency deformDependency = renderGraph.addDependency();

	Ref< render::RenderPass > deformPass = new render::RenderPass(L"Static mesh deform", render::RenderPass::Queue::AsyncCompute);
	deformPass->addInput(render::RGDependency::First);
	deformPass->setOutput(deformDependency);
	deformPass->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
		for (StaticMesh* mesh : m_deformMeshes)
			mesh->beginDeform();
		for (StaticMeshComponent* meshComponent : m_deform)
			meshComponent->buildDeform(worldRenderView, renderContext);
	});
	renderGraph.addPass(deformPass);

	// All passes drawing the gathered entities consume the deform results.
	context.addSetupAttachment(deformDependency);

	// Update the slot acceleration structures from the deform results, also on the
	// asynchronous compute queue. The top level structure build consumes the
	// acceleration structure dependency.
	if (haveAccelerationStructures)
	{
		Ref< render::RenderPass > blasPass = new render::RenderPass(L"Static mesh deform AS", render::RenderPass::Queue::AsyncCompute);
		blasPass->addInput(render::RGDependency::First);
		blasPass->addInput(deformDependency);
		blasPass->setOutput(context.getAccelerationStructureDependency());
		blasPass->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
			for (StaticMeshComponent* meshComponent : m_deform)
				meshComponent->buildDeformAccelerationStructure(worldRenderView, renderContext);
		});
		renderGraph.addPass(blasPass);
	}
}

void StaticMeshComponentRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	const AlignedVector< Object* >& renderables)
{
	for (Object* renderable : renderables)
	{
		StaticMeshComponent* meshComponent = static_cast< StaticMeshComponent* >(renderable);
		meshComponent->build(context, worldRenderView, worldRenderPass);
	}
}

}
