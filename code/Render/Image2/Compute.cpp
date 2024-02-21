/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Random.h"
#include "Render/Buffer.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/Image2/Compute.h"

namespace traktor::render
{
	namespace
	{

const static Handle s_handleOutput(L"Output");
const static Handle s_handleViewEdgeTopLeft(L"ViewEdgeTopLeft");
const static Handle s_handleViewEdgeTopRight(L"ViewEdgeTopRight");
const static Handle s_handleViewEdgeBottomLeft(L"ViewEdgeBottomLeft");
const static Handle s_handleViewEdgeBottomRight(L"ViewEdgeBottomRight");
const static Handle s_handleProjection(L"Projection");
const static Handle s_handleView(L"View");
const static Handle s_handleViewInverse(L"ViewInverse");
const static Handle s_handleLastView(L"LastView");
const static Handle s_handleLastViewInverse(L"LastViewInverse");
const static Handle s_handleMagicCoeffs(L"MagicCoeffs");
const static Handle s_handleNoiseOffset(L"NoiseOffset");

Random s_random;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Compute", Compute, ImagePassStep)

void Compute::addRenderPassInputs(
	const ImageGraph* graph,
	const ImageGraphContext& context,
	RenderPass& pass
) const
{
	addInputs(context, pass);
}

void Compute::build(
	const ImageGraph* graph,
	const ImageGraphContext& context,
	const ImageGraphView& view,
	const targetSetVector_t& targetSetIds,
	const targetSetVector_t& sbufferIds,
	const PassOutput& output,
	const RenderGraph& renderGraph,
	const ProgramParameters* sharedParams,
	RenderContext* renderContext,
	ScreenRenderer* screenRenderer
) const
{
	const Scalar p11 = view.projection.get(0, 0);
	const Scalar p22 = view.projection.get(1, 1);
	const Vector4 viewEdgeTopLeft = view.viewFrustum.corners[4];
	const Vector4 viewEdgeTopRight = view.viewFrustum.corners[5];
	const Vector4 viewEdgeBottomLeft = view.viewFrustum.corners[7];
	const Vector4 viewEdgeBottomRight = view.viewFrustum.corners[6];

	auto rb = renderContext->allocNamed< ComputeRenderBlock >(L"Image - compute");

	// Setup parameters for the shader.
	auto pp = renderContext->alloc< ProgramParameters >();
	pp->beginParameters(renderContext);
	pp->attachParameters(sharedParams);

	pp->setVectorParameter(s_handleViewEdgeTopLeft, viewEdgeTopLeft);
	pp->setVectorParameter(s_handleViewEdgeTopRight, viewEdgeTopRight);
	pp->setVectorParameter(s_handleViewEdgeBottomLeft, viewEdgeBottomLeft);
	pp->setVectorParameter(s_handleViewEdgeBottomRight, viewEdgeBottomRight);
	pp->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleNoiseOffset, Vector4(s_random.nextFloat(), s_random.nextFloat(), 0.0f, 0.0f));
	pp->setMatrixParameter(s_handleProjection, view.projection);
	pp->setMatrixParameter(s_handleView, view.view);
	pp->setMatrixParameter(s_handleViewInverse, view.view.inverse());
	pp->setMatrixParameter(s_handleLastView, view.lastView);
	pp->setMatrixParameter(s_handleLastViewInverse, view.lastView.inverse());

	if (output.sbuffer >= 0)
	{
		auto buffer = renderGraph.getBuffer(sbufferIds[output.sbuffer]);
		pp->setBufferViewParameter(s_handleOutput, buffer->getBufferView());
	}

	switch (m_workSize)
	{
	case WorkSize::Manual:
		{
			rb->workSize[0] = m_manualWorkSize[0];
			rb->workSize[1] = m_manualWorkSize[1];
			rb->workSize[2] = m_manualWorkSize[2];
		}
		break;

	case WorkSize::Output:
		if (output.targetSet >= 0)
		{
			auto targetSet = renderGraph.getTargetSet(targetSetIds[output.targetSet]);
			rb->workSize[0] = targetSet->getWidth();
			rb->workSize[1] = targetSet->getHeight();
		}
		break;

	case WorkSize::SizeOf:
		{
			for (const auto& source : m_textureSources)
			{
				if (source.shaderParameter == render::getParameterHandle(L"WorkSize"))
				{
					const auto texture = context.findTexture(renderGraph, source.id);
					const auto textureSize = texture->getSize();
					rb->workSize[0] = textureSize.x;
					rb->workSize[1] = textureSize.y;
					break;
				}
			}
		}
		break;
	}

	bindSources(context, renderGraph, pp);

	pp->endParameters(renderContext);

	IProgram* program = m_shader->getProgram().program;
	if (!program)
		return;

	rb->program = program;
	rb->programParams = pp;
	renderContext->compute(rb);

	renderContext->compute< BarrierRenderBlock >(Stage::Compute, Stage::Fragment | Stage::Compute, nullptr, 0);
}

}
