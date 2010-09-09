#include "Core/Math/Random.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/ISimpleTexture.h"
#include "Render/RenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessStepSmProj.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const Vector4 c_poissonTaps[] =
{
	Vector4(-0.326212f, -0.40581f, 0.519456f, 0.767022f),
	Vector4(-0.840144f, -0.07358f, 0.185461f, -0.893124f),
	Vector4(-0.695914f, 0.457137f, 0.507431f, 0.064425f),
	Vector4(-0.203345f, 0.620716f, 0.89642f, 0.412458f),
	Vector4(0.96234f, -0.194983f, -0.32194f, -0.932615f),
	Vector4(0.473434f, -0.480026f, -0.791559f, -0.59771f)
};

/*! \brief Random rotation texture.
 *
 * This texture is used to rotate the Poisson distribution
 * disc for each fragment in shadow mapping.
 */
Ref< render::ISimpleTexture > createRandomRotationTexture(render::IRenderSystem* renderSystem)
{
	static Random random;

	AutoArrayPtr< uint8_t > data(new uint8_t [128 * 128 * 4]);
	for (uint32_t y = 0; y < 128; ++y)
	{
		for (uint32_t x = 0; x < 128; ++x)
		{
			float angle = (random.nextFloat() * 2.0f - 1.0f) * PI;
			float c = cosf(angle) * 127.5f + 127.5f;
			float s = sinf(angle) * 127.5f + 127.5f;
			data[(x + y * 128) * 4 + 0] = uint8_t(c);
			data[(x + y * 128) * 4 + 1] = uint8_t(s);
			data[(x + y * 128) * 4 + 2] = uint8_t(c);
			data[(x + y * 128) * 4 + 3] = uint8_t(s);
		}
	}

	render::SimpleTextureCreateDesc desc;
	desc.width = 128;
	desc.height = 128;
	desc.mipCount = 1;
	desc.format = render::TfR8G8B8A8;
	desc.immutable = true;
	desc.initialData[0].data = data.ptr();
	desc.initialData[0].pitch = 128 * 4;
	desc.initialData[0].slicePitch = 0;

	return renderSystem->createSimpleTexture(desc);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepSmProj", 0, PostProcessStepSmProj, PostProcessStep)

Ref< PostProcessStep::Instance > PostProcessStepSmProj::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	if (!resourceManager->bind(m_shader))
		return 0;

	// Generate screen-space random rotation textures.
	Ref< render::ISimpleTexture > shadowMapDiscRotation[2];
	shadowMapDiscRotation[0] = createRandomRotationTexture(renderSystem);
	shadowMapDiscRotation[1] = createRandomRotationTexture(renderSystem);
	if (!shadowMapDiscRotation[0] || !shadowMapDiscRotation[1])
		return 0;

	return new InstanceSmProj(this, shadowMapDiscRotation);
}

bool PostProcessStepSmProj::serialize(ISerializer& s)
{
	return s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
}

// Instance

PostProcessStepSmProj::InstanceSmProj::InstanceSmProj(
	const PostProcessStepSmProj* step,
	Ref< render::ISimpleTexture > shadowMapDiscRotation[2]
)
:	m_step(step)
,	m_frame(0)
{
	m_shadowMapDiscRotation[0] = shadowMapDiscRotation[0];
	m_shadowMapDiscRotation[1] = shadowMapDiscRotation[1];
}

void PostProcessStepSmProj::InstanceSmProj::destroy()
{
}

void PostProcessStepSmProj::InstanceSmProj::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	resource::Proxy< render::Shader > shader = m_step->m_shader;
	if (!shader.validate())
		return;

	Ref< render::RenderTargetSet > sourceShMap = postProcess->getTargetRef(PostProcess::PdtSourceColor);
	Ref< render::RenderTargetSet > sourceDepth = postProcess->getTargetRef(PostProcess::PdtSourceDepth);
	if (!sourceShMap || !sourceDepth)
		return;

	postProcess->prepareShader(shader);

	float shadowMapBias = params.shadowMapBias / params.depthRange;

	Vector4 sourceDepthSize(
		float(sourceDepth->getWidth()),
		float(sourceDepth->getHeight()),
		0.0f,
		0.0f
	);
	Vector4 shadowMapSizeAndBias(
		1.0f / float(sourceShMap->getWidth()),
		shadowMapBias,
		0.0f,
		0.0f
	);
	
	Scalar viewEdgeNorm = params.viewFrustum.getFarZ() / Scalar(params.depthRange);
	Vector4 viewEdgeTopLeft = params.viewFrustum.corners[4] / viewEdgeNorm;
	Vector4 viewEdgeTopRight = params.viewFrustum.corners[5] / viewEdgeNorm;
	Vector4 viewEdgeBottomLeft = params.viewFrustum.corners[7] / viewEdgeNorm;
	Vector4 viewEdgeBottomRight = params.viewFrustum.corners[6] / viewEdgeNorm;

	Scalar p11 = params.projection.get(0, 0);
	Scalar p22 = params.projection.get(1, 1);

	shader->setTextureParameter(L"ShadowMap", sourceShMap->getColorTexture(0));
	shader->setTextureParameter(L"ShadowMapDiscRotation", m_shadowMapDiscRotation[m_frame & 1]);
	shader->setVectorParameter(L"ShadowMapSizeAndBias", shadowMapSizeAndBias);
	shader->setVectorArrayParameter(L"ShadowMapPoissonTaps", c_poissonTaps, sizeof_array(c_poissonTaps));
	shader->setFloatParameter(L"ShadowFarZ", params.shadowFarZ);
	shader->setTextureParameter(L"Depth", sourceDepth->getColorTexture(0));
	shader->setFloatParameter(L"DepthRange", params.depthRange);
	shader->setVectorParameter(L"Depth_Size", sourceDepthSize);	
	shader->setVectorParameter(L"MagicCoeffs", Vector4(1.0f / p11, 1.0f / p22, 0.0f, 0.0f));
	shader->setVectorParameter(L"ViewEdgeTopLeft", viewEdgeTopLeft);
	shader->setVectorParameter(L"ViewEdgeTopRight", viewEdgeTopRight);
	shader->setVectorParameter(L"ViewEdgeBottomLeft", viewEdgeBottomLeft);
	shader->setVectorParameter(L"ViewEdgeBottomRight", viewEdgeBottomRight);
	shader->setMatrixParameter(L"ViewToLight", params.viewToLight);

	const float maskClear[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	renderView->clear(render::CfColor, maskClear, 1.0f, 0);

	screenRenderer->draw(renderView, shader);

	++m_frame;
}

	}
}
