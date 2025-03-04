/*
 * TRAKTOR
 * Copyright (c) 2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Shared/Passes/DownScalePass.h"

#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Misc/String.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/IRenderTargetSet.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityRenderer.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"

namespace traktor::world
{
namespace
{

const resource::Id< render::Shader > c_downScaleShader(L"{BC74A385-9406-884D-9FB5-EA84F413185C}");

const render::Handle s_handleDownScaleInput(L"World_DownScaleInput");
const render::Handle s_handleDownScaleOutput(L"World_DownScaleOutput");
const render::Handle s_handleDownScaleWorkSize(L"World_DownScaleWorkSize");

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.DownScalePass", DownScalePass, Object)

bool DownScalePass::create(resource::IResourceManager* resourceManager)
{
	if (!resourceManager->bind(c_downScaleShader, m_downScaleShader))
	{
		log::error << L"Unable to create down-scale process." << Endl;
		return false;
	}
	return true;
}

render::handle_t DownScalePass::setup(
	const WorldRenderView& worldRenderView,
	render::RenderGraph& renderGraph,
	render::handle_t gbufferTargetSetId) const
{
	T_PROFILER_SCOPE(L"DownScalePass::setup");

	const render::RenderGraphTextureDesc rgtxd = {
		.referenceWidthDenom = 2,
		.referenceHeightDenom = 2,
		.mipCount = 1,
		.format = render::TfR16G16B16A16F
	};
	const render::handle_t downScaleTextureId = renderGraph.addTransientTexture(L"DownScale", rgtxd);

	Ref< render::RenderPass > rp = new render::RenderPass(L"DownScale");
	rp->addInput(gbufferTargetSetId);
	rp->setOutput(downScaleTextureId);
	rp->addBuild(
		[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext) {
		render::ITexture* inputTexture = renderGraph.getTargetSet(gbufferTargetSetId)->getColorTexture(0);
		render::ITexture* outputTexture = renderGraph.getTexture(downScaleTextureId);

		const render::ITexture::Size inputSize = inputTexture->getSize();
		const render::ITexture::Size outputSize = outputTexture->getSize();

		auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"DownScale");

		renderBlock->program = m_downScaleShader->getProgram().program;
		renderBlock->workSize[0] = outputSize.x;
		renderBlock->workSize[1] = outputSize.y;
		renderBlock->workSize[2] = 1;

		renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
		renderBlock->programParams->beginParameters(renderContext);
		renderBlock->programParams->setVectorParameter(s_handleDownScaleWorkSize, Vector4(outputSize.x, outputSize.y, inputSize.x, inputSize.y));
		renderBlock->programParams->setImageViewParameter(s_handleDownScaleOutput, outputTexture, 0);
		renderBlock->programParams->setImageViewParameter(s_handleDownScaleInput, inputTexture, 0);
		renderBlock->programParams->endParameters(renderContext);

		renderContext->compute(renderBlock);
		renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Compute, outputTexture, 0);
	});
	renderGraph.addPass(rp);

	return downScaleTextureId;
}

}
