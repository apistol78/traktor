/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Instance/InstanceMesh.h"

#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Mesh/Instance/InstanceMeshData.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/IProgram.h"
#include "Render/IRenderSystem.h"
#include "Render/Mesh/Mesh.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"

#include <algorithm>

namespace traktor::mesh
{
namespace
{

render::Handle s_handleInstanceWorld(L"InstanceWorld");
render::Handle s_handleDraw(L"InstanceMesh_Draw");
render::Handle s_handleCompact(L"InstanceMesh_Compact");
render::Handle s_handleIndexCount(L"InstanceMesh_IndexCount");
render::Handle s_handleFirstIndex(L"InstanceMesh_FirstIndex");
render::Handle s_handleInstanceOffset(L"InstanceMesh_InstanceOffset");
render::Handle s_handleInstanceCount(L"InstanceMesh_InstanceCount");
render::Handle s_handlePartIndex(L"InstanceMesh_PartIndex");
render::Handle s_handlePartCount(L"InstanceMesh_PartCount");

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMesh", InstanceMesh, IMesh)

InstanceMesh::InstanceMesh(
	render::IRenderSystem* renderSystem,
	const resource::Proxy< render::Shader >& shaderDraw,
	const resource::Proxy< render::Shader >& shaderCompact)
	: m_renderSystem(renderSystem)
	, m_shaderDraw(shaderDraw)
	, m_shaderCompact(shaderCompact)
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

const render::Buffer* InstanceMesh::getRTVertexAttributes() const
{
	return m_renderMesh->getAuxBuffer(c_fccRayTracingVertexAttributes);
}

void InstanceMesh::cullableBuildSetup(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	render::Buffer* instanceBuffer,
	render::Buffer* visibilityBuffer,
	uint32_t start,
	uint32_t count)
{
	const auto it = m_parts.find(worldRenderPass.getTechnique());
	if (it == m_parts.end())
		return;

	render::RenderContext* renderContext = context.getRenderContext();

	const AlignedVector< Part >& parts = it->second;
	const auto& meshPrimitives = m_renderMesh->getPrimitives();
	const int32_t shadowMapIndex = worldRenderView.getShadowMapIndex();

	// Lazy create the buffers.
	const uint32_t bufferItemCount = (uint32_t)alignUp(count, 16);
	if (count > m_allocatedCount)
	{
		m_drawBuffers.resize(0);
		m_compactBuffers.resize(0);
		m_allocatedCount = count;
	}

	// One indirect command per part; the visible instances are compacted into a
	// dense list so each part is drawn by a single instanced command rather than
	// one command per instance. The buffers are shared by every technique, so size
	// them for whichever technique has the most parts.
	uint32_t maxPartCount = 0;
	for (const auto& technique : m_parts)
		maxPartCount = std::max(maxPartCount, (uint32_t)technique.second.size());

	const int32_t peakShadowMapIndex = shadowMapIndex;
	for (uint32_t i = (uint32_t)m_drawBuffers.size(); i < (uint32_t)peakShadowMapIndex + 1; ++i)
	{
		m_drawBuffers.push_back(m_renderSystem->createBuffer(
			render::BufferUsage::BuStructured | render::BufferUsage::BuIndirect,
			maxPartCount * sizeof(render::IndexedIndirectDraw),
			false,
			T_FILE_LINE_W));
		m_compactBuffers.push_back(m_renderSystem->createBuffer(
			render::BufferUsage::BuStructured,
			bufferItemCount * sizeof(float),
			false,
			T_FILE_LINE_W));
	}

	render::Buffer* drawBuffer = m_drawBuffers[shadowMapIndex];

	// Initialize the indirect command of each part with the static geometry range
	// and a zeroed instance count; the compact phase accumulates that count with
	// atomics after the culling component's setup to compact barrier.
	// Compute blocks are executed before render pass, so draws for shadow map rendering all cascades
	// are dispatched at the same time.
	for (uint32_t i = 0; i < parts.size(); ++i)
	{
		const auto& part = parts[i];

		auto permutation = worldRenderPass.getPermutation(m_shader);
		permutation.technique = part.shaderTechnique;
		const auto sp = m_shader->getProgram(permutation);
		if (!sp)
			continue;

		const auto& primitives = meshPrimitives[part.meshPart];

		auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(
			str(L"InstanceMesh draw command %d %d", shadowMapIndex, i));

		renderBlock->program = m_shaderDraw->getProgram().program;

		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setFloatParameter(s_handleIndexCount, primitives.getVertexCount() + 0.5f);
		renderBlock->programParams->setFloatParameter(s_handleFirstIndex, primitives.offset + 0.5f);
		renderBlock->programParams->setFloatParameter(s_handlePartIndex, i + 0.5f);
		renderBlock->programParams->setBufferViewParameter(s_handleDraw, drawBuffer->getBufferView());
		renderBlock->programParams->endParameters(renderContext);

		renderBlock->workSize[0] = 1;

		renderContext->compute(renderBlock);
	}
}

void InstanceMesh::cullableBuildCompact(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	render::Buffer* instanceBuffer,
	render::Buffer* visibilityBuffer,
	uint32_t start,
	uint32_t count)
{
	const auto it = m_parts.find(worldRenderPass.getTechnique());
	if (it == m_parts.end())
		return;

	render::RenderContext* renderContext = context.getRenderContext();

	const AlignedVector< Part >& parts = it->second;
	const int32_t shadowMapIndex = worldRenderView.getShadowMapIndex();

	render::Buffer* drawBuffer = m_drawBuffers[shadowMapIndex];
	render::Buffer* compactBuffer = m_compactBuffers[shadowMapIndex];

	// Compact the visible instances of this batch into a dense list, and accumulate
	// the instance count shared by every part's command.
	auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(
		str(L"InstanceMesh compact %d", shadowMapIndex));

	renderBlock->program = m_shaderCompact->getProgram().program;

	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->programParams->beginParameters(renderContext);
	renderBlock->programParams->setFloatParameter(s_handleInstanceOffset, start + 0.5f);
	renderBlock->programParams->setFloatParameter(s_handleInstanceCount, count + 0.5f);
	renderBlock->programParams->setFloatParameter(s_handlePartCount, parts.size() + 0.5f);
	renderBlock->programParams->setBufferViewParameter(world::ShaderParameter::Visibility, visibilityBuffer->getBufferView());
	renderBlock->programParams->setBufferViewParameter(s_handleDraw, drawBuffer->getBufferView());
	renderBlock->programParams->setBufferViewParameter(s_handleCompact, compactBuffer->getBufferView());
	renderBlock->programParams->endParameters(renderContext);

	renderBlock->workSize[0] = (int32_t)count;

	renderContext->compute(renderBlock);
}

void InstanceMesh::cullableBuildDraw(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	render::Buffer* instanceBuffer,
	render::Buffer* visibilityBuffer,
	uint32_t start,
	uint32_t count)
{
	const auto it = m_parts.find(worldRenderPass.getTechnique());
	if (it == m_parts.end())
		return;

	render::RenderContext* renderContext = context.getRenderContext();

	const AlignedVector< Part >& parts = it->second;
	const auto& meshPrimitives = m_renderMesh->getPrimitives();
	const int32_t shadowMapIndex = worldRenderView.getShadowMapIndex();

	render::Buffer* drawBuffer = m_drawBuffers[shadowMapIndex];
	render::Buffer* compactBuffer = m_compactBuffers[shadowMapIndex];

	// Add a single indirect draw for each mesh part.
	for (uint32_t i = 0; i < parts.size(); ++i)
	{
		const auto& part = parts[i];
		const bool depthStream = part.depthStream && m_renderMesh->getDepthVertexBuffer() != nullptr;

		auto permutation = worldRenderPass.getPermutation(m_shader);
		permutation.technique = part.shaderTechnique;
		const auto sp = m_shader->getProgram(permutation);
		if (!sp)
			continue;

		auto renderBlock = renderContext->allocNamed< render::IndirectRenderBlock >(
			str(L"InstanceMesh draw %d %d", shadowMapIndex, i));
		renderBlock->distance = 10000.0f;
		renderBlock->program = sp.program;
		renderBlock->indexBuffer = m_renderMesh->getIndexBuffer()->getBufferView();
		renderBlock->indexType = m_renderMesh->getIndexType();
		renderBlock->vertexBuffer = depthStream ? m_renderMesh->getDepthVertexBuffer()->getBufferView() : m_renderMesh->getVertexBuffer()->getBufferView();
		renderBlock->vertexLayout = depthStream ? m_renderMesh->getDepthVertexLayout() : m_renderMesh->getVertexLayout();
		renderBlock->primitive = meshPrimitives[part.meshPart].type;
		renderBlock->drawBuffer = drawBuffer->getBufferView();
		renderBlock->drawOffset = i * (uint32_t)sizeof(render::IndexedIndirectDraw);
		renderBlock->drawCount = 1;

		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->programParams->beginParameters(renderContext);

		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			Transform::identity(),
			Transform::identity());

		renderBlock->programParams->setFloatParameter(s_handleInstanceOffset, start + 0.5f);
		renderBlock->programParams->setBufferViewParameter(s_handleInstanceWorld, instanceBuffer->getBufferView());
		renderBlock->programParams->setBufferViewParameter(s_handleCompact, compactBuffer->getBufferView());
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(sp.priority, renderBlock);
	}
}

}
