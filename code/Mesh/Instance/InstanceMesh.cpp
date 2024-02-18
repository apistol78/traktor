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
#include "Render/Buffer.h"
#include "Render/IProgram.h"
#include "Render/IRenderSystem.h"
#include "Render/Context/RenderContext.h"
#include "Render/Mesh/Mesh.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{
	namespace
	{

render::Handle s_handleInstanceWorld(L"InstanceWorld");
render::Handle s_handleInstanceWorldLast(L"InstanceWorldLast");

render::Handle s_handleBoundingBoxMin(L"InstanceMesh_BoundingBoxMin");
render::Handle s_handleBoundingBoxMax(L"InstanceMesh_BoundingBoxMax");
render::Handle s_handleView(L"InstanceMesh_View");
render::Handle s_handleViewInverse(L"InstanceMesh_ViewInverse");
render::Handle s_handleVisibility(L"InstanceMesh_Visibility");
render::Handle s_handleCullFrustum(L"InstanceMesh_CullFrustum");

render::Handle s_handleDraw(L"InstanceMesh_Draw");
render::Handle s_handleIndexCount(L"InstanceMesh_IndexCount");
render::Handle s_handleFirstIndex(L"InstanceMesh_FirstIndex");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMesh", InstanceMesh, IMesh)

InstanceMesh::InstanceMesh(
	render::IRenderSystem* renderSystem,
	const resource::Proxy< render::Shader >& shaderCull,
	const resource::Proxy< render::Shader >& shaderDraw
)
:	m_renderSystem(renderSystem)
,	m_shaderCull(shaderCull)
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
	render::RenderContext* renderContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	render::ProgramParameters* extraParameters
) const
{
	bool haveAlphaBlend = false;

	if (m_instances.empty())
		return;

	const auto it = m_parts.find(worldRenderPass.getTechnique());
	if (it == m_parts.end())
		return;

	const AlignedVector< Part >& parts = it->second;
	T_FATAL_ASSERT(parts.size() <= 4);

	const auto& meshParts = m_renderMesh->getParts();

	// Lazy create the buffers.
	if (!m_instanceBuffer)
	{
		m_instanceBuffer = m_renderSystem->createBuffer(render::BufferUsage::BuStructured, m_instances.size() * sizeof(InstanceMeshData), true);

		m_visibilityBuffers.resize(4);
		for (uint32_t i = 0; i < 4; ++i)
			m_visibilityBuffers[i] = m_renderSystem->createBuffer(render::BufferUsage::BuStructured, m_instances.size() * sizeof(float), false);

		m_drawBuffers.resize(4 * 4);
		for (uint32_t i = 0; i < 4 * 4; ++i)
			m_drawBuffers[i] = m_renderSystem->createBuffer(render::BufferUsage::BuStructured | render::BufferUsage::BuIndirect, m_instances.size() * sizeof(render::IndexedIndirectDraw), false);

		m_instanceBufferDirty = true;
	}

	// Update buffer is any instance has moved.
	if (m_instanceBufferDirty)
	{
		auto ptr = (InstanceMeshData*)m_instanceBuffer->lock();
		for (const auto& instance : m_instances)
			*ptr++ = packInstanceMeshData(instance->transform);
		m_instanceBuffer->unlock();
		m_instanceBufferDirty = false;
	}

	render::Buffer* visibilityBuffer = m_visibilityBuffers[worldRenderView.getCascade()];

	// Cull instances.
	// #todo Compute blocks are executed before render pass, so for shadow map rendering all cascades
	// are culled before being rendered.
	{
		Vector4 cullFrustum[12];

		const Frustum& cf = worldRenderView.getCullFrustum();
		T_FATAL_ASSERT(cf.planes.size() <= sizeof_array(cullFrustum));
		for (int32_t i = 0; i < cf.planes.size(); ++i)
			cullFrustum[i] = cf.planes[i].normal().xyz0() + Vector4(0.0f, 0.0f, 0.0f, cf.planes[i].distance());
		for (int32_t i = cf.planes.size(); i < sizeof_array(cullFrustum); ++i)
			cullFrustum[i] = Vector4::zero();

		auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(
			str(L"InstanceMesh cull %d", worldRenderView.getCascade())
		);

		renderBlock->program = m_shaderCull->getProgram().program;

		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setVectorParameter(s_handleBoundingBoxMin, m_renderMesh->getBoundingBox().mn);
		renderBlock->programParams->setVectorParameter(s_handleBoundingBoxMax, m_renderMesh->getBoundingBox().mx);
		renderBlock->programParams->setVectorArrayParameter(s_handleCullFrustum, cullFrustum, sizeof_array(cullFrustum));
		renderBlock->programParams->setMatrixParameter(s_handleView, worldRenderView.getView());
		renderBlock->programParams->setMatrixParameter(s_handleViewInverse, worldRenderView.getView().inverse());
		renderBlock->programParams->setBufferViewParameter(s_handleInstanceWorld, m_instanceBuffer->getBufferView());
		renderBlock->programParams->setBufferViewParameter(s_handleVisibility, visibilityBuffer->getBufferView());
		renderBlock->programParams->endParameters(renderContext);

		renderBlock->workSize[0] = m_instances.size();

		renderContext->compute(renderBlock);
		renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Compute);
	}

	// Create draw buffers from visibility buffer.
	for (uint32_t i = 0; i < parts.size(); ++i)
	{
		const auto& part = parts[i];

		auto permutation = worldRenderPass.getPermutation(m_shader);
		permutation.technique = part.shaderTechnique;
		auto sp = m_shader->getProgram(permutation);
		if (!sp)
			continue;

		render::Buffer* drawBuffer = m_drawBuffers[worldRenderView.getCascade() * 4 + i];

		const auto& primitives = meshParts[part.meshPart].primitives;

		auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(
			str(L"InstanceMesh draw commands %d %d", worldRenderView.getCascade(), i)
		);

		renderBlock->program = m_shaderDraw->getProgram().program;

		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setFloatParameter(s_handleIndexCount, primitives.getVertexCount() + 0.5f);
		renderBlock->programParams->setFloatParameter(s_handleFirstIndex, primitives.offset + 0.5f);
		renderBlock->programParams->setBufferViewParameter(s_handleVisibility, visibilityBuffer->getBufferView());
		renderBlock->programParams->setBufferViewParameter(s_handleDraw, drawBuffer->getBufferView());
		renderBlock->programParams->endParameters(renderContext);

		renderBlock->workSize[0] = m_instances.size();

		renderContext->compute(renderBlock);
	}

	renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Indirect);

	// Add indirect draw for each mesh part.
	for (uint32_t i = 0; i < parts.size(); ++i)
	{
		const auto& part = parts[i];

		auto permutation = worldRenderPass.getPermutation(m_shader);
		permutation.technique = part.shaderTechnique;
		auto sp = m_shader->getProgram(permutation);
		if (!sp)
			continue;

		render::Buffer* drawBuffer = m_drawBuffers[worldRenderView.getCascade() * 4 + i];

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
		renderBlock->drawCount = m_instances.size();

		renderBlock->programParams->beginParameters(renderContext);

		if (extraParameters)
			renderBlock->programParams->attachParameters(extraParameters);

		worldRenderPass.setProgramParameters(
			renderBlock->programParams,
			Transform::identity(),
			Transform::identity()
		);

		// #todo Same world buffer
		renderBlock->programParams->setBufferViewParameter(s_handleInstanceWorld, m_instanceBuffer->getBufferView());
		renderBlock->programParams->setBufferViewParameter(s_handleInstanceWorldLast, m_instanceBuffer->getBufferView());
		renderBlock->programParams->endParameters(renderContext);

		renderContext->draw(sp.priority, renderBlock);
	}
}

InstanceMesh::Instance* InstanceMesh::allocateInstance()
{
	Instance* instance = new Instance();
	instance->mesh = this;
	instance->transform = Transform::identity();

	m_instances.push_back(instance);

	safeDestroy(m_instanceBuffer);

	return instance;
}

void InstanceMesh::releaseInstance(Instance* instance)
{
	T_FATAL_ASSERT(instance->mesh == this);

	auto it = std::find(m_instances.begin(), m_instances.end(), instance);
	m_instances.erase(it);
	delete instance;

	safeDestroy(m_instanceBuffer);
}

void InstanceMesh::Instance::setTransform(const Transform& transform)
{
	this->mesh->m_instanceBufferDirty = true;
	this->transform = transform;
}

}
