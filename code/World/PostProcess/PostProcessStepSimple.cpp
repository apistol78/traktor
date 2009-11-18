#include "World/PostProcess/PostProcessStepSimple.h"
#include "World/PostProcess/PostProcess.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/ShaderGraph.h"
#include "Render/RenderTargetSet.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PostProcessStepSimple", PostProcessStepSimple, PostProcessStep)

Ref< PostProcessStep::Instance > PostProcessStepSimple::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const
{
	if (!resourceManager->bind(m_shader))
		return false;

	return new InstanceSimple(this);
}

bool PostProcessStepSimple::serialize(ISerializer& s)
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

bool PostProcessStepSimple::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< int32_t >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
	return true;
}

// Instance

PostProcessStepSimple::InstanceSimple::InstanceSimple(const PostProcessStepSimple* step)
:	m_step(step)
,	m_time(0.0f)
{
}

void PostProcessStepSimple::InstanceSimple::destroy()
{
}

void PostProcessStepSimple::InstanceSimple::render(
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

	shader->setFloatParameter(L"Time", m_time);
	shader->setFloatParameter(L"DeltaTime", deltaTime);

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

	screenRenderer->draw(renderView, shader);

	m_time += deltaTime;
}

	}
}
