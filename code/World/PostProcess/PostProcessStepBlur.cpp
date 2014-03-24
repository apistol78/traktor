#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Random.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/RenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/WorldRenderView.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessStepBlur.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

Random s_random;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepBlur", 2, PostProcessStepBlur, PostProcessStep)

PostProcessStepBlur::PostProcessStepBlur()
:	m_direction(1.0f, 0.0f, 0.0f, 0.0f)
,	m_taps(15)
,	m_blurType(BtGaussian)
{
}

Ref< PostProcessStepBlur::Instance > PostProcessStepBlur::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return 0;

	std::vector< InstanceBlur::Source > sources(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		sources[i].param = render::getParameterHandle(m_sources[i].param);
		sources[i].source = render::getParameterHandle(m_sources[i].source);
		sources[i].index = m_sources[i].index;
	}

	AlignedVector< Vector4 > gaussianOffsetWeights(m_taps);
	float totalWeight = 0.0f;

	if (m_blurType == BtGaussian)
	{
		float sigma = m_taps / 4.73f;

		// Iterate to prevent under or over sigma.
		for (int32_t i = 0; i < 10; ++i)
		{
			const float x = m_taps / 2.0f;
			float a = 1.0f / sqrtf(TWO_PI * sigma * sigma);
			float weight = a * std::exp(-((x * x) / (2.0f * sigma * sigma)));
			if (weight > 0.01f)
				sigma -= 0.1f;
			else if (weight < 0.001f)
				sigma += 0.01f;
			else
				break;
		}

		const float a = 1.0f / sqrtf(TWO_PI * sigma * sigma);
		for (int32_t i = 0; i < m_taps; ++i)
		{
			float x = i - m_taps / 2.0f;
			
			float weight = a * std::exp(-((x * x) / (2.0f * sigma * sigma)));
			totalWeight += weight;

			gaussianOffsetWeights[i].set(
				i - m_taps / 2.0f,
				weight,
				0.0f,
				0.0f
			);
		}
	}
	else if (m_blurType == BtSine)
	{
		float step = 1.0f / (m_taps - 1.0f);
		float angleMin = (PI * step) / 2.0f;
		float angleMax = PI - angleMin;

		for (int i = 0; i < m_taps; ++i)
		{
			float phi = (float(i) * step) * (angleMax - angleMin) + angleMin;

			float weight = sinf(phi);
			totalWeight += weight;

			gaussianOffsetWeights[i].set(
				i - m_taps / 2.0f,
				weight,
				0.0f,
				0.0f
			);
		}
	}
	else if (m_blurType == BtBox)
	{
		for (int i = 0; i < m_taps; ++i)
		{
			gaussianOffsetWeights[i].set(
				i - m_taps / 2.0f,
				1.0f,
				0.0f,
				0.0f
			);
		}
		totalWeight = float(m_taps);
	}
	else if (m_blurType == BtBox2D)
	{
		for (int i = 0; i < m_taps; ++i)
		{
			float x = s_random.nextFloat() * 2.0f - 1.0f;
			float y = s_random.nextFloat() * 2.0f - 1.0f;
			gaussianOffsetWeights[i].set(
				x,
				y,
				0.0f,
				0.0f
			);
		}
	}
	else if (m_blurType == BtCircle2D)
	{
		for (int i = 0; i < m_taps; )
		{
			float x = s_random.nextFloat() * 2.0f - 1.0f;
			float y = s_random.nextFloat() * 2.0f - 1.0f;
			if (std::sqrt(x * x + y * y) <= 1.0f)
			{
				gaussianOffsetWeights[i].set(
					x,
					y,
					0.0f,
					0.0f
				);
				++i;
			}
		}
	}

	if (totalWeight > FUZZY_EPSILON)
	{
		Vector4 invWeight(1.0f, 1.0f / totalWeight, 1.0f, 1.0f);
		for (int i = 0; i < m_taps; ++i)
			gaussianOffsetWeights[i] *= invWeight;
	}

	return new InstanceBlur(
		shader,
		sources,
		m_direction,
		gaussianOffsetWeights
	);
}

void PostProcessStepBlur::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
	s >> Member< Vector4 >(L"direction", m_direction, AttributeDirection());
	if (s.getVersion() >= 1)
		s >> Member< int32_t >(L"taps", m_taps);
	if (s.getVersion() >= 2)
	{
		const MemberEnum< BlurType >::Key c_BlurType_Keys[] =
		{
			{ L"BtGaussian", BtGaussian },
			{ L"BtSine", BtSine },
			{ L"BtBox", BtBox },
			{ L"BtBox2D", BtBox2D },
			{ L"BtCircle2D", BtCircle2D },
			{ 0 }
		};
		s >> MemberEnum< BlurType >(L"blurType", m_blurType, c_BlurType_Keys);
	}
}

PostProcessStepBlur::Source::Source()
:	index(0)
{
}

void PostProcessStepBlur::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< std::wstring >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
}

// Instance

PostProcessStepBlur::InstanceBlur::InstanceBlur(
	const resource::Proxy< render::Shader >& shader,
	const std::vector< Source >& sources,
	const Vector4& direction,
	const AlignedVector< Vector4 >& gaussianOffsetWeights
)
:	m_shader(shader)
,	m_sources(sources)
,	m_direction(direction)
,	m_gaussianOffsetWeights(gaussianOffsetWeights)
{
	m_handleGaussianOffsetWeights = render::getParameterHandle(L"GaussianOffsetWeights");
	m_handleDirection = render::getParameterHandle(L"Direction");
	m_handleViewFar = render::getParameterHandle(L"ViewFar");
	m_handleNoiseOffset = render::getParameterHandle(L"NoiseOffset");
}

void PostProcessStepBlur::InstanceBlur::destroy()
{
}

void PostProcessStepBlur::InstanceBlur::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	postProcess->prepareShader(m_shader);

	for (std::vector< Source >::const_iterator i = m_sources.begin(); i != m_sources.end(); ++i)
	{
		render::RenderTargetSet* source = postProcess->getTarget(i->source);
		if (source)
			m_shader->setTextureParameter(i->param, source->getColorTexture(i->index));
	}

	m_shader->setVectorArrayParameter(m_handleGaussianOffsetWeights, &m_gaussianOffsetWeights[0], uint32_t(m_gaussianOffsetWeights.size()));
	m_shader->setVectorParameter(m_handleDirection, m_direction * Scalar(0.5f));
	m_shader->setFloatParameter(m_handleViewFar, params.viewFrustum.getFarZ());
	m_shader->setVectorParameter(m_handleNoiseOffset, Vector4(
		s_random.nextFloat(),
		s_random.nextFloat(),
		0.0f,
		0.0f
	));

	screenRenderer->draw(renderView, m_shader);
}

	}
}
