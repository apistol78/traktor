#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/RenderTargetSet.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "World/PostProcess/PostProcessStepSimple.h"
#include "World/PostProcess/PostProcess.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepSimple", 0, PostProcessStepSimple, PostProcessStep)

Ref< PostProcessStep::Instance > PostProcessStepSimple::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	resource::Proxy< render::Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return 0;

	std::vector< InstanceSimple::Source > sources(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		sources[i].param = render::getParameterHandle(m_sources[i].param);
		sources[i].source = render::getParameterHandle(m_sources[i].source);
		sources[i].index = m_sources[i].index;
	}

	return new InstanceSimple(this, shader, sources);
}

void PostProcessStepSimple::serialize(ISerializer& s)
{
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
}

PostProcessStepSimple::Source::Source()
:	index(0)
{
}

void PostProcessStepSimple::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< std::wstring >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
}

// Instance

PostProcessStepSimple::InstanceSimple::InstanceSimple(const PostProcessStepSimple* step, const resource::Proxy< render::Shader >& shader, const std::vector< Source >& sources)
:	m_step(step)
,	m_shader(shader)
,	m_sources(sources)
,	m_time(0.0f)
{
	m_handleTime = render::getParameterHandle(L"Time");
	m_handleDeltaTime = render::getParameterHandle(L"DeltaTime");
}

void PostProcessStepSimple::InstanceSimple::destroy()
{
}

void PostProcessStepSimple::InstanceSimple::render(
	PostProcess* postProcess,
	render::IRenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	postProcess->prepareShader(m_shader);

	m_shader->setFloatParameter(m_handleTime, m_time);
	m_shader->setFloatParameter(m_handleDeltaTime, params.deltaTime);

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
