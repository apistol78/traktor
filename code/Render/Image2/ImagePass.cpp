/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Buffer.h"
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

RenderPass* ImagePass::addRenderGraphPasses(
	const ImageGraph* graph,
	const ImageGraphContext& context,
	const ImageGraphView& view,
	const targetSetVector_t& targetSetIds,
	const targetSetVector_t& sbufferIds,
	const std::function< void(const RenderGraph& renderGraph, ProgramParameters*) >& parametersFn,
	ScreenRenderer* screenRenderer,
	RenderGraph& renderGraph
) const
{
	Ref< RenderPass > rp = new RenderPass(m_name);

	// Add inputs from all steps of this pass.
	for (auto step : m_steps)
		step->addRenderPassInputs(graph, context, *rp);

	// Set pass output, either render target or a sbuffer.
	if (m_output.targetSet >= 0)
	{
		uint32_t load = render::TfNone;
		if ((m_clear.mask & render::CfColor) == 0)
			load |= render::TfColor;
		rp->setOutput(targetSetIds[m_output.targetSet], m_clear, load, render::TfColor);
	}
	if (m_output.sbuffer >= 0)
		rp->setOutput(sbufferIds[m_output.sbuffer]);

	// Add this pass build which setup shared parameters for all steps.
	rp->addBuild(
		[=, this](const RenderGraph& renderGraph, RenderContext* renderContext)
		{
			auto sharedParams = renderContext->alloc< ProgramParameters >();
			sharedParams->beginParameters(renderContext);
			
			sharedParams->setFloatParameter(s_handleTime, view.time);
			sharedParams->setFloatParameter(s_handleDeltaTime, view.deltaTime);

			if (parametersFn)
				parametersFn(renderGraph, sharedParams);

			sharedParams->endParameters(renderContext);
				
			for (auto step : m_steps)
			{
				step->build(
					graph,
					context,
					view,
					targetSetIds,
					sbufferIds,
					m_output,
					renderGraph,
					sharedParams,
					renderContext,
					screenRenderer
				);
			}
		}
	);

	renderGraph.addPass(rp);
	return rp;
}

}
