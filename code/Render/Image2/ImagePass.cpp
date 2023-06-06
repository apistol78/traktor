/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/ITexture.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/Image2/ImagePass.h"
#include "Render/Image2/ImagePassStep.h"

namespace traktor::render
{
	namespace
	{

const static Handle s_handleTime(L"Time");
const static Handle s_handleDeltaTime(L"DeltaTime");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImagePass", ImagePass, Object)

void ImagePass::addRenderGraphPasses(
	const ImageGraph* graph,
	const ImageGraphContext& context,
	const ImageGraphView& view,
	const targetSetVector_t& targetSetIds,
	const targetSetVector_t& sbufferIds,
	ScreenRenderer* screenRenderer,
	RenderGraph& renderGraph
) const
{
	Ref< RenderPass > rp = new RenderPass(m_name);

	for (auto step : m_steps)
		step->addRenderPassInputs(graph, context, *rp);

	if (m_outputTargetSet >= 0)
	{
		uint32_t load = render::TfNone;

		if ((m_clear.mask & render::CfColor) == 0)
			load |= render::TfColor;

		rp->setOutput(targetSetIds[m_outputTargetSet], m_clear, load, render::TfColor);
	}

	if (m_outputSBuffer >= 0)
		rp->setOutput(sbufferIds[m_outputSBuffer]);

	rp->addBuild(
		[=](const RenderGraph& renderGraph, RenderContext* renderContext)
		{
			auto sharedParams = renderContext->alloc< ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			
			sharedParams->setFloatParameter(s_handleTime, view.time);
			sharedParams->setFloatParameter(s_handleDeltaTime, view.deltaTime);

			for (auto it : context.getFloatParameters())
				sharedParams->setFloatParameter(it.first, it.second);
			for (auto it : context.getVectorParameters())
				sharedParams->setVectorParameter(it.first, it.second);

			sharedParams->endParameters(renderContext);
				
			for (auto step : m_steps)
				step->build(
					graph,
					context,
					view,
					renderGraph,
					sharedParams,
					renderContext,
					screenRenderer
				);
		}
	);

	renderGraph.addPass(rp);
}

}
