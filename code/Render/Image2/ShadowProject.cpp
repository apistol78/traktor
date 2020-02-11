#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Image2/ImageGraph.h"
#include "Render/Image2/ImageGraphContext.h"
#include "Render/Image2/ShadowProject.h"

namespace traktor
{
    namespace render
    {
		namespace
		{

const float c_sliceBias = 0.0001f;	//!< Extra slice dimension bias; slight overlap over slices.

const Vector4 c_poissonTaps[] =
{
	Vector4(-0.326212f, -0.40581f, 0.519456f, 0.767022f),
	Vector4(-0.840144f, -0.07358f, 0.185461f, -0.893124f),
	Vector4(-0.695914f, 0.457137f, 0.507431f, 0.064425f),
	Vector4(-0.203345f, 0.620716f, 0.89642f, 0.412458f),
	Vector4(0.96234f, -0.194983f, -0.32194f, -0.932615f),
	Vector4(0.473434f, -0.480026f, -0.791559f, -0.59771f)
};

const static Handle s_handleLastSlice(L"LastSlice");
const static Handle s_handleTime(L"Time");
const static Handle s_handleDeltaTime(L"DeltaTime");
const static Handle s_handleShadowMapSizeAndBias(L"ShadowMapSizeAndBias");
const static Handle s_handleShadowMapUvTransform(L"ShadowMapUvTransform");
const static Handle s_handleMagicCoeffs(L"MagicCoeffs");
const static Handle s_handleViewEdgeTopLeft(L"ViewEdgeTopLeft");
const static Handle s_handleViewEdgeTopRight(L"ViewEdgeTopRight");
const static Handle s_handleViewEdgeBottomLeft(L"ViewEdgeBottomLeft");
const static Handle s_handleViewEdgeBottomRight(L"ViewEdgeBottomRight");
const static Handle s_handleShadowMapPoissonTaps(L"ShadowMapPoissonTaps");
const static Handle s_handleViewToLight(L"ViewToLight");
const static Handle s_handleShadowMap(L"InputShadowMap");	// \tbd Check name convention...
const static Handle s_handleShadowMapDiscRotation(L"ShadowMapDiscRotation");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ShadowProject", ShadowProject, IImageStep)

void ShadowProject::setup(const ImageGraph* /*imageGraph*/, const ImageGraphContext& cx, RenderPass& pass) const
{
	for (const auto& source : m_sources)
	{
		auto targetSetId = cx.findTextureTargetSetId(source.textureId);
		if (targetSetId != 0)
			pass.addInput(targetSetId);
	}
}

void ShadowProject::build(
	const ImageGraph* imageGraph,
	const ImageGraphContext& cx,
	const RenderGraph& renderGraph,
	const ProgramParameters* sharedParams,
	RenderContext* renderContext
) const
{
	const auto& params = cx.getParams();

	// \tbd Assuming ISimpleTexture
	auto shadowMap = (ISimpleTexture*)cx.findTexture(renderGraph, s_handleShadowMap);
	if (!shadowMap)
		return;

	float shadowMapBias = params.shadowMapBias / params.shadowFarZ;
	float shadowFadeZ = params.shadowFarZ * 0.7f;
	float shadowFadeRate = 1.0f / (params.shadowFarZ - shadowFadeZ);

	Vector4 shadowMapSizeAndBias(
		1.0f / (float)shadowMap->getWidth(),
		shadowMapBias / 1.0f,
		shadowFadeZ,
		shadowFadeRate
	);

	Scalar viewEdgeNorm = params.viewFrustum.getFarZ() / Scalar(params.shadowFarZ);
	Vector4 viewEdgeTopLeft = params.viewFrustum.corners[4] / viewEdgeNorm;
	Vector4 viewEdgeTopRight = params.viewFrustum.corners[5] / viewEdgeNorm;
	Vector4 viewEdgeBottomLeft = params.viewFrustum.corners[7] / viewEdgeNorm;
	Vector4 viewEdgeBottomRight = params.viewFrustum.corners[6] / viewEdgeNorm;

	Scalar p11 = params.projection.get(0, 0);
	Scalar p22 = params.projection.get(1, 1);

	// \tbd Need to refactor "setCombination" -> "getCurrentProgram"
	m_shader->setCombination(s_handleLastSlice, (bool)(params.sliceIndex >= (params.sliceCount - 1)));

	// Setup parameters for the shader.
	auto pp = renderContext->alloc< ProgramParameters >();
	pp->beginParameters(renderContext);
	pp->attachParameters(sharedParams);	

	pp->setFloatParameter(s_handleTime, params.time);
	pp->setFloatParameter(s_handleDeltaTime, params.deltaTime);
	pp->setVectorParameter(s_handleShadowMapSizeAndBias, shadowMapSizeAndBias);
	pp->setVectorParameter(s_handleShadowMapUvTransform, params.shadowMapUvTransform);
	pp->setVectorParameter(s_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, params.sliceNearZ - c_sliceBias, params.sliceFarZ + c_sliceBias));
	pp->setVectorParameter(s_handleViewEdgeTopLeft, viewEdgeTopLeft);
	pp->setVectorParameter(s_handleViewEdgeTopRight, viewEdgeTopRight);
	pp->setVectorParameter(s_handleViewEdgeBottomLeft, viewEdgeBottomLeft);
	pp->setVectorParameter(s_handleViewEdgeBottomRight, viewEdgeBottomRight);
	pp->setVectorArrayParameter(s_handleShadowMapPoissonTaps, c_poissonTaps, sizeof_array(c_poissonTaps));
	pp->setMatrixParameter(s_handleViewToLight, params.viewToLight);
	pp->setTextureParameter(s_handleShadowMapDiscRotation, m_shadowMapDiscRotation[params.frame & 1]);

	for (const auto& source : m_sources)
	{
		auto texture = cx.findTexture(renderGraph, source.textureId);
		pp->setTextureParameter(source.parameter, texture);
	}

	pp->endParameters(renderContext);

	// Draw fullscreen quad with shader.
	cx.getScreenRenderer()->draw(renderContext, m_shader, pp);
}

    }
}