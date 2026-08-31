/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/CullingComponent.h"

#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/IRenderSystem.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"

namespace traktor::world
{
namespace
{

const resource::Id< render::Shader > c_shaderInstanceMeshCull(L"{37998131-BDA1-DE45-B175-35B088FEE61C}");	// World/Culling/Visibility

static const render::Handle s_handleInstanceWorld(L"InstanceWorld");
static const render::Handle s_techniqueVelocityWrite(L"World_VelocityWrite");

/*! Local work group size of the culling shader.
 *
 * The dispatch is rounded up to whole work groups and the shader writes its
 * visibility slot unconditionally, so the padding threads of the last group write
 * past the instance count. The buffers are sized to cover them; keep this in sync
 * with the local size declared in World/Culling/Visibility.
 */
constexpr uint32_t c_cullWorkGroupSize = 16;

/*! Order of instances in the instance buffer.
 *
 * Primarily sorted by ordinal so all instances of a single mesh form one run which
 * can be drawn with a single batch. Within such a run static instances are placed
 * before dynamic ones so a static only pass just need to shorten the run.
 */
bool instanceSortPredicate(const CullingComponent::Instance* lh, const CullingComponent::Instance* rh)
{
	if (lh->ordinal != rh->ordinal)
		return lh->ordinal < rh->ordinal;
	else
		return (int32_t)lh->dynamic < (int32_t)rh->dynamic;
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.CullingComponent", CullingComponent, IWorldComponent)

CullingComponent::CullingComponent(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
	: m_renderSystem(renderSystem)
{
	resourceManager->bind(c_shaderInstanceMeshCull, m_shaderCull);
}

void CullingComponent::destroy()
{
	T_FATAL_ASSERT_M(m_instances.empty(), L"Culling instances not empty.");
	safeDestroy(m_instanceBuffer);
	for (auto visibilityBuffer : m_visibilityBuffers)
		if (visibilityBuffer)
			visibilityBuffer->destroy();
	m_visibilityBuffers.resize(0);
	m_shaderCull.clear();
	m_renderSystem = nullptr;
}

void CullingComponent::update(World* world, const UpdateParams& update)
{
}

void CullingComponent::build(
	const WorldBuildContext& context,
	const WorldRenderView& worldRenderView,
	const IWorldRenderPass& worldRenderPass)
{
	if (m_instances.empty())
		return;

	render::RenderContext* renderContext = context.getRenderContext();
	const uint32_t bufferItemCount = (uint32_t)alignUp(m_instances.size(), c_cullWorkGroupSize);

	// Lazy create the buffers if necessary.
	if (!m_instanceBuffer || bufferItemCount > m_instanceAllocatedCount)
	{
		m_instanceBuffer = m_renderSystem->createBuffer(render::BufferUsage::BuStructured, bufferItemCount * sizeof(InstanceRenderData), true, T_FILE_LINE_W);
		m_visibilityBuffers.resize(0);
		m_instanceAllocatedCount = bufferItemCount;
		m_instanceBufferDirty = true;
	}

	// Ensure we have visibility buffers for all cascades.
	const int32_t peakShadowMapIndex = worldRenderView.getShadowMapIndex();
	const uint32_t vbSize = (uint32_t)m_visibilityBuffers.size();
	for (uint32_t i = vbSize; i < peakShadowMapIndex + 1; ++i)
		m_visibilityBuffers.push_back(m_renderSystem->createBuffer(render::BufferUsage::BuStructured, bufferItemCount * sizeof(float), false, T_FILE_LINE_W));

	// Update buffer is any instance has moved.
	if (m_instanceBufferDirty)
	{
		auto ptr = (InstanceRenderData*)m_instanceBuffer->lock();
		for (auto& instance : m_instances)
		{
			InstanceRenderData& ird = *ptr++;
			instance->transform.rotation().e.storeAligned(ird.rotation);
			instance->transform.translation().storeAligned(ird.translation);
			instance->lastTransform.rotation().e.storeAligned(ird.lastRotation);
			instance->lastTransform.translation().storeAligned(ird.lastTranslation);
			instance->boundingBox.mn.storeAligned(ird.boundingBoxMin);
			instance->boundingBox.mx.storeAligned(ird.boundingBoxMax);
			instance->lastTransform = instance->transform;
		}
		m_instanceBuffer->unlock();
		m_instanceBufferDirty = false;
	}

	// In case no instance has been moved we don't render at all for velocity technique.
	if (worldRenderPass.getTechnique() == s_techniqueVelocityWrite)
	{
		if (!m_velocityDirty)
			return;
		m_velocityDirty = false;
	}

	// A static only pass with no static instances emits no runs at all; skip the
	// cull dispatch and the barriers entirely since the visibility buffer is only
	// consumed by blocks emitted from the same pass.
	const bool staticOnly = worldRenderView.getStaticOnly();
	if (staticOnly)
	{
		bool anyStatic = false;
		for (const auto instance : m_instances)
		{
			if (!instance->dynamic)
			{
				anyStatic = true;
				break;
			}
		}
		if (!anyStatic)
			return;
	}

	render::Buffer* visibilityBuffer = m_visibilityBuffers[worldRenderView.getShadowMapIndex()];

	// Every pass which culls (g-buffer, shadow, velocity, ...) reuses the same set of
	// visibility, draw command and compaction buffers - they are only keyed by cascade,
	// and the primary view shares cascade 0 with the first shadow slice. The render graph
	// does not put any barrier between passes, so without this the culling of one pass
	// would be free to overwrite buffers that the previous pass' indirect draws and vertex
	// shaders are still reading, which shows up as whole batches blinking out for a frame.
	// A write-after-read hazard only needs an execution dependency, no cache flush.
	renderContext->compute< render::BarrierRenderBlock >(
		render::Stage::Compute | render::Stage::Vertex | render::Stage::Indirect,
		render::Stage::Compute,
		nullptr,
		0);

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
			str(L"Cull %d", worldRenderView.getShadowMapIndex()));

		render::Shader::Permutation perm;
		if (
			worldRenderPass.getTechnique() == ShaderTechnique::ZPrePassWrite ||
			worldRenderPass.getTechnique() == ShaderTechnique::DeferredGBufferWrite)
		{
			// Z pre-pass and deferred g-buffer pass have access to HiZ texture; both
			// must cull identically so the depth written by the pre-pass matches the
			// depth the g-buffer pass expects.
			m_shaderCull->setCombination(ShaderPermutation::CullingHiZ, true, perm);
		}
		else
		{
			// All other paths use simple frustum culling only.
			m_shaderCull->setCombination(ShaderPermutation::CullingHiZ, false, perm);
		}

		renderBlock->program = m_shaderCull->getProgram(perm).program;

		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->programParams->beginParameters(renderContext);

		worldRenderPass.setProgramParameters(renderBlock->programParams);

		renderBlock->programParams->setVectorParameter(ShaderParameter::TargetSize, Vector4(viewSize.x, viewSize.y, 0.0f, 0.0f));
		renderBlock->programParams->setMatrixParameter(ShaderParameter::Projection, worldRenderView.getProjection() * worldRenderView.getView());
		renderBlock->programParams->setVectorArrayParameter(ShaderParameter::CullFrustum, cullFrustum, sizeof_array(cullFrustum));
		renderBlock->programParams->setBufferViewParameter(s_handleInstanceWorld, m_instanceBuffer->getBufferView());
		renderBlock->programParams->setBufferViewParameter(ShaderParameter::Visibility, visibilityBuffer->getBufferView());
		renderBlock->programParams->endParameters(renderContext);

		renderBlock->workSize[0] = (int32_t)m_instances.size();

		renderContext->compute(renderBlock);
	}

	// Invoke a build phase on each batch run of instances; assumes m_instances are
	// sorted by "ordinal" so we can scan for run length.
	const auto forEachRun = [&](auto&& phase)
	{
		for (uint32_t i = 0; i < (uint32_t)m_instances.size();)
		{
			uint32_t j = i + 1;
			for (; j < (uint32_t)m_instances.size(); ++j)
				if (m_instances[i]->ordinal != m_instances[j]->ordinal)
					break;

			// Instances are sorted static before dynamic within each run so a static only
			// pass need only draw the leading part of the run.
			uint32_t k = j;
			if (staticOnly)
			{
				for (k = i; k < j; ++k)
					if (m_instances[k]->dynamic)
						break;
			}

			if (k > i)
				phase(m_instances[i]->cullable, i, k - i);

			i = j;
		}
	};

	// Emit each phase for all runs before a single barrier and the next phase, so
	// the number of barriers stays constant instead of two per run.
	forEachRun([&](ICullable* cullable, uint32_t start, uint32_t count) {
		cullable->cullableBuildSetup(context, worldRenderView, worldRenderPass, m_instanceBuffer, visibilityBuffer, start, count);
	});

	// The visibility buffer written by the cull dispatch and the draw commands
	// initialized by the setup phase are both consumed by the compact phase's
	// compute blocks, not by an indirect read, so this has to be a shader read barrier.
	renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Compute, nullptr, 0);

