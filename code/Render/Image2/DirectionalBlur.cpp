#include "Core/Math/Random.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/Image2/DirectionalBlur.h"

namespace traktor
{
    namespace render
    {
		namespace
		{

const static Handle s_handleTime(L"Time");
const static Handle s_handleDeltaTime(L"DeltaTime");
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
const static Handle s_handleViewLast(L"ViewLast");
const static Handle s_handleViewInverse(L"ViewInverse");
const static Handle s_handleDeltaView(L"DeltaView");
const static Handle s_handleDeltaViewProj(L"DeltaViewProj");

Random s_random;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.DirectionalBlur", DirectionalBlur, ImageStep)

void DirectionalBlur::setup(const ImageGraph* /*imageGraph*/, const ImageGraphContext& cx, RenderPass& pass) const
{
	for (const auto& source : m_sources)
	{
		auto targetSetId = cx.findTextureTargetSetId(source.textureId);
		if (targetSetId.first != 0)
			pass.addInput(targetSetId.first, targetSetId.second);
	}
}

void DirectionalBlur::build(
	const ImageGraph* imageGraph,
	const ImageGraphContext& cx,
	const RenderGraph& renderGraph,
	const ProgramParameters* sharedParams,
	RenderContext* renderContext
) const
{
	const auto& params = cx.getParams();

	Scalar p11 = params.projection.get(0, 0);
	Scalar p22 = params.projection.get(1, 1);
	Vector4 viewEdgeTopLeft = params.viewFrustum.corners[4];
	Vector4 viewEdgeTopRight = params.viewFrustum.corners[5];
	Vector4 viewEdgeBottomLeft = params.viewFrustum.corners[7];
	Vector4 viewEdgeBottomRight = params.viewFrustum.corners[6];
	Matrix44 deltaView = params.lastView * params.view.inverse();

	// Setup parameters for the shader.
	auto pp = renderContext->alloc< ProgramParameters >();
	pp->beginParameters(renderContext);
	pp->attachParameters(sharedParams);

	pp->setFloatParameter(s_handleTime, params.time);
	pp->setFloatParameter(s_handleDeltaTime, params.deltaTime);
	pp->setFloatParameter(s_handleViewFar, params.viewFrustum.getFarZ());
	pp->setVectorParameter(s_handleViewEdgeTopLeft, viewEdgeTopLeft);
	pp->setVectorParameter(s_handleViewEdgeTopRight, viewEdgeTopRight);
	pp->setVectorParameter(s_handleViewEdgeBottomLeft, viewEdgeBottomLeft);
	pp->setVectorParameter(s_handleViewEdgeBottomRight, viewEdgeBottomRight);
	pp->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleDirection, m_direction * Scalar(0.5f));
	pp->setVectorParameter(s_handleNoiseOffset, Vector4(s_random.nextFloat(), s_random.nextFloat(), 0.0f, 0.0f));
	pp->setVectorArrayParameter(s_handleGaussianOffsetWeights, &m_gaussianOffsetWeights[0], (uint32_t)m_gaussianOffsetWeights.size());
	pp->setMatrixParameter(s_handleProjection, params.projection);
	pp->setMatrixParameter(s_handleView, params.view);
	pp->setMatrixParameter(s_handleViewLast, params.lastView);
	pp->setMatrixParameter(s_handleViewInverse, params.view.inverse());
	pp->setMatrixParameter(s_handleDeltaView, deltaView);
	pp->setMatrixParameter(s_handleDeltaViewProj, params.projection * deltaView);

	for (const auto& source : m_sources)
	{
		auto texture = cx.findTexture(renderGraph, source.textureId);
		pp->setTextureParameter(source.parameter, texture);
	}

	pp->endParameters(renderContext);

	// Draw fullscreen quad with shader.
	cx.getScreenRenderer()->draw(renderContext, m_shader, Shader::Permutation(), pp);
}

    }
}