/*
 * TRAKTOR
 * Copyright (c) 2023-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
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

	const int32_t hiZWidth = viewWidth >> 1;
	const int32_t hiZHeight = viewHeight >> 1;

	const int32_t hiZMipCount = log2(std::max(hiZWidth, hiZHeight)) + 1;

	Ref< render::RenderPass > rp = new render::RenderPass(L"HiZ");
	rp->addInput(gbufferTargetSetId);
	rp->setOutput(outputHiZTextureId);

	for (int32_t i = 0; i < hiZMipCount; ++i)
	{
		const int32_t mipWidth = std::max(hiZWidth >> i, 1);
		const int32_t mipHeight = std::max(hiZHeight >> i, 1);

		const bool first = bool(i == 0);

		rp->addBuild(
			[=](const render::RenderGraph& renderGraph, render::RenderContext* renderContext)
			{
				auto renderBlock = renderContext->allocNamed< render::ComputeRenderBlock >(L"HiZ");

				renderBlock->program = m_hiZBuildShader->getProgram().program;
				renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
				renderBlock->workSize[0] = mipWidth;
				renderBlock->workSize[1] = mipHeight;
				renderBlock->workSize[2] = 1;

				renderBlock->programParams->beginParameters(renderContext);

				if (i == 0)
				{
					const auto inputTexture = renderGraph.getTargetSet(gbufferTargetSetId)->getColorTexture(0);
					const auto outputTexture = renderGraph.getTexture(outputHiZTextureId);

					renderBlock->programParams->setImageViewParameter(s_handleHiZInput, inputTexture, 0);
					renderBlock->programParams->setImageViewParameter(s_handleHiZOutput, outputTexture, 0);
				}
				else
				{
					const auto inoutTexture = renderGraph.getTexture(outputHiZTextureId);

					renderBlock->programParams->setImageViewParameter(s_handleHiZInput, inoutTexture, i - 1);
					renderBlock->programParams->setImageViewParameter(s_handleHiZOutput, inoutTexture, i);
				}

				renderBlock->programParams->endParameters(renderContext);

				renderContext->compute(renderBlock);
				renderContext->compute< render::BarrierRenderBlock >(render::Stage::Compute, render::Stage::Compute);
			}
		);
	}

	renderGraph.addPass(rp);
}

}
