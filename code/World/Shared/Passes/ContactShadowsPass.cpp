/*
 * TRAKTOR
 * Copyright (c) 2023-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/ContactShadowsPass.h"

#include "Core/Log/Log.h"
#include "Core/Timer/Profiler.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Resource/IResourceManager.h"
#include "World/Entity/LightComponent.h"
#include "World/IEntityRenderer.h"
#include "World/IWorldRenderer.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"

namespace traktor::world
{
namespace
{

const resource::Id< render::ImageGraph > c_contactShadows(L"{9A8273AE-06A0-8649-A24F-11D5F93255C9}");

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.ContactShadowsPass", ContactShadowsPass, Object)

bool ContactShadowsPass::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const WorldCreateDesc& desc)
{
	// Create contact shadows processing.
	if (desc.quality.ambientOcclusion > Quality::Disabled)
	{
		if (!resourceManager->bind(c_contactShadows, m_contactShadows))
		{
			log::error << L"Unable to create contact shadows process." << Endl;
			return false;
		}
	}

	// Create screen renderer.
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	return true;
}

render::RGTargetSet ContactShadowsPass::setup(
	const WorldRenderView& worldRenderView,
	const GatherView& gatheredView,
	render::RenderGraph& renderGraph,
	render::RGTargetSet gbufferTargetSetId,
	render::RGTargetSet outputTargetSetId) const
{
	T_PROFILER_SCOPE(L"ContactShadowsPass::setup");
	render::ImageGraphView view;

	if (m_contactShadows == nullptr || gbufferTargetSetId == render::RGTargetSet::Invalid)
		return render::RGTargetSet::Invalid;

	// Find major directional light.
	const LightComponent* majorLight = nullptr;
	for (auto light : gatheredView.lights)
	{
		if (light != nullptr && light->getLightType() == LightType::Directional && light->getCastShadow())
		{
			majorLight = light;
			break;
		}
	}
	if (majorLight == nullptr)
		return render::RGTargetSet::Invalid;

	const Vector4 lightDirection = worldRenderView.getView() * majorLight->getTransform().axisY();

	// Add contact shadows target set.
	render::RenderGraphTargetSetDesc rgtd;
	rgtd.count = 1;
	rgtd.createDepthStencil = false;
	rgtd.referenceWidthDenom = 1;
	rgtd.referenceHeightDenom = 1;
	rgtd.targets[0].colorFormat = render::TfR8; // Shadow (R)
	auto contactShadowsTargetSetId = renderGraph.addTransientTargetSet(L"Contact shadows", rgtd, render::RGTargetSet::Invalid, outputTargetSetId);

	// Add contact shadows render pass.
	view.viewFrustum = worldRenderView.getViewFrustum();
	view.view = worldRenderView.getView();
	view.projection = worldRenderView.getProjection();

	render::ImageGraphContext igctx;

	Ref< render::RenderPass > rp = new render::RenderPass(L"Contact shadows");
	rp->addInput(gbufferTargetSetId);

	render::Clear clear;
	clear.mask = render::CfColor;
	clear.colors[0] = Color4f(0.0f, 0.0f, 0.0f, 0.0f);
	rp->setOutput(contactShadowsTargetSetId, clear, render::TfNone, render::TfColor);

	auto setParameters = [=](const render::RenderGraph& renderGraph, render::ProgramParameters* params) {
		const auto gbufferTargetSet = renderGraph.getTargetSet(gbufferTargetSetId);
		params->setTextureParameter(ShaderParameter::GBufferA, gbufferTargetSet->getColorTexture(0));
		params->setTextureParameter(ShaderParameter::GBufferB, gbufferTargetSet->getColorTexture(1));
		params->setTextureParameter(ShaderParameter::GBufferC, gbufferTargetSet->getColorTexture(2));
		params->setVectorParameter(ShaderParameter::ContactLightDirection, lightDirection);
	};

	m_contactShadows->addPasses(
		m_screenRenderer,
		renderGraph,
		rp,
		igctx,
		view,
		setParameters);

	renderGraph.addPass(rp);
	return contactShadowsTargetSetId;
}

}
