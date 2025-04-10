/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Editor/Overlays/TilesOverlay.h"

#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "World/Shared/Passes/LightClusterPass.h"
#include "World/Shared/WorldRendererShared.h"
#include "World/WorldRenderView.h"
#include "World/WorldHandles.h"

namespace traktor::world
{
namespace
{

const resource::Id< render::Shader > c_debugShader(Guid(L"{949B3C96-0196-F24E-B36E-98DD504BCE9D}"));
const render::Handle c_handleDebugTechnique(L"Tiles");
const render::Handle c_handleDebugAlpha(L"Scene_DebugAlpha");
const render::Handle c_handleDebugTexture(L"Scene_DebugTexture");
const render::Handle c_handleDebugMagicCoeffs(L"Scene_DebugMagicCoeffs");
const render::Handle c_handleDebugTileBuffer(L"World_TileSBuffer");
const render::Handle c_handleDebugViewDistance(L"World_ViewDistance");

render::RGTargetSet findTargetByName(const render::RenderGraph& renderGraph, const wchar_t* name)
{
	for (const auto& tm : renderGraph.getTargets())
		if (wcscmp(tm.second.name, name) == 0)
			return tm.first;
	return render::RGTargetSet::Invalid;
}

}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.TilesOverlay", 0, TilesOverlay, BaseOverlay)

bool TilesOverlay::create(resource::IResourceManager* resourceManager)
{
	if (!BaseOverlay::create(resourceManager))
		return false;

	if (!resourceManager->bind(c_debugShader, m_shader))
		return false;

	return true;
}

void TilesOverlay::setup(render::RenderGraph& renderGraph, render::ScreenRenderer* screenRenderer, World* world, IWorldRenderer* worldRenderer, const WorldRenderView& worldRenderView, float alpha, float mip) const
{
	WorldRendererShared* worldRendererShared = dynamic_type_cast< WorldRendererShared* >(worldRenderer);
	if (!worldRendererShared)
	{
		BaseOverlay::setup(renderGraph, screenRenderer, world, worldRenderer, worldRenderView, alpha, mip);
		return;
	}

	const render::RGTargetSet gbufferId = findTargetByName(renderGraph, L"GBuffer");
	if (gbufferId == render::RGTargetSet::Invalid)
	{
		BaseOverlay::setup(renderGraph, screenRenderer, world, worldRenderer, worldRenderView, alpha, mip);
		return;
	}

	const Matrix44& projection = worldRenderView.getProjection();
	const Matrix44& view = worldRenderView.getView();
	const Scalar p11 = projection.get(0, 0);
	const Scalar p22 = projection.get(1, 1);
	const Vector4 magicCoeffs(1.0f / p11, 1.0f / p22, 0.0f, 0.0f);

	const float viewNearZ = worldRenderView.getViewFrustum().getNearZ();
	const float viewFarZ = worldRenderView.getViewFrustum().getFarZ();
	const float viewSliceScale = ClusterDimZ / std::log(viewFarZ / viewNearZ);
	const float viewSliceBias = ClusterDimZ * std::log(viewNearZ) / std::log(viewFarZ / viewNearZ) - 0.001f;

	Ref< render::RenderPass > rp = new render::RenderPass(L"Tiles overlay");
	rp->setOutput(render::RGTargetSet::Output, render::TfColor, render::TfColor);
	rp->addInput(gbufferId);
	rp->addBuild([=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
		auto gbufferTargetSet = renderGraph.getTargetSet(gbufferId);
		if (!gbufferTargetSet)
			return;

		render::Buffer* tileSBuffer = worldRendererShared->m_lightClusterPass->getTileSBuffer();
		if (!tileSBuffer)
			return;

		const render::Shader::Permutation perm(c_handleDebugTechnique);

		auto pp = renderContext->alloc< render::ProgramParameters >();
		pp->beginParameters(renderContext);
		pp->setFloatParameter(c_handleDebugAlpha, alpha);
		pp->setVectorParameter(c_handleDebugViewDistance, Vector4(viewNearZ, viewFarZ, viewSliceScale, viewSliceBias));
		pp->setVectorParameter(c_handleDebugMagicCoeffs, magicCoeffs);
		pp->setMatrixParameter(s_handleProjection, projection);
		pp->setTextureParameter(c_handleDebugTexture, gbufferTargetSet->getColorTexture(0));
		pp->setBufferViewParameter(c_handleDebugTileBuffer, tileSBuffer->getBufferView());
		pp->endParameters(renderContext);

		screenRenderer->draw(renderContext, m_shader, perm, pp);
	});
	renderGraph.addPass(rp);
}

}
