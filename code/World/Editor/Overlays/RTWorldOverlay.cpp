/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
//#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Resource/IResourceManager.h"
#include "World/World.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Editor/Overlays/RTWorldOverlay.h"
#include "World/Entity/RTWorldComponent.h"

namespace traktor::world
{
	namespace
	{

const resource::Id< render::Shader > c_debugShader(Guid(L"{385D649D-56CD-EF47-8D99-6105EA86E849}"));

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.RTWorldOverlay", 0, RTWorldOverlay, BaseOverlay)

bool RTWorldOverlay::create(resource::IResourceManager* resourceManager)
{
	if (!BaseOverlay::create(resourceManager))
		return false;

	if (!resourceManager->bind(c_debugShader, m_shader))
		return false;

	return true;
}

void RTWorldOverlay::setup(render::RenderGraph& renderGraph, render::ScreenRenderer* screenRenderer, World* world, IWorldRenderer* worldRenderer, const WorldRenderView& worldRenderView, float alpha, float mip) const
{
	RTWorldComponent* rtw = world->getComponent< RTWorldComponent >();
	if (!rtw || !rtw->getTopLevel())
	{
		BaseOverlay::setup(renderGraph, screenRenderer, world, worldRenderer, worldRenderView, alpha, mip);
		return;
	}

	const Matrix44 viewInverse = worldRenderView.getView().inverse();

	Ref< render::RenderPass > rp = new render::RenderPass(L"RTWorld overlay");
	rp->setOutput(0, render::TfColor, render::TfColor);
	rp->addBuild([=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {

		auto pp = renderContext->alloc< render::ProgramParameters >();
		pp->beginParameters(renderContext);
		pp->setMatrixParameter(s_handleViewInverse, viewInverse);
		pp->setAccelerationStructureParameter(s_handleTLAS, rtw->getTopLevel());
		pp->endParameters(renderContext);

		screenRenderer->draw(renderContext, m_shader, pp);
	});
	renderGraph.addPass(rp);
}

}
