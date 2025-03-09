/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Image2/ShadowProject.h"

#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"

namespace traktor::render
{
namespace
{

const float c_sliceBias = 0.0001f; //!< Extra slice dimension bias; slight overlap over slices.

const Vector4 c_poissonTaps[] = {
	Vector4(-0.326212f, -0.40581f, 0.519456f, 0.767022f),
	Vector4(-0.840144f, -0.07358f, 0.185461f, -0.893124f),
	Vector4(-0.695914f, 0.457137f, 0.507431f, 0.064425f),
	Vector4(-0.203345f, 0.620716f, 0.89642f, 0.412458f),
	Vector4(0.96234f, -0.194983f, -0.32194f, -0.932615f),
	Vector4(0.473434f, -0.480026f, -0.791559f, -0.59771f)
};

const static Handle s_handleLastSlice(L"LastSlice");
const static Handle s_handleShadowMapSizeAndBias(L"ShadowMapSizeAndBias");
const static Handle s_handleShadowMapUvTransform(L"ShadowMapUvTransform");
const static Handle s_handleMagicCoeffs(L"MagicCoeffs");
const static Handle s_handleViewEdgeTopLeft(L"ViewEdgeTopLeft");
const static Handle s_handleViewEdgeTopRight(L"ViewEdgeTopRight");
const static Handle s_handleViewEdgeBottomLeft(L"ViewEdgeBottomLeft");
const static Handle s_handleViewEdgeBottomRight(L"ViewEdgeBottomRight");
const static Handle s_handleShadowMapPoissonTaps(L"ShadowMapPoissonTaps");
const static Handle s_handleViewToLight(L"ViewToLight");
const static Handle s_handleShadowMap(L"InputShadowMap"); // \tbd Check name convention...
const static Handle s_handleShadowMapDiscRotation(L"ShadowMapDiscRotation");

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShadowProject", ShadowProject, ImagePassStep)

void ShadowProject::addRenderPassInputs(
	const ImageGraph* graph,
	const ImageGraphContext& context,
	RenderPass& pass) const
{
	addInputs(context, pass);
}

void ShadowProject::build(
	const ImageGraph* graph,
	const ImageGraphContext& context,
	const ImageGraphView& view,
	const targetSetVector_t& targetSetIds,
	const bufferVector_t& sbufferIds,
	const PassOutput& output,
	const RenderGraph& renderGraph,
	const ProgramParameters* sharedParams,
	RenderContext* renderContext,
	ScreenRenderer* screenRenderer) const
{
	auto shadowMap = context.findTexture(renderGraph, s_handleShadowMap);
	if (!shadowMap)
		return;

	const float shadowMapBias = view.shadowMapBias / view.shadowFarZ;
	const float shadowFadeZ = view.shadowFarZ * 0.7f;
	const float shadowFadeRate = 1.0f / (view.shadowFarZ - shadowFadeZ);

	const Vector4 shadowMapSizeAndBias(
		1.0f / (float)shadowMap->getSize().x,
		shadowMapBias / 1.0f,
		shadowFadeZ,
		shadowFadeRate);

	const Scalar viewEdgeNorm = view.viewFrustum.getFarZ() / Scalar(view.shadowFarZ);
	const Vector4 viewEdgeTopLeft = view.viewFrustum.corners[4] / viewEdgeNorm;
	const Vector4 viewEdgeTopRight = view.viewFrustum.corners[5] / viewEdgeNorm;
	const Vector4 viewEdgeBottomLeft = view.viewFrustum.corners[7] / viewEdgeNorm;
	const Vector4 viewEdgeBottomRight = view.viewFrustum.corners[6] / viewEdgeNorm;

	const Scalar p11 = view.projection.get(0, 0);
	const Scalar p22 = view.projection.get(1, 1);

	// Setup parameters for the shader.
	auto pp = renderContext->alloc< ProgramParameters >();
	pp->beginParameters(renderContext);
	pp->attachParameters(sharedParams);

	pp->setVectorParameter(s_handleShadowMapSizeAndBias, shadowMapSizeAndBias);
	pp->setVectorParameter(s_handleShadowMapUvTransform, view.shadowMapUvTransform);
	pp->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, view.sliceNearZ - c_sliceBias, view.sliceFarZ + c_sliceBias));
	pp->setVectorParameter(s_handleViewEdgeTopLeft, viewEdgeTopLeft);
	pp->setVectorParameter(s_handleViewEdgeTopRight, viewEdgeTopRight);
	pp->setVectorParameter(s_handleViewEdgeBottomLeft, viewEdgeBottomLeft);
	pp->setVectorParameter(s_handleViewEdgeBottomRight, viewEdgeBottomRight);
	pp->setVectorArrayParameter(s_handleShadowMapPoissonTaps, c_poissonTaps, sizeof_array(c_poissonTaps));
	pp->setMatrixParameter(s_handleViewToLight, view.viewToLight);
	pp->setTextureParameter(s_handleShadowMapDiscRotation, m_shadowMapDiscRotation[view.frame & 1]);

	bindSources(context, renderGraph, pp);

	pp->endParameters(renderContext);

	// Draw fullscreen quad with shader.
	Shader::Permutation permutation;
	context.applyTechniqueFlags(m_shader, permutation);
	m_shader->setCombination(s_handleLastSlice, (bool)(view.sliceIndex >= (view.sliceCount - 1)), permutation);
	screenRenderer->draw(renderContext, m_shader, permutation, pp);
}

}