	forEachRun([&](ICullable* cullable, uint32_t start, uint32_t count) {
		cullable->cullableBuildCompact(context, worldRenderView, worldRenderPass, m_instanceBuffer, visibilityBuffer, start, count);
	});

	// The compacted draw commands are consumed by the indirect draws emitted in the
	// draw phase.
	renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Indirect, nullptr, 0);

	forEachRun([&](ICullable* cullable, uint32_t start, uint32_t count) {
		cullable->cullableBuildDraw(context, worldRenderView, worldRenderPass, m_instanceBuffer, visibilityBuffer, start, count);
	});
}

CullingComponent::Instance* CullingComponent::createInstance(ICullable* cullable, intptr_t ordinal, bool dynamic)
{
	Instance* instance = new Instance();
	instance->owner = this;
	instance->cullable = cullable;
	instance->ordinal = ordinal;
	instance->dynamic = dynamic;
	instance->transform = Transform::identity();
	instance->lastTransform = Transform::identity();
	instance->boundingBox = cullable->cullableGetBoundingBox();

	insertInstance(instance);
	return instance;
}

void CullingComponent::destroyInstance(Instance* instance)
{
	T_FATAL_ASSERT(instance->owner == this);
	auto it = std::find(m_instances.begin(), m_instances.end(), instance);
	if (it != m_instances.end())
	{
		m_instances.erase(it);
		m_instanceBufferDirty = true;
		delete instance;
	}
}

