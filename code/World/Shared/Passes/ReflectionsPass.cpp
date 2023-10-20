/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Timer/Profiler.h"
#include "Render/ITexture.h"
#include "Render/ScreenRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityRenderer.h"
#include "World/IWorldRenderer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Entity/ProbeComponent.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/Shared/Passes/ReflectionsPass.h"

namespace traktor::world
{
	namespace
	{

const resource::Id< render::ImageGraph > c_probeGlobalReflections(L"{79208A82-A65C-2045-A8AD-85E4DA2D160D}");
const resource::Id< render::ImageGraph > c_probeLocalReflections(L"{3DCB8715-1E44-074A-8D1D-151BE6A8BF81}");
const resource::Id< render::ImageGraph > c_screenReflections(L"{2F8EC56A-FD46-DF42-94B5-9DD676B8DD8A}");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ReflectionsPass", ReflectionsPass, Object)

ReflectionsPass::ReflectionsPass(
    const WorldRenderSettings& settings,
    WorldEntityRenderers* entityRenderers,
    render::IRenderTargetSet* sharedDepthStencil
)
:   m_settings(settings)
,   m_entityRenderers(entityRenderers)
,   m_sharedDepthStencil(sharedDepthStencil)
{
}

bool ReflectionsPass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	// Create probe reflections processes.
	if (desc.quality.reflections >= Quality::Low)
	{
		if (!resourceManager->bind(c_probeGlobalReflections, m_probeGlobalReflections))
		{
			log::error << L"Unable to create probe (global) reflections process." << Endl;
			return false;
		}
		if (!resourceManager->bind(c_probeLocalReflections, m_probeLocalReflections))
		{
			log::error << L"Unable to create probe (local) reflections process." << Endl;
			return false;
		}
	}

	// Create screen reflections processing.
	if (desc.quality.reflections >= Quality::High)
	{
		if (!resourceManager->bind(c_screenReflections, m_screenReflections))
		{
			log::error << L"Unable to create screen space reflections process." << Endl;
			return false;
		}
	}

	// Create screen renderer.
	if (m_probeGlobalReflections || m_probeLocalReflections || m_screenReflections)
	{
		m_screenRenderer = new render::ScreenRenderer();
		if (!m_screenRenderer->create(renderSystem))
			return false;

		m_reflectionsQuality = desc.quality.reflections;
	}
	return true;
}

render::handle_t ReflectionsPass::setup(
	const WorldRenderView& worldRenderView,
	const Entity* rootEntity,
    const GatherView& gatheredView,
	render::RenderGraph& renderGraph,
	render::handle_t gbufferTargetSetId,
	render::handle_t visualReadTargetSetId,
	render::handle_t outputTargetSetId
) const
{
	T_PROFILER_SCOPE(L"ReflectionsPass::setup");

	if (m_reflectionsQuality == Quality::Disabled)
		return 0;

	// Add reflections target.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.usingPrimaryDepthStencil = false;
	rgtd.ignoreStencil = true;
	rgtd.targets[0].colorFormat = render::TfR11G11B10F;

	switch (m_reflectionsQuality)
	{
	default:
	case Quality::Low:
		rgtd.referenceWidthDenom = 2;	// 50%
		rgtd.referenceHeightDenom = 2;
		break;

	case Quality::Medium:
		rgtd.referenceWidthMul = 2;		// 67%
		rgtd.referenceWidthDenom = 3;
		rgtd.referenceHeightMul = 2;
		rgtd.referenceHeightDenom = 3;
		break;

	case Quality::High:
		rgtd.referenceWidthMul = 4;		// 80%
		rgtd.referenceWidthDenom = 5;
		rgtd.referenceHeightMul = 4;
		rgtd.referenceHeightDenom = 5;
		break;

	case Quality::Ultra:
		rgtd.referenceWidthDenom = 1;	// 100%
		rgtd.referenceHeightDenom = 1;
		break;
	}

	auto reflectionsTargetSetId = renderGraph.addTransientTargetSet(L"Reflections", rgtd, m_sharedDepthStencil, outputTargetSetId);

	// Add reflections render pass.
	Ref< render::RenderPass > rp = new render::RenderPass(L"Reflections");

	rp->addInput(gbufferTargetSetId);
	rp->addInput(visualReadTargetSetId);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	rp->setOutput(reflectionsTargetSetId, clear, render::TfNone, render::TfColor);

	render::ImageGraphView view;
	view.viewFrustum = worldRenderView.getViewFrustum();
	view.view = worldRenderView.getView();
	view.lastView = worldRenderView.getLastView();
	view.projection = worldRenderView.getProjection();
	view.deltaTime = (float)worldRenderView.getDeltaTime();

	render::ImageGraphContext igctx;
	igctx.associateTextureTargetSet(s_handleInputColorLast, visualReadTargetSetId, 0);
	igctx.associateTextureTargetSet(s_handleInputDepth, gbufferTargetSetId, 0);
	igctx.associateTextureTargetSet(s_handleInputNormal, gbufferTargetSetId, 1);
	igctx.associateTextureTargetSet(s_handleInputRoughness, gbufferTargetSetId, 0);

	// Global reflections.
	if (m_probeGlobalReflections)
	{
		// Find first, non-local, probe.
		const ProbeComponent* probe = nullptr;
		for (auto p : gatheredView.probes)
		{
			if (!p->getLocal() && p->getTexture() != nullptr)
			{
				probe = p;
				break;
			}
		}

		if (probe != nullptr)
		{
			auto setParameters = [=](render::ProgramParameters* params) {
				params->setFloatParameter(s_handleProbeIntensity, probe->getIntensity());
				params->setFloatParameter(s_handleProbeTextureMips, (float)probe->getTexture()->getSize().mips);
				params->setTextureParameter(s_handleProbeTexture, probe->getTexture());
			};

			m_probeGlobalReflections->addPasses(
				m_screenRenderer,
				renderGraph,
				rp,
				igctx,
				view,
				setParameters
			);
		}
	}

	// Apply local reflections.
	if (m_probeLocalReflections)
	{
		for (auto p : gatheredView.probes)
		{
			if (p->getLocal() && p->getTexture() != nullptr)
			{
				const Transform& transform = p->getTransform();
				const Matrix44 worldView = worldRenderView.getView() * transform.toMatrix44();

				float distance;
				if (!worldRenderView.isBoxVisible(p->getVolume(), p->getTransform(), distance))
					continue;

				const Aabb3 worldVolume = p->getBoundingBox();

				auto setParameters = [=](render::ProgramParameters* params) {
					params->setFloatParameter(s_handleProbeIntensity, p->getIntensity());
					params->setFloatParameter(s_handleProbeTextureMips, (float)p->getTexture()->getSize().mips);
					params->setTextureParameter(s_handleProbeTexture, p->getTexture());
					params->setVectorParameter(s_handleProbeVolumeCenter,worldVolume.getCenter());
					params->setVectorParameter(s_handleProbeVolumeExtent, worldVolume.getExtent());
					params->setMatrixParameter(s_handleWorldView, worldView);
					params->setMatrixParameter(s_handleWorldViewInv, worldView.inverse());
				};

				m_probeLocalReflections->addPasses(
					m_screenRenderer,
					renderGraph,
					rp,
					igctx,
					view,
					setParameters
				);
			}
		}
	}

	// Apply screenspace traced reflections.
	if (m_screenReflections)
	{
		m_screenReflections->addPasses(
			m_screenRenderer,
			renderGraph,
			rp,
			igctx,
			view,
			nullptr
		);
	}

	renderGraph.addPass(rp);
	return reflectionsTargetSetId;
}

}
