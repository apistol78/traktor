/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshData.h"
#include "Render/Buffer.h"
#include "Render/IProgram.h"
#include "Render/IRenderSystem.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{
	namespace
	{

render::Handle s_handleInstanceWorld(L"InstanceWorld");
render::Handle s_handleInstanceWorldLast(L"InstanceWorldLast");
render::Handle s_handleVisibility(L"InstanceMesh_Visibility");
render::Handle s_handleDraw(L"InstanceMesh_Draw");
render::Handle s_handleIndexCount(L"InstanceMesh_IndexCount");
render::Handle s_handleFirstIndex(L"InstanceMesh_FirstIndex");
render::Handle s_handleInstanceOffset(L"InstanceMesh_InstanceOffset");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMesh", InstanceMesh, IMesh)

InstanceMesh::InstanceMesh(
	render::IRenderSystem* renderSystem,
	const resource::Proxy< render::Shader >& shaderDraw
)
:	m_renderSystem(renderSystem)
,	m_shaderDraw(shaderDraw)
{
}

const Aabb3& InstanceMesh::getBoundingBox() const
{
	return m_renderMesh->getBoundingBox();
}

bool InstanceMesh::supportTechnique(render::handle_t technique) const
{
	return m_parts.find(technique) != m_parts.end();
}

void InstanceMesh::getTechniques(SmallSet< render::handle_t >& outHandles) const
{
	for (const auto part : m_parts)
		outHandles.insert(part.first);
}

void InstanceMesh::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	render::Buffer* instanceBuffer,
	render::Buffer* visibilityBuffer,
	uint32_t start,
	uint32_t count
)
{
	const auto it = m_parts.find(worldRenderPass.getTechnique());
	if (it == m_parts.end())
		return;

	render::RenderContext* renderContext = context.getRenderContext();

	const AlignedVector< Part >& parts = it->second;
	const auto& meshParts = m_renderMesh->getParts();

	// Lazy create the buffers.
	const uint32_t bufferItemCount = (uint32_t)alignUp(count, 16);
	if (count > m_allocatedCount)
	{
		m_drawBuffers.resize(0);
		m_allocatedCount = count;
	}

	const uint32_t peakCascade = worldRenderView.getCascade();
	const uint32_t dbSize = (uint32_t)m_drawBuffers.size();
	for (uint32_t i = dbSize; i < (peakCascade + 1) * parts.size(); ++i)
		m_drawBuffers.push_back(m_renderSystem->createBuffer(
			render::BufferUsage::BuStructured | render::BufferUsage::BuIndirect,
			bufferItemCount * sizeof(render::IndexedIndirectDraw),
			false
		));

	// Create draw buffers from visibility buffer.
	// Compute blocks are executed before render pass, so draws for shadow map rendering all cascades
	// are dispatched at the same time.
	for (uint32_t i = 0; i < parts.size(); ++i)
	{
		const auto& part = parts[i];

		auto permutation = worldRenderPass.getPermutation(m_shader);
		permutation.technique = part.shaderTechnique;
		auto sp = m_shader->getProgram(permutation);
		if (!sp)
			continue;

		render::Buffer* drawBuffer = m_drawBuffers[worldRenderView.getCascade() * parts.size() + i];

		const auto& primitives = meshParts[part.meshPart].primitives;

		auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(
			str(L"InstanceMesh draw commands %d %d", worldRenderView.getCascade(), i)
		);

		renderBlock->program = m_shaderDraw->getProgram().program;

		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setFloatParameter(s_handleIndexCount, primitives.getVertexCount() + 0.5f);
		renderBlock->programParams->setFloatParameter(s_handleFirstIndex, primitives.offset + 0.5f);
		renderBlock->programParams->setFloatParameter(s_handleInstanceOffset, start + 0.5f);
		renderBlock->programParams->setBufferViewParameter(s_handleVisibility, visibilityBuffer->getBufferView());
		renderBlock->programParams->setBufferViewParameter(s_handleDraw, drawBuffer->getBufferView());
		renderBlock->programParams->endParameters(renderContext);

		renderBlock->workSize[0] = (int32_t)count;

		renderContext->compute(renderBlock);
	}

	renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Indirect, nullptr, 0);

	// Add indirect draw for each mesh part.
	for (uint32_t i = 0; i < parts.size(); ++i)
	{
		const auto& part = parts[i];

		auto permutation = worldRenderPass.getPermutation(m_shader);
		permutation.technique = part.shaderTechnique;
		auto sp = m_shader->getProgram(permutation);
		if (!sp)
			continue;

		render::Buffer* drawBuffer = m_drawBuffers[worldRenderView.getCascade() * parts.size() + i];

		auto renderBlock = renderContext->allocNamed< render::IndirectRenderBlock >(
			str(L"InstanceMesh draw %d %d", worldRenderView.getCascade(), i)
		);
		renderBlock->distance = 10000.0f;
		renderBlock->program = sp.program;
		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->indexBuffer = m_renderMesh->getIndexBuffer()->getBufferView();
		renderBlock->indexType = m_renderMesh->getIndexType();
		renderBlock->vertexBuffer = m_renderMesh->getVertexBuffer()->getBufferView();
		renderBlock->vertexLayout = m_renderMesh->getVertexLayout();
		renderBlock->primitive = meshParts[part.meshPart].primitives.type;
		renderBlock->drawBuffer = drawBuffer->getBufferView();
		renderBlock->drawCount = (uint32_t)count;

		renderBlock->programParams->beginParameters(renderContext);

		//if (extraParameters)
		//	renderBlock->programParams->attachParameters(extraParameters);

		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			Transform::identity(),
			Transform::identity()
		);

		// #todo Same world buffer
		renderBlock->programParams->setFloatParameter(s_handleInstanceOffset, start + 0.5f);
		renderBlock->programParams->setBufferViewParameter(s_handleInstanceWorld, instanceBuffer->getBufferView());
		renderBlock->programParams->setBufferViewParameter(s_handleInstanceWorldLast, instanceBuffer->getBufferView());
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(sp.priority, renderBlock);
	}
}

}
