#include "World/PostProcess/PostProcessStepSimple.h"
#include "World/PostProcess/PostProcess.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/ShaderGraph.h"
#include "Render/RenderTargetSet.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.world.PostProcessStepSimple", PostProcessStepSimple, PostProcessStep)

bool PostProcessStepSimple::create(PostProcess* postProcess, render::RenderSystem* renderSystem)
{
	m_time = 0.0f;
	return true;
}

void PostProcessStepSimple::destroy(PostProcess* postProcess)
{
}

void PostProcessStepSimple::render(
	PostProcess* postProcess,
	const WorldRenderView& worldRenderView,
	render::RenderView* renderView,
	render::ScreenRenderer* screenRenderer,
	float deltaTime
)
{
	if (!m_shader.validate())
		return;

	m_shader->setFloatParameter(L"Time", m_time);
	m_shader->setFloatParameter(L"DeltaTime", deltaTime);

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

	screenRenderer->draw(renderView, m_shader);

	m_time += deltaTime;
}

bool PostProcessStepSimple::serialize(Serializer& s)
{
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
	return true;
}

PostProcessStepSimple::Source::Source()
:	source(0)
,	index(0)
{
}

bool PostProcessStepSimple::Source::serialize(Serializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< uint32_t >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
	return true;
}

	}
}
