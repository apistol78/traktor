/*
 * TRAKTOR
 * Copyright (c) 2023-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Math/Log2.h"
#include "Core/Misc/String.h"
#include "Core/Timer/Profiler.h"
#include "Render/Buffer.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Resource/IResourceManager.h"
#include "World/IEntityRenderer.h"
#include "World/WorldBuildContext.h"
#include "World/WorldEntityRenderers.h"
#include "World/WorldHandles.h"
#include "World/WorldRenderView.h"
#include "World/Shared/WorldRenderPassShared.h"
#include "World/Shared/Passes/HiZPass.h"

namespace traktor::world
{
	namespace
	{

const resource::Id< render::Shader > c_hiZBuildShader(L"{E8879B75-F646-5D46-8873-E11A518C5256}");

const render::Handle s_handleHiZInput(L"World_HiZInput");
const render::Handle s_handleHiZOutput(L"World_HiZOutput");

const render::Handle s_persistentHiZTexture[] =
{
	render::Handle(L"World_HiZTexture_0"),
	render::Handle(L"World_HiZTexture_1"),
	render::Handle(L"World_HiZTexture_2"),
	render::Handle(L"World_HiZTexture_3")
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.HiZPass", HiZPass, Object)

bool HiZPass::create(resource::IResourceManager* resourceManager)
{
	if (!resourceManager->bind(c_hiZBuildShader, m_hiZBuildShader))
	{
		log::error << L"Unable to create HiZ process." << Endl;
		return false;
	}
	return true;
}

render::handle_t HiZPass::addTexture(const WorldRenderView& worldRenderView, render::RenderGraph& renderGraph) const
{
	const Vector2 viewSize = worldRenderView.getViewSize();
	const int32_t viewWidth = (int32_t)viewSize.x;
	const int32_t viewHeight = (int32_t)viewSize.y;
	const int32_t hiZWidth = nearestLog2(viewWidth >> 1);
	const int32_t hiZHeight = nearestLog2(viewHeight >> 1);
	const int32_t hiZMipCount = log2(std::max(hiZWidth, hiZHeight)) + 1;

	render::RenderGraphTextureDesc rgtxd;
	rgtxd.width = hiZWidth;
	rgtxd.height = hiZHeight;
	rgtxd.mipCount = hiZMipCount;
	rgtxd.format = render::TfR32F;
	return renderGraph.addPersistentTexture(L"HiZ", s_persistentHiZTexture[worldRenderView.getIndex()], rgtxd);
}

void HiZPass::setup(
	const WorldRenderView& worldRenderView,
	render::RenderGraph& renderGraph,
	render::handle_t gbufferTargetSetId,
	render::handle_t outputHiZTextureId
) const
{
	T_PROFILER_SCOPE(L"HiZPass::setup");

	const Vector2 viewSize = worldRenderView.getViewSize();
	const int32_t viewWidth = (int32_t)viewSize.x;
	const int32_t viewHeight = (int32_t)viewSize.y;
	const int32_t hiZWidth = nearestLog2(viewWidth >> 1);
	const int32_t hiZHeight = nearestLog2(viewHeight >> 1);
	const int32_t hiZMipCount = log2(std::max(hiZWidth, hiZHeight)) + 1;

	Ref< render::RenderPass > rp = new render::RenderPass(L"HiZ");
	rp->addInput(gbufferTargetSetId);
	rp->setOutput(outputHiZTextureId);

	for (int32_t i = 0; i < hiZMipCount; ++i)
	{
		const int32_t mipWidth = std::max(hiZWidth >> i, 1);
		const int32_t mipHeight = std::max(hiZHeight >> i, 1);

		const int32_t workWidth = std::max(viewWidth >> (i + 1), 1);
		const int32_t workHeight = std::max(viewHeight >> (i + 1), 1);

		T_FATAL_ASSERT(workWidth <= mipWidth);
		T_FATAL_ASSERT(workHeight <= mipHeight);

		rp->addBuild(
			[=, this](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
			{
				auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"HiZ");

				renderBlock->program = m_hiZBuildShader->getProgram().program;
				renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlock->workSize[0] = mipWidth;
				renderBlock->workSize[1] = mipHeight;
				renderBlock->workSize[2] = 1;

				renderBlock->programParams->beginParameters(renderContext);

				renderBlock->programParams->setVectorParameter(render::getParameterHandle(L"World_HiZWorkSize"), Vector4(workWidth, workHeight, 0.0f, 0.0f));

				const auto outputTexture = renderGraph.getTexture(outputHiZTextureId);
				renderBlock->programParams->setImageViewParameter(s_handleHiZOutput, outputTexture, i);

				if (i == 0)
				{
					const auto inputTexture = renderGraph.getTargetSet(gbufferTargetSetId)->getColorTexture(0);
					renderBlock->programParams->setImageViewParameter(s_handleHiZInput, inputTexture, 0);
				}
				else
				{
					const auto inputTexture = outputTexture;
					renderBlock->programParams->setImageViewParameter(s_handleHiZInput, inputTexture, i - 1);
				}

				renderBlock->programParams->endParameters(renderContext);

				renderContext->compute(renderBlock);
				renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Compute, outputTexture, i);
			}
		);
	}

	renderGraph.addPass(rp);
}

}
