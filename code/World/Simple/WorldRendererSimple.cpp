/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"
#include "Core/Timer/Profiler.h"
#include "Render/IRenderSystem.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "World/Entity.h"
#include "World/IEntityComponent.h"
#include "World/IEntityRenderer.h"
#include "World/World.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderSettings.h"
#include "World/WorldRenderView.h"
#include "World/WorldSetupContext.h"
#include "World/Simple/WorldRendererSimple.h"
#include "World/Simple/WorldRenderPassSimple.h"

namespace traktor::world
{
	namespace
	{

Ref< render::ITexture > create1x1Texture(render::IRenderSystem* renderSystem, float value)
{
	render::SimpleTextureCreateDesc stcd = {};
	stcd.width = 1;
	stcd.height = 1;
	stcd.mipCount = 1;
	stcd.format = render::TfR32F;
	stcd.sRGB = false;
	stcd.immutable = true;
	stcd.initialData[0].data = &value;
	stcd.initialData[0].pitch = 4;
	return renderSystem->createSimpleTexture(stcd, T_FILE_LINE_W);
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.WorldRendererSimple", 0, WorldRendererSimple, IWorldRenderer)

bool WorldRendererSimple::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const WorldCreateDesc& desc
)
{
	m_entityRenderers = desc.entityRenderers;
	m_depthTexture = create1x1Texture(renderSystem, desc.worldRenderSettings->viewFarZ);
	return true;
}

void WorldRendererSimple::destroy()
{
	m_entityRenderers = nullptr;
	safeDestroy(m_depthTexture);
}

void WorldRendererSimple::setup(
	const World* world,
	const WorldRenderView& worldRenderView,
	render::RenderGraph& renderGraph,
	render::handle_t outputTargetSetId,
	const std::function< bool(const EntityState& state) >& filter
)
{
	// Gather active renderables for this frame.
	{
		T_PROFILER_SCOPE(L"WorldRendererSimple gather");

		m_gathered.resize(0);
		for (auto entity : world->getEntities())
		{
			if (filter != nullptr && filter(entity->getState()) == false)
				continue;

			IEntityRenderer* entityRenderer = m_entityRenderers->find(type_of(entity));
			if (entityRenderer)
				m_gathered.push_back({ entityRenderer, entity });

			for (auto component : entity->getComponents())
			{
				IEntityRenderer* entityRenderer = m_entityRenderers->find(type_of(component));
				if (entityRenderer)
					m_gathered.push_back({ entityRenderer, component });
			}
		}
	}

	// Add additional passes by entity renderers.
	{
		T_PROFILER_SCOPE(L"WorldRendererSimple setup extra passes");
		const WorldSetupContext context(world, m_entityRenderers, nullptr, renderGraph);

		for (auto gathered : m_gathered)
			gathered.entityRenderer->setup(context, worldRenderView, gathered.renderable);
	
		for (auto entityRenderer : m_entityRenderers->get())
			entityRenderer->setup(context);
	}

	// Add passes to render graph.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Visual");

	render::Clear cl;
	cl.mask = render::CfColor | render::CfDepth;
	cl.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	cl.depth = 1.0f;
	rp->setOutput(outputTargetSetId, cl, render::TfNone, render::TfAll);

	rp->addBuild(
		[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
		{
			const WorldBuildContext wc(
				m_entityRenderers,
				renderContext
			);

			// Default visual parameters.
			auto globalProgramParams = renderContext->alloc< render::ProgramParameters >();
			globalProgramParams->beginParameters(renderContext);
			globalProgramParams->setFloatParameter(s_handleTime, (float)worldRenderView.getTime());
			globalProgramParams->setMatrixParameter(s_handleProjection, worldRenderView.getProjection());
			globalProgramParams->setTextureParameter(s_handleGBufferDepthMap, m_depthTexture);
			globalProgramParams->endParameters(renderContext);

			// Build visual context.
			const WorldRenderPassSimple defaultPass(
				s_techniqueSimpleColor,
				globalProgramParams,
				worldRenderView.getView()
			);

			for (auto gathered : m_gathered)
				gathered.entityRenderer->build(wc, worldRenderView, defaultPass, gathered.renderable);
	
			for (auto entityRenderer : m_entityRenderers->get())
				entityRenderer->build(wc, worldRenderView, defaultPass);
		}
	);
	renderGraph.addPass(rp);
}

}
