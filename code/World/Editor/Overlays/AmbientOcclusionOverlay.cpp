/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Resource/IResourceManager.h"
#include "World/Editor/Overlays/AmbientOcclusionOverlay.h"

namespace traktor::world
{
	namespace
	{

const resource::Id< render::Shader > c_debugShader(Guid(L"{949B3C96-0196-F24E-B36E-98DD504BCE9D}"));
const render::Handle c_handleDebugTechnique(L"Default");
const render::Handle c_handleDebugAlpha(L"Scene_DebugAlpha");
const render::Handle c_handleDebugTexture(L"Scene_DebugTexture");

render::handle_t findTargetByName(const render::RenderGraph& renderGraph, const wchar_t* name)
{
	for (const auto& tm : renderGraph.getTargets())
	{
		if (wcscmp(tm.second.name, name) == 0)
			return tm.first;
	}
	return 0;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.AmbientOcclusionOverlay", 0, AmbientOcclusionOverlay, IDebugOverlay)

bool AmbientOcclusionOverlay::create(resource::IResourceManager* resourceManager)
{
	if (!resourceManager->bind(c_debugShader, m_shader))
		return false;

	return true;
}

void AmbientOcclusionOverlay::setup(render::RenderGraph& renderGraph, render::ScreenRenderer* screenRenderer, IWorldRenderer* worldRenderer, const WorldRenderView& worldRenderView, float alpha, float mip) const
{
	render::handle_t ambientOcclusionId = findTargetByName(renderGraph, L"Ambient occlusion");
	if (!ambientOcclusionId)
		return;

	Ref< render::RenderPass > rp = new render::RenderPass(L"Ambient occlusion overlay");
	rp->setOutput(0, render::TfColor, render::TfColor);
	rp->addInput(ambientOcclusionId);
	rp->addBuild([=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
		auto ambientOcclusionTargetSet = renderGraph.getTargetSet(ambientOcclusionId);
		if (!ambientOcclusionTargetSet)
			return;

		const render::Shader::Permutation perm(c_handleDebugTechnique);

		auto pp = renderContext->alloc< render::ProgramParameters >();
		pp->beginParameters(renderContext);
		pp->setFloatParameter(c_handleDebugAlpha, alpha);
		pp->setTextureParameter(c_handleDebugTexture, ambientOcclusionTargetSet->getColorTexture(0));
		pp->endParameters(renderContext);

		screenRenderer->draw(renderContext, m_shader, perm, pp);
	});
	renderGraph.addPass(rp);
}

}
