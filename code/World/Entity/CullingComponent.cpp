/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Entity/CullingComponent.h"

namespace traktor::world
{
	namespace
	{

const resource::Id< render::Shader > c_shaderInstanceMeshCull(L"{37998131-BDA1-DE45-B175-35B088FEE61C}");

render::Handle s_handleInstanceWorld(L"InstanceWorld");
render::Handle s_handleTargetSize(L"InstanceMesh_TargetSize");
render::Handle s_handleViewProjection(L"InstanceMesh_ViewProjection");
render::Handle s_handleVisibility(L"InstanceMesh_Visibility");
render::Handle s_handleCullFrustum(L"InstanceMesh_CullFrustum");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.CullingComponent", CullingComponent, IWorldComponent)

CullingComponent::CullingComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
{
	resourceManager->bind(c_shaderInstanceMeshCull, m_shaderCull);
}

void CullingComponent::destroy()
{
	T_FATAL_ASSERT_M(m_instances.empty(), L"Culling instances not empty.");
	safeDestroy(m_instanceBuffer);
	for (auto visibilityBuffer : m_visibilityBuffers)
	{
		if (visibilityBuffer)
			visibilityBuffer->destroy();
	}
	m_visibilityBuffers.resize(0);
	m_renderSystem = nullptr;
}

void CullingComponent::update(World* world, const UpdateParams& update)
{
}

void CullingComponent::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass
)
{
	if (m_instances.empty())
		return;

	render::RenderContext* renderContext = context.getRenderContext();
	const uint32_t bufferItemCount = (uint32_t)alignUp(m_instances.size(), 16);

	// Lazy create the buffers if necessary.
	if (!m_instanceBuffer || bufferItemCount > m_instanceAllocatedCount)
	{
		m_instanceBuffer = m_renderSystem->createBuffer(render::BufferUsage::BuStructured, bufferItemCount * sizeof(InstanceRenderData), true);
		m_visibilityBuffers.resize(0);
		m_instanceAllocatedCount = bufferItemCount;
		m_instanceBufferDirty = true;
	}

	// Ensure we have visibility buffers for all cascades.
	const uint32_t peakCascade = worldRenderView.getCascade();
	const uint32_t vbSize = (uint32_t)m_visibilityBuffers.size();
	for (uint32_t i = vbSize; i < peakCascade + 1; ++i)
		m_visibilityBuffers.push_back(m_renderSystem->createBuffer(render::BufferUsage::BuStructured, bufferItemCount * sizeof(float), false));

	// Update buffer is any instance has moved.
	if (m_instanceBufferDirty)
	{
		auto ptr = (InstanceRenderData*)m_instanceBuffer->lock();
		for (const auto& instance : m_instances)
		{
			InstanceRenderData& ird = *ptr++;
			instance->transform.rotation().e.storeAligned(ird.rotation);
			instance->transform.translation().storeAligned(ird.translation);
			instance->boundingBox.mn.storeAligned(ird.boundingBoxMin);
			instance->boundingBox.mx.storeAligned(ird.boundingBoxMax);
		}
		m_instanceBuffer->unlock();
		m_instanceBufferDirty = false;
	}

	render::Buffer* visibilityBuffer = m_visibilityBuffers[worldRenderView.getCascade()];

	// Cull instances, output are visibility buffer.
	// Compute blocks are executed before render pass, so for shadow map rendering all cascades
	// are culled before being rendered.
	{
		Vector4 cullFrustum[12];

		const Frustum& cf = worldRenderView.getCullFrustum();
		T_FATAL_ASSERT(cf.planes.size() <= sizeof_array(cullFrustum));
		for (int32_t i = 0; i < (int32_t)cf.planes.size(); ++i)
			cullFrustum[i] = cf.planes[i].normal().xyz0() + Vector4(0.0f, 0.0f, 0.0f, cf.planes[i].distance());
		for (int32_t i = (int32_t)cf.planes.size(); i < sizeof_array(cullFrustum); ++i)
			cullFrustum[i] = Vector4::zero();

		const Vector2 viewSize = worldRenderView.getViewSize();

		auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(
			str(L"Cull %d", worldRenderView.getCascade())
		);

		render::Shader::Permutation perm;
		if (worldRenderPass.getTechnique() == s_techniqueDeferredGBufferWrite)
		{
			// Deferred g-buffer pass has access to HiZ texture.
			m_shaderCull->setCombination(s_handleCullingHiZ, true, perm);
		}
		else
		{
			// All other paths use simple frustum culling only.
			m_shaderCull->setCombination(s_handleCullingHiZ, false, perm);
		}

		renderBlock->program = m_shaderCull->getProgram(perm).program;

		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->programParams->beginParameters(renderContext);

		worldRenderPass.setProgramParameters(renderBlock->programParams);

		renderBlock->programParams->setVectorParameter(s_handleTargetSize, Vector4(viewSize.x, viewSize.y, 0.0f, 0.0f));
		renderBlock->programParams->setMatrixParameter(s_handleViewProjection, worldRenderView.getProjection() * worldRenderView.getView());
		renderBlock->programParams->setVectorArrayParameter(s_handleCullFrustum, cullFrustum, sizeof_array(cullFrustum));
		renderBlock->programParams->setBufferViewParameter(s_handleInstanceWorld, m_instanceBuffer->getBufferView());
		renderBlock->programParams->setBufferViewParameter(s_handleVisibility, visibilityBuffer->getBufferView());
		renderBlock->programParams->endParameters(renderContext);

		renderBlock->workSize[0] = (int32_t)m_instances.size();

		renderContext->compute(renderBlock);
		renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Compute, nullptr, 0);
	}

	// Batch draw instances; assumes m_instances are sorted by "ordinal" so we can scan for run length.
	for (uint32_t i = 0; i < (uint32_t)m_instances.size(); )
	{
		uint32_t j = i + 1;
		for (; j < (uint32_t)m_instances.size(); ++j)
		{
			if (m_instances[i]->ordinal != m_instances[j]->ordinal)
				break;
		}

		m_instances[i]->cullable->cullableBuild(
			context,
			worldRenderView,
			worldRenderPass,
			m_instanceBuffer,
			visibilityBuffer,
			i,
			(j - i)
		);

		i = j;
	}
}

CullingComponent::Instance* CullingComponent::allocateInstance(ICullable* cullable, intptr_t ordinal)
{
	Instance* instance = new Instance();
	instance->owner = this;
	instance->cullable = cullable;
	instance->ordinal = ordinal;
	instance->transform = Transform::identity();
	instance->boundingBox = cullable->cullableGetBoundingBox();
	
	// Insert instance sorted by ordinal so we can calculate run length when building.
	auto it = std::upper_bound(m_instances.begin(), m_instances.end(), instance, [=](Instance* lh, Instance* rh) {
		return lh->ordinal < rh->ordinal;
	});
	m_instances.insert(it, instance);
	return instance;
}

void CullingComponent::releaseInstance(Instance*& instance)
{
	T_FATAL_ASSERT(instance->owner == this);
	auto it = std::find(m_instances.begin(), m_instances.end(), instance);
	m_instances.erase(it);
	delete instance;
	instance = nullptr;
}

void CullingComponent::Instance::setTransform(const Transform& transform)
{
	this->transform = transform;
	this->boundingBox = this->cullable->cullableGetBoundingBox().transform(transform);
	this->owner->m_instanceBufferDirty = true;
}

}
