/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/EntityIdPass.h"

#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "World/Entity.h"
#include "World/Entity/CullingComponent.h"
#include "World/EntityIdQuery.h"
#include "World/IEntityComponent.h"
#include "World/IEntityRenderer.h"
#include "World/IWorldRenderer.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/World.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"

#include <algorithm>

namespace traktor::world
{
namespace
{

const resource::Id< render::Shader > c_readBackShader(L"{23A40284-2D7E-4DB3-B169-D0DADFCF842D}"); // System/World/EntityId/Shaders/EntityIdReadBack

constexpr float c_readBackPending = -1.0f;

/*! World render pass stamping each draw with the id of the entity being built. */
class EntityIdRenderPass : public WorldRenderPassShared
{
public:
	explicit EntityIdRenderPass(render::ProgramParameters* sharedParams, const WorldRenderView& worldRenderView)
		: WorldRenderPassShared(ShaderTechnique::EntityIdWrite, sharedParams, worldRenderView)
	{
	}

	void setId(float id) { m_id = id; }

	virtual void setProgramParameters(render::ProgramParameters* programParams) const override final
	{
		WorldRenderPassShared::setProgramParameters(programParams);
		programParams->setFloatParameter(ShaderParameter::EntityId, m_id);
	}

	virtual void setProgramParameters(render::ProgramParameters* programParams, const Transform& lastWorld, const Transform& world) const override final
	{
		WorldRenderPassShared::setProgramParameters(programParams, lastWorld, world);
		programParams->setFloatParameter(ShaderParameter::EntityId, m_id);
	}

private:
	float m_id = 0.0f;
};

struct Renderable
{
	IEntityRenderer* entityRenderer;
	Object* object;
	float id;
};

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.EntityIdPass", EntityIdPass, Object)

bool EntityIdPass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	if (!desc.entityIdQuery)
		return false;

	if (!resourceManager->bind(c_readBackShader, m_readBackShader))
	{
		log::warning << L"Unable to create entity id pass; missing read back shader." << Endl;
		return false;
	}

	Ref< render::Buffer > readBackBuffer = renderSystem->createBuffer(
		render::BufferUsage::BuStructured | render::BufferUsage::BuReadBack,
		sizeof(float),
		false,
		T_FILE_LINE_W);
	if (!readBackBuffer)
		return false;

	// Freshly allocated memory is undefined; reset all slots so none is mistaken for an id.
	float* slots = (float*)readBackBuffer->lock();
	if (!slots)
	{
		log::warning << L"Unable to create entity id pass; read back buffer is not readable." << Endl;
		return false;
	}
	slots[0] = c_readBackPending;
	readBackBuffer->unlock();

