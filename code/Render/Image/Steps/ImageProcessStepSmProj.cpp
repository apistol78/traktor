#include "Core/Math/Random.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ISimpleTexture.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Render/Image/ImageProcess.h"
#include "Render/Image/Steps/ImageProcessStepSmProj.h"

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

/*! Random rotation texture.
 *
 * This texture is used to rotate the Poisson distribution
 * disc for each fragment in shadow mapping.
 */
Ref< ISimpleTexture > createRandomRotationTexture(IRenderSystem* renderSystem)
{
	static Random random;

	AutoArrayPtr< uint8_t > data(new uint8_t [128 * 128 * 4]);
	for (uint32_t y = 0; y < 128; ++y)
	{
		for (uint32_t x = 0; x < 128; ++x)
		{
			float angle = (random.nextFloat() * 2.0f - 1.0f) * PI;
			float xa =  cosf(angle) * 127.5f + 127.5f;
			float xb =  sinf(angle) * 127.5f + 127.5f;
			float ya =  sinf(angle) * 127.5f + 127.5f;
			float yb = -cosf(angle) * 127.5f + 127.5f;
			data[(x + y * 128) * 4 + 0] = uint8_t(xa);
			data[(x + y * 128) * 4 + 1] = uint8_t(xb);
			data[(x + y * 128) * 4 + 2] = uint8_t(ya);
			data[(x + y * 128) * 4 + 3] = uint8_t(yb);
		}
	}

	SimpleTextureCreateDesc desc;
	desc.width = 128;
	desc.height = 128;
	desc.mipCount = 1;
	desc.format = TfR8G8B8A8;
	desc.immutable = true;
	desc.initialData[0].data = data.ptr();
	desc.initialData[0].pitch = 128 * 4;
	desc.initialData[0].slicePitch = 0;

	return renderSystem->createSimpleTexture(desc, T_FILE_LINE_W);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepSmProj", 0, ImageProcessStepSmProj, ImageProcessStep)

Ref< ImageProcessStep::Instance > ImageProcessStepSmProj::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	resource::Proxy< Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return nullptr;

	// Generate screen-space random rotation textures.
	Ref< ISimpleTexture > shadowMapDiscRotation[2];
	shadowMapDiscRotation[0] = createRandomRotationTexture(renderSystem);
	shadowMapDiscRotation[1] = createRandomRotationTexture(renderSystem);
	if (!shadowMapDiscRotation[0] || !shadowMapDiscRotation[1])
		return nullptr;

	return new InstanceSmProj(this, shadowMapDiscRotation, shader);
}

void ImageProcessStepSmProj::serialize(ISerializer& s)
{
	s >> resource::Member< Shader >(L"shader", m_shader);
}

// Instance

ImageProcessStepSmProj::InstanceSmProj::InstanceSmProj(
	const ImageProcessStepSmProj* step,
	Ref< ISimpleTexture > shadowMapDiscRotation[2],
	const resource::Proxy< Shader >& shader
)
:	m_step(step)
,	m_shader(shader)
,	m_frame(0)
{
	m_shadowMapDiscRotation[0] = shadowMapDiscRotation[0];
	m_shadowMapDiscRotation[1] = shadowMapDiscRotation[1];

	m_handleInputColor = getParameterHandle(L"InputColor");
	m_handleInputDepth = getParameterHandle(L"InputDepth");

	m_handleShadowMap = getParameterHandle(L"ShadowMap");
	m_handleShadowMapDiscRotation = getParameterHandle(L"ShadowMapDiscRotation");
	m_handleShadowMapSizeAndBias = getParameterHandle(L"ShadowMapSizeAndBias");
	m_handleShadowMapPoissonTaps = getParameterHandle(L"ShadowMapPoissonTaps");
	m_handleDepth = getParameterHandle(L"Depth");
	m_handleMagicCoeffs = getParameterHandle(L"MagicCoeffs");
	m_handleViewEdgeTopLeft = getParameterHandle(L"ViewEdgeTopLeft");
	m_handleViewEdgeTopRight = getParameterHandle(L"ViewEdgeTopRight");
	m_handleViewEdgeBottomLeft = getParameterHandle(L"ViewEdgeBottomLeft");
	m_handleViewEdgeBottomRight = getParameterHandle(L"ViewEdgeBottomRight");
	m_handleViewToLight = getParameterHandle(L"ViewToLight");
	m_handleLastSlice = getParameterHandle(L"LastSlice");
}

void ImageProcessStepSmProj::InstanceSmProj::destroy()
{
}

void ImageProcessStepSmProj::InstanceSmProj::render(
	ImageProcess* imageProcess,
	RenderContext* renderContext,
	ProgramParameters* sharedParams,
	const RenderParams& params
)
{
	ISimpleTexture* sourceShMap = imageProcess->getTarget(m_handleInputColor);
	ISimpleTexture* sourceDepth = imageProcess->getTarget(m_handleInputDepth);
	if (!sourceShMap || !sourceDepth)
		return;

	float shadowMapBias = params.shadowMapBias / params.shadowFarZ;
	float shadowFadeZ = params.shadowFarZ * 0.7f;
	float shadowFadeRate = 1.0f / (params.shadowFarZ - shadowFadeZ);

	Vector4 shadowMapSizeAndBias(
		1.0f / float(sourceShMap->getWidth()),
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

	m_shader->setCombination(m_handleLastSlice, (bool)(params.sliceIndex >= (params.sliceCount - 1)));

	auto pp = renderContext->alloc< ProgramParameters >();
	pp->beginParameters(renderContext);
	pp->attachParameters(sharedParams);
	pp->setTextureParameter(m_handleShadowMap, sourceShMap);
	pp->setTextureParameter(m_handleShadowMapDiscRotation, m_shadowMapDiscRotation[m_frame & 1]);
	pp->setVectorParameter(m_handleShadowMapSizeAndBias, shadowMapSizeAndBias);
	pp->setVectorArrayParameter(m_handleShadowMapPoissonTaps, c_poissonTaps, sizeof_array(c_poissonTaps));
	pp->setTextureParameter(m_handleDepth, sourceDepth);
	pp->setVectorParameter(m_handleMagicCoeffs, Vector4(1.0f / p11, 1.0f / p22, params.sliceNearZ - c_sliceBias, params.sliceFarZ + c_sliceBias));
	pp->setVectorParameter(m_handleViewEdgeTopLeft, viewEdgeTopLeft);
	pp->setVectorParameter(m_handleViewEdgeTopRight, viewEdgeTopRight);
	pp->setVectorParameter(m_handleViewEdgeBottomLeft, viewEdgeBottomLeft);
	pp->setVectorParameter(m_handleViewEdgeBottomRight, viewEdgeBottomRight);
	pp->setMatrixParameter(m_handleViewToLight, params.viewToLight);
	pp->endParameters(renderContext);

	imageProcess->getScreenRenderer()->draw(renderContext, m_shader, pp);

	++m_frame;
}

	}
}
