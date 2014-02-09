#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/IRenderSystem.h"
#include "Render/RenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/PostProcess/PostProcessStepGrain.h"
#include "World/PostProcess/PostProcess.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepGrain", 0, PostProcessStepGrain, PostProcessStep)

Ref< PostProcessStep::Instance > PostProcessStepGrain::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return 0;

	std::vector< InstanceGrain::Source > sources(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		sources[i].param = render::getParameterHandle(m_sources[i].param);
		sources[i].source = render::getParameterHandle(m_sources[i].source);
		sources[i].index = m_sources[i].index;
	}

	return new InstanceGrain(this, shader, sources);
}

void PostProcessStepGrain::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
}

PostProcessStepGrain::Source::Source()
:	index(0)
{
}

void PostProcessStepGrain::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< std::wstring >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
}

// Instance

PostProcessStepGrain::InstanceGrain::InstanceGrain(const PostProcessStepGrain* step, const resource::Proxy< render::Shader >& shader, const std::vector< Source >& sources)
:	m_step(step)
,	m_shader(shader)
,	m_sources(sources)
,	m_time(0.0f)
{
	m_handleTime = render::getParameterHandle(L"Time");
	m_handleDeltaTime = render::getParameterHandle(L"DeltaTime");
	m_handleNoiseOffset = render::getParameterHandle(L"NoiseOffset");
}

void PostProcessStepGrain::InstanceGrain::destroy()
{
}

void PostProcessStepGrain::InstanceGrain::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	postProcess->prepareShader(m_shader);

	m_shader->setFloatParameter(m_handleTime, m_time);
	m_shader->setFloatParameter(m_handleDeltaTime, params.deltaTime);
	m_shader->setVectorParameter(m_handleNoiseOffset, Vector4(
		m_random.nextFloat(),
		m_random.nextFloat(),
		0.0f,
		0.0f
	));

	for (std::vector< Source >::const_iterator i = m_sources.begin(); i != m_sources.end(); ++i)
	{
		render::RenderTargetSet* source = postProcess->getTarget(i->source);
		if (source)
			m_shader->setTextureParameter(i->param, source->getColorTexture(i->index));
	}

	screenRenderer->draw(renderView, m_shader);

	m_time += params.deltaTime;
}

	}
}
