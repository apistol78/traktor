/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Instance/InstanceMeshComponentRenderer.h"

#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshComponent.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"

#include <algorithm>

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.InstanceMeshComponentRenderer", 0, InstanceMeshComponentRenderer, world::IEntityRenderer)

bool InstanceMeshComponentRenderer::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet InstanceMeshComponentRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< InstanceMeshComponent >();
}

void InstanceMeshComponentRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	const AlignedVector< Object* >& renderables)
{
	m_deform.resize(0);
	m_deformMeshes.resize(0);

	// Deform slots are acquired and released here, on the CPU; each deforming mesh
	// rebuilds its slot table, indexed by the instances' culling batch indices, so
	// the draws pick the deformed positions for the instances holding slots.
	AlignedVector< InstanceMesh* > deformTableMeshes;
	bool haveAccelerationStructures = false;
	for (Object* renderable : renderables)
	{
		InstanceMeshComponent* meshComponent = static_cast< InstanceMeshComponent* >(renderable);
		meshComponent->setup(context, worldRenderView);

		InstanceMesh* mesh = meshComponent->getMesh();
		if (mesh == nullptr || !mesh->haveDeform())
			continue;

		if (std::find(deformTableMeshes.begin(), deformTableMeshes.end(), mesh) == deformTableMeshes.end())
		{
			deformTableMeshes.push_back(mesh);
			mesh->beginDeformSlotTable();
		}

		const bool deform = meshComponent->setupDeform(worldRenderView);

		uint32_t batchIndex;
		if (meshComponent->getBatchIndex(batchIndex))
			mesh->setDeformSlotTableEntry(batchIndex, deform ? meshComponent->getDeformSlot() : -1);

		if (deform)
		{
			m_deform.push_back(meshComponent);
			if (std::find(m_deformMeshes.begin(), m_deformMeshes.end(), mesh) == m_deformMeshes.end())
				m_deformMeshes.push_back(mesh);
			haveAccelerationStructures |= (mesh->getAccelerationStructure() != nullptr);
		}
	}

	for (InstanceMesh* mesh : deformTableMeshes)
		mesh->endDeformSlotTable();

	if (m_deform.empty())
		return;

	render::RenderGraph& renderGraph = context.getRenderGraph();

	// Deform all instances holding slots on the asynchronous compute queue; the render
	// graph synchronizes the graphics queue before the first pass drawing the results.
	const render::RGDependency deformDependency = renderGraph.addDependency();

	Ref< render::RenderPass > deformPass = new render::RenderPass(L"Instance mesh deform", render::RenderPass::Queue::AsyncCompute);
	deformPass->addInput(render::RGDependency::First);
	deformPass->setOutput(deformDependency);
	deformPass->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
		for (InstanceMesh* mesh : m_deformMeshes)
			mesh->beginDeform();
		for (InstanceMeshComponent* meshComponent : m_deform)
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
		Ref< render::RenderPass > blasPass = new render::RenderPass(L"Instance mesh deform AS", render::RenderPass::Queue::AsyncCompute);
		blasPass->addInput(render::RGDependency::First);
		blasPass->addInput(deformDependency);
		blasPass->setOutput(context.getAccelerationStructureDependency());
		blasPass->addBuild([=, this](const render::RenderGraph&, render::RenderContext* renderContext) {
			for (InstanceMeshComponent* meshComponent : m_deform)
				meshComponent->buildDeformAccelerationStructure(worldRenderView, renderContext);
		});
		renderGraph.addPass(blasPass);
	}
}

void InstanceMeshComponentRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	const AlignedVector< Object* >& renderables)
{
}

}
