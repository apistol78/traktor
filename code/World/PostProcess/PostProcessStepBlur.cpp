#include "World/PostProcess/PostProcessStepBlur.h"
#include "World/PostProcess/PostProcess.h"
#include "World/WorldRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/ShaderGraph.h"
#include "Render/RenderTargetSet.h"
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepBlur", PostProcessStepBlur, PostProcessStep)

Ref< PostProcessStepBlur::Instance > PostProcessStepBlur::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	if (!resourceManager->bind(m_shader))
		return false;

	Vector4 gaussianOffsetWeights[15];
	float totalWeight = 0.0f;

	float step = 1.0f / (sizeof_array(gaussianOffsetWeights) - 1.0f);
	float angleMin = (PI * step) / 2.0f;
	float angleMax = PI - angleMin;

	for (int i = 0; i < sizeof_array(gaussianOffsetWeights); ++i)
	{
		float phi = (float(i) * step) * (angleMax - angleMin) + angleMin;

		float weight = sinf(phi);
		totalWeight += weight;

		gaussianOffsetWeights[i].set(
			i - sizeof_array(gaussianOffsetWeights) / 2.0f,
			weight,
			0.0f,
			0.0f
		);
	}

	Vector4 invWeight(1.0f, 1.0f / totalWeight, 1.0f, 1.0f);
	for (int i = 0; i < sizeof_array(gaussianOffsetWeights); ++i)
		gaussianOffsetWeights[i] *= invWeight;

	return new InstanceBlur(
		this,
		gaussianOffsetWeights
	);
}

bool PostProcessStepBlur::serialize(ISerializer& s)
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

bool PostProcessStepBlur::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< int32_t >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
	return true;
}

// Instance

PostProcessStepBlur::InstanceBlur::InstanceBlur(
	const PostProcessStepBlur* step,
	const Vector4 gaussianOffsetWeights[15]
)
:	m_step(step)
{
	for (int i = 0; i < sizeof_array(m_gaussianOffsetWeights); ++i)
		m_gaussianOffsetWeights[i] = gaussianOffsetWeights[i];
}

void PostProcessStepBlur::InstanceBlur::destroy()
{
}

void PostProcessStepBlur::InstanceBlur::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const Frustum& viewFrustum,
	const Matrix44& projection,
	float shadowMapBias,
	float deltaTime
)
{
	resource::Proxy< render::Shader > shader = m_step->m_shader;
	if (!shader.validate())
		return;

	postProcess->prepareShader(shader);

	const std::vector< Source >& sources = m_step->m_sources;
	for (std::vector< Source >::const_iterator i = sources.begin(); i != sources.end(); ++i)
	{
		Ref< render::RenderTargetSet > source = postProcess->getTargetRef(i->source);
		if (source)
		{
			shader->setSamplerTexture(i->param, source->getColorTexture(i->index));
			shader->setVectorParameter(i->param + L"_Size", Vector4(
				float(source->getWidth()),
				float(source->getHeight()),
				0.0f,
				0.0f
			));
		}
	}

	shader->setVectorArrayParameter(L"GaussianOffsetWeights", m_gaussianOffsetWeights, sizeof_array(m_gaussianOffsetWeights));
	shader->setVectorParameter(L"Direction", m_step->m_direction);
	shader->setFloatParameter(L"ViewFar", viewFrustum.getFarZ());

	screenRenderer->draw(renderView, shader);
}

	}
}
