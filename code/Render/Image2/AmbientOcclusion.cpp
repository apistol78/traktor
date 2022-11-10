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
#include "Render/Image2/AmbientOcclusion.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"

namespace traktor
{
    namespace render
    {
		namespace
		{

const static Handle s_handleTime(L"Time");
const static Handle s_handleDeltaTime(L"DeltaTime");
const static Handle s_handleViewEdgeTopLeft(L"ViewEdgeTopLeft");
const static Handle s_handleViewEdgeTopRight(L"ViewEdgeTopRight");
const static Handle s_handleViewEdgeBottomLeft(L"ViewEdgeBottomLeft");
const static Handle s_handleViewEdgeBottomRight(L"ViewEdgeBottomRight");
const static Handle s_handleProjection(L"Projection");
const static Handle s_handleView(L"View");
const static Handle s_handleViewInverse(L"ViewInverse");
const static Handle s_handleMagicCoeffs(L"MagicCoeffs");
const static Handle s_handleRandom(L"Random");
const static Handle s_handleOffsets(L"Offsets");
const static Handle s_handleDirections(L"Directions");
const static Handle s_handleRandomNormals(L"RandomNormals");
const static Handle s_handleRandomRotations(L"RandomRotations");

Random s_random;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.AmbientOcclusion", AmbientOcclusion, ImagePassOp)

void AmbientOcclusion::setup(
	const ImageGraph* graph,
	const ImageGraphContext& context,
	RenderPass& pass
) const
{
	for (const auto& source : m_sources)
	{
		auto targetSetId = context.findTextureTargetSetId(source.textureId);
		if (targetSetId != 0)
			pass.addInput(targetSetId);
	}
}

void AmbientOcclusion::build(
	const ImageGraph* graph,
	const ImageGraphContext& context,
	const ImageGraphView& view,
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

	pp->setFloatParameter(s_handleTime, view.time);
	pp->setFloatParameter(s_handleDeltaTime, view.deltaTime);
	pp->setVectorParameter(s_handleViewEdgeTopLeft, viewEdgeTopLeft);
	pp->setVectorParameter(s_handleViewEdgeTopRight, viewEdgeTopRight);
	pp->setVectorParameter(s_handleViewEdgeBottomLeft, viewEdgeBottomLeft);
	pp->setVectorParameter(s_handleViewEdgeBottomRight, viewEdgeBottomRight);
	pp->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleRandom, Vector4(s_random.nextFloat(), s_random.nextFloat(), s_random.nextFloat(), s_random.nextFloat()));
	pp->setVectorArrayParameter(s_handleOffsets, m_offsets, sizeof_array(m_offsets));
	pp->setVectorArrayParameter(s_handleDirections, m_directions, sizeof_array(m_directions));
	pp->setMatrixParameter(s_handleProjection, view.projection);
	pp->setMatrixParameter(s_handleView, view.view);
	pp->setMatrixParameter(s_handleViewInverse, view.view.inverse());
	pp->setTextureParameter(s_handleRandomNormals, m_randomNormals);
	pp->setTextureParameter(s_handleRandomRotations, m_randomRotations);

	for (const auto& source : m_sources)
	{
		auto texture = context.findTexture(renderGraph, source.textureId);
		pp->setTextureParameter(source.parameter, texture);
	}

	pp->endParameters(renderContext);

	// Draw fullscreen quad with shader.
	screenRenderer->draw(renderContext, m_shader, Shader::Permutation(), pp);
}

    }
}