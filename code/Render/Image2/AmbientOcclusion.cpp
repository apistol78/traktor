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
const static Handle s_handleViewLast(L"ViewLast");
const static Handle s_handleViewInverse(L"ViewInverse");
const static Handle s_handleDeltaView(L"DeltaView");
const static Handle s_handleDeltaViewProj(L"DeltaViewProj");
const static Handle s_handleMagicCoeffs(L"MagicCoeffs");
const static Handle s_handleRandom(L"Random");
const static Handle s_handleOffsets(L"Offsets");
const static Handle s_handleDirections(L"Directions");
const static Handle s_handleRandomNormals(L"RandomNormals");
const static Handle s_handleRandomRotations(L"RandomRotations");

Random s_random;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.AmbientOcclusion", AmbientOcclusion, ImageStep)

void AmbientOcclusion::setup(const ImageGraph* /*imageGraph*/, const ImageGraphContext& cx, RenderPass& pass) const
{
	for (const auto& source : m_sources)
	{
		auto targetSetId = cx.findTextureTargetSetId(source.textureId);
		if (targetSetId != 0)
			pass.addInput(targetSetId);
	}
}

void AmbientOcclusion::build(
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
	pp->setVectorParameter(s_handleViewEdgeTopLeft, viewEdgeTopLeft);
	pp->setVectorParameter(s_handleViewEdgeTopRight, viewEdgeTopRight);
	pp->setVectorParameter(s_handleViewEdgeBottomLeft, viewEdgeBottomLeft);
	pp->setVectorParameter(s_handleViewEdgeBottomRight, viewEdgeBottomRight);
	pp->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
	pp->setVectorParameter(s_handleRandom, Vector4(s_random.nextFloat(), s_random.nextFloat(), s_random.nextFloat(), s_random.nextFloat()));
	pp->setVectorArrayParameter(s_handleOffsets, m_offsets, sizeof_array(m_offsets));
	pp->setVectorArrayParameter(s_handleDirections, m_directions, sizeof_array(m_directions));
	pp->setMatrixParameter(s_handleProjection, params.projection);
	pp->setMatrixParameter(s_handleView, params.view);
	pp->setMatrixParameter(s_handleViewLast, params.lastView);
	pp->setMatrixParameter(s_handleViewInverse, params.view.inverse());
	pp->setMatrixParameter(s_handleDeltaView, deltaView);
	pp->setMatrixParameter(s_handleDeltaViewProj, params.projection * deltaView);
	pp->setTextureParameter(s_handleRandomNormals, m_randomNormals);
	pp->setTextureParameter(s_handleRandomRotations, m_randomRotations);

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