void CullingComponent::insertInstance(Instance* instance)
{
	// Insert instance sorted so we can calculate run length when building.
	auto it = std::upper_bound(m_instances.begin(), m_instances.end(), instance, instanceSortPredicate);
	m_instances.insert(it, instance);

	// All instances following the inserted one have shifted position in the buffer.
	m_instanceBufferDirty = true;
}

void CullingComponent::Instance::destroy()
{
	T_FATAL_ASSERT(this->owner);
	this->owner->destroyInstance(this);
}

void CullingComponent::Instance::setTransform(const Transform& transform)
{
	this->lastTransform = this->transform;
	this->transform = transform;
	this->boundingBox = this->cullable->cullableGetBoundingBox().transform(transform);
	this->owner->m_instanceBufferDirty = true;
	this->owner->m_velocityDirty = true;
}

void CullingComponent::Instance::setDynamic(bool dynamic)
{
	if (dynamic == this->dynamic)
		return;

	this->dynamic = dynamic;

	// Sort order depend on dynamic state; re-insert instance at it's new position.
	AlignedVector< Instance* >& instances = this->owner->m_instances;
	const auto it = std::find(instances.begin(), instances.end(), this);
	T_FATAL_ASSERT(it != instances.end());
	instances.erase(it);
	this->owner->insertInstance(this);
}

}