	m_entityRenderers = desc.entityRenderers;
	m_query = desc.entityIdQuery;
	m_query->m_readBackBuffer = readBackBuffer;
	m_query->m_supported = true;
	return true;
}

void EntityIdPass::destroy()
{
	if (m_query)
	{
		m_query->m_supported = false;
		m_query->m_state = EntityIdQuery::State::Idle;
		m_query->m_entities.clear();
		m_query->m_instanceEntities.clear();
		safeDestroy(m_query->m_readBackBuffer);
		m_query = nullptr;
	}
	m_readBackShader.clear();
	m_entityRenderers = nullptr;
}

bool EntityIdPass::isRequested() const
{
	return m_query->m_state == EntityIdQuery::State::Requested;
}

void EntityIdPass::setup(
	const World* world,
	const WorldRenderView& worldRenderView,
	const AlignedVector< render::RGDependency >& setupAttachments,
	render::RenderGraph& renderGraph,
	render::RGTargetSet depthTargetSetId,
	render::RGTargetSet visualTargetSetId,
	const std::function< bool(const EntityState& state) >& filter) const
{
	if (m_query->m_state != EntityIdQuery::State::Requested)
		return;

	T_PROFILER_SCOPE(L"EntityIdPass::setup");

	// Reset slot this request is read back into; the GPU overwrites it with the id.
	float* slots = (float*)m_query->m_readBackBuffer->lock();
	if (!slots)
		return;
	slots[0] = c_readBackPending;
	m_query->m_readBackBuffer->unlock();

	// Assign ids to entities gathered by the world renderer, all components of an entity are drawn with its id.
	AlignedVector< Renderable > renderables;
	AlignedVector< std::pair< const Object*, Entity* > > owners;

	m_query->m_entities.resize(0);
	for (auto entity : world->getEntities())
	{
		const EntityState state = entity->getState();
		if (filter != nullptr && filter(state) == false)
			continue;
		else if (filter == nullptr && state.visible == false)
			continue;

		const float id = (float)(m_query->m_entities.size() + 1);
		bool haveRenderable = false;

		for (auto component : entity->getComponents())
		{
			owners.push_back({ component, entity });

			IEntityRenderer* entityRenderer = m_entityRenderers->find(type_of(component));
			if (entityRenderer)
			{
				renderables.push_back({ entityRenderer, component, id });
				haveRenderable = true;
			}
		}

		if (haveRenderable)
			m_query->m_entities.push_back(entity);
	}

	// Instances are drawn together by the culling component; ids following the entities'
	// are offset by each instance's index into the instance buffer, which is added in the
	// shader, thus snapshot the entity owning each instance.
	m_query->m_instanceBase = (uint32_t)m_query->m_entities.size() + 1;
	m_query->m_instanceEntities.resize(0);

	CullingComponent* culling = world->getComponent< CullingComponent >();
	IEntityRenderer* cullingRenderer = (culling != nullptr) ? m_entityRenderers->find(type_of(culling)) : nullptr;
	if (cullingRenderer)
	{
		renderables.push_back({ cullingRenderer, culling, (float)m_query->m_instanceBase });

		std::sort(owners.begin(), owners.end());
		for (const auto instance : culling->getInstances())
		{
			const auto it = std::lower_bound(owners.begin(), owners.end(), std::pair< const Object*, Entity* >(instance->component, nullptr));
			m_query->m_instanceEntities.push_back((it != owners.end() && it->first == instance->component) ? it->second : nullptr);
		}
	}

	// Entity ids; single channel since ids are exact as floats up to 2^24.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR32F;

	// Share depth of visible surfaces so only those write their id.
	const render::RGTargetSet entityIdTargetSetId = renderGraph.addTransientTargetSet(L"Entity id", rgtd, depthTargetSetId, depthTargetSetId);

	Ref< render::RenderPass > rp = new render::RenderPass(L"Entity id");
	rp->addInput(depthTargetSetId);
	rp->addInput(visualTargetSetId);
	for (const auto& attachment : setupAttachments)
		rp->addInput(attachment);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	rp->setOutput(entityIdTargetSetId, clear, render::TfDepth, render::TfColor | render::TfDepth);

	rp->addBuild([=, this, renderables = std::move(renderables)](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
		const WorldBuildContext wc(m_entityRenderers, renderContext);

		auto sharedParams = renderContext->alloc< render::ProgramParameters >();
		sharedParams->beginParameters(renderContext);
		sharedParams->setFloatParameter(ShaderParameter::Time, (float)worldRenderView.getTime());
		sharedParams->setMatrixParameter(ShaderParameter::Projection, worldRenderView.getProjection());
		sharedParams->setMatrixParameter(ShaderParameter::View, worldRenderView.getView());
		sharedParams->setMatrixParameter(ShaderParameter::ViewInverse, worldRenderView.getView().inverse());
		sharedParams->endParameters(renderContext);

		EntityIdRenderPass entityIdRenderPass(sharedParams, worldRenderView);

		// Build each renderable by itself so every draw is stamped with the id of its entity.
		AlignedVector< Object* > objects(1);
		for (const auto& renderable : renderables)
		{
			entityIdRenderPass.setId(renderable.id);
			objects[0] = renderable.object;
			renderable.entityRenderer->build(wc, worldRenderView, entityIdRenderPass, objects);
		}
	});

	renderGraph.addPass(rp);

	// Copy id at requested position into read back slot; compute built into a pass runs
	// ahead of that pass' draws thus it must be a pass of its own after the id pass.
	const Vector2 position = m_query->m_position;
	Ref< render::Buffer > readBackBuffer = m_query->m_readBackBuffer;

	Ref< render::RenderPass > rrp = new render::RenderPass(L"Entity id read back");
	rrp->addInput(entityIdTargetSetId);
	rrp->addBuild([=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
		const auto entityIdTargetSet = renderGraph.getTargetSet(entityIdTargetSetId);
		const int32_t width = entityIdTargetSet->getWidth();
		const int32_t height = entityIdTargetSet->getHeight();
		const int32_t x = std::clamp((int32_t)(position.x * width), 0, width - 1);
		const int32_t y = std::clamp((int32_t)(position.y * height), 0, height - 1);

		auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"Entity id read back");
		renderBlock->program = m_readBackShader->getProgram().program;
		renderBlock->workSize[0] = 1;
		renderBlock->workSize[1] = 1;
		renderBlock->workSize[2] = 1;

		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setImageViewParameter(ShaderParameter::EntityIdInput, entityIdTargetSet->getColorTexture(0), 0);
		renderBlock->programParams->setBufferViewParameter(ShaderParameter::EntityIdBuffer, readBackBuffer->getBufferView());
		renderBlock->programParams->setVectorParameter(ShaderParameter::EntityIdParams, Vector4((float)x, (float)y, 0.0f, 0.0f));
		renderBlock->programParams->endParameters(renderContext);

		renderContext->compute(renderBlock);
	});

	renderGraph.addPass(rrp);

	m_query->m_state = EntityIdQuery::State::Rendered;
}

}
