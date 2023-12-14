/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Random.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/Image2/DirectionalBlur.h"

namespace traktor::render
{
	namespace
	{

const static Handle s_handleViewFar(L"ViewFar");
const static Handle s_handleViewEdgeTopLeft(L"ViewEdgeTopLeft");
const static Handle s_handleViewEdgeTopRight(L"ViewEdgeTopRight");
const static Handle s_handleViewEdgeBottomLeft(L"ViewEdgeBottomLeft");
const static Handle s_handleViewEdgeBottomRight(L"ViewEdgeBottomRight");
const static Handle s_handleMagicCoeffs(L"MagicCoeffs");
const static Handle s_handleDirection(L"Direction");
const static Handle s_handleNoiseOffset(L"NoiseOffset");
const static Handle s_handleGaussianOffsetWeights(L"GaussianOffsetWeights");
const static Handle s_handleProjection(L"Projection");
const static Handle s_handleView(L"View");
const static Handle s_handleViewInverse(L"ViewInverse");

Random s_random;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.DirectionalBlur", DirectionalBlur, ImagePassStep)

void DirectionalBlur::addRenderPassInputs(
	const ImageGraph* graph,
	const ImageGraphContext& context,
	RenderPass& pass
) const
{
	addInputs(context, pass);
}

void DirectionalBlur::build(
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

	// Setup parameters for the shader.
	auto pp = renderContext->alloc< ProgramParameters >();
	pp->beginParameters(renderContext);
	pp->attachParameters(sharedParams);

	pp->setFloatParameter(s_handleViewFar, view.viewFrustum.getFarZ());
	pp->setVectorParameter(s_handleViewEdgeTopLeft, viewEdgeTopLeft);
	pp->setVectorParameter(s_handleViewEdgeTopRight, viewEdgeTopRight);
	pp->setVectorParameter(s_handleViewEdgeBottomLeft, viewEdgeBottomLeft);
	pp->setVectorParameter(s_handleViewEdgeBottomRight, viewEdgeBottomRight);
	pp->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleDirection, m_direction * Scalar(0.5f));
	pp->setVectorParameter(s_handleNoiseOffset, Vector4(s_random.nextFloat(), s_random.nextFloat(), 0.0f, 0.0f));
	pp->setVectorArrayParameter(s_handleGaussianOffsetWeights, &m_gaussianOffsetWeights[0], (uint32_t)m_gaussianOffsetWeights.size());
	pp->setMatrixParameter(s_handleProjection, view.projection);
	pp->setMatrixParameter(s_handleView, view.view);
	pp->setMatrixParameter(s_handleViewInverse, view.view.inverse());

	bindSources(context, renderGraph, pp);

	pp->endParameters(renderContext);

	// Draw fullscreen quad with shader.
	screenRenderer->draw(renderContext, m_shader, Shader::Permutation(), pp);
}

}
