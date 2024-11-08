/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Resource/IResourceManager.h"
#include "World/Editor/Overlays/BaseOverlay.h"

namespace traktor::world
{
	namespace
	{

const resource::Id< render::Shader > c_debugShader(Guid(L"{949B3C96-0196-F24E-B36E-98DD504BCE9D}"));
const render::Handle c_handleDebugTechnique(L"Invalid");
const render::Handle c_handleDebugAlpha(L"Scene_DebugAlpha");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.BaseOverlay", BaseOverlay, IDebugOverlay)

bool BaseOverlay::create(resource::IResourceManager* resourceManager)
{
	if (!resourceManager->bind(c_debugShader, m_shader))
		return false;

	return true;
}

void BaseOverlay::setup(render::RenderGraph& renderGraph, render::ScreenRenderer* screenRenderer, World* world, IWorldRenderer* worldRenderer, const WorldRenderView& worldRenderView, float alpha, float mip) const
{
	Ref< render::RenderPass > rp = new render::RenderPass(L"Invalid overlay");
	rp->setOutput(0, render::TfColor, render::TfColor);
	rp->addBuild([=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
		const render::Shader::Permutation perm(c_handleDebugTechnique);

		auto pp = renderContext->alloc< render::ProgramParameters >();
		pp->beginParameters(renderContext);
		pp->setFloatParameter(c_handleDebugAlpha, alpha);
		pp->endParameters(renderContext);

		screenRenderer->draw(renderContext, m_shader, perm, pp);
	});
	renderGraph.addPass(rp);
}

}
