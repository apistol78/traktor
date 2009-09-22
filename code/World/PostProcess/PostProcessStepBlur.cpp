#include "World/PostProcess/PostProcessStepBlur.h"
#include "World/PostProcess/PostProcess.h"
#include "World/WorldRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/ShaderGraph.h"
#include "Render/RenderTargetSet.h"
#include "Core/Math/Const.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.PostProcessStepBlur", PostProcessStepBlur, PostProcessStep)

bool PostProcessStepBlur::create(PostProcess* postProcess, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem)
{
	if (!resourceManager->bind(m_shader))
		return false;

	float totalWeight = 0.0f;
	for (int i = 0; i < sizeof_array(m_gaussianOffsetWeights); ++i)
	{
		float weight = std::sin(i * PI / 14.0f);

		totalWeight += weight;

		m_gaussianOffsetWeights[i].set(
			i - sizeof_array(m_gaussianOffsetWeights) / 2.0f,
			weight,
			0.0f,
			0.0f
		);
	}

	Vector4 invWeight(1.0f, 1.0f / totalWeight, 1.0f, 1.0f);
	for (int i = 0; i < sizeof_array(m_gaussianOffsetWeights); ++i)
		m_gaussianOffsetWeights[i] *= invWeight;

	return true;
}

void PostProcessStepBlur::destroy(PostProcess* postProcess)
{
}

void PostProcessStepBlur::render(
	PostProcess* postProcess,
	const WorldRenderView& worldRenderView,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	float deltaTime
)
{
	if (!m_shader.validate())
		return;

	postProcess->prepareShader(m_shader);

	for (std::vector< Source >::iterator i = m_sources.begin(); i != m_sources.end(); ++i)
	{
		Ref< render::RenderTargetSet > source = postProcess->getTargetRef(i->source);
		if (source)
		{
			m_shader->setSamplerTexture(i->param, source->getColorTexture(i->index));
			m_shader->setVectorParameter(i->param + L"_Size", Vector4(
				float(source->getWidth()),
				float(source->getHeight()),
				0.0f,
				0.0f
			));
		}
	}

	const Frustum& viewFrustum = worldRenderView.getViewFrustum();

	m_shader->setVectorArrayParameter(L"GaussianOffsetWeights", m_gaussianOffsetWeights, sizeof_array(m_gaussianOffsetWeights));
	m_shader->setVectorParameter(L"Direction", m_direction);
	m_shader->setFloatParameter(L"ViewFar", viewFrustum.getFarZ());

	screenRenderer->draw(renderView, m_shader);
}

bool PostProcessStepBlur::serialize(Serializer& s)
{
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
	s >> Member< Vector4 >(L"direction", m_direction);
	return true;
}

PostProcessStepBlur::Source::Source()
:	source(0)
,	index(0)
{
}

bool PostProcessStepBlur::Source::serialize(Serializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< int32_t >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
	return true;
}

	}
}
