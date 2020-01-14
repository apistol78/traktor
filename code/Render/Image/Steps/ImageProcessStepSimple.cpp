#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Serialization/MemberComposite.h"
#include "Render/IRenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Render/Image/ImageProcess.h"
#include "Render/Image/Steps/ImageProcessStepSimple.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepSimple", 1, ImageProcessStepSimple, ImageProcessStep)

Ref< ImageProcessStep::Instance > ImageProcessStepSimple::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	resource::Proxy< Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return nullptr;

	std::vector< InstanceSimple::Source > sources(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		sources[i].param = getParameterHandle(m_sources[i].param);
		sources[i].source = getParameterHandle(m_sources[i].source);
	}

	return new InstanceSimple(this, shader, sources);
}

void ImageProcessStepSimple::serialize(ISerializer& s)
{
	s >> resource::Member< Shader >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
}

ImageProcessStepSimple::Source::Source()
{
}

void ImageProcessStepSimple::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< std::wstring >(L"source", source);

	if (s.getVersion() < 1)
	{
		uint32_t index = 0;
		s >> Member< uint32_t >(L"index", index);
		T_FATAL_ASSERT_M (index == 0, L"Index must be zero, update binding");
	}
}

// Instance

ImageProcessStepSimple::InstanceSimple::InstanceSimple(const ImageProcessStepSimple* step, const resource::Proxy< Shader >& shader, const std::vector< Source >& sources)
:	m_step(step)
,	m_shader(shader)
,	m_sources(sources)
,	m_time(0.0f)
{
	m_handleTime = getParameterHandle(L"Time");
	m_handleDeltaTime = getParameterHandle(L"DeltaTime");
}

void ImageProcessStepSimple::InstanceSimple::destroy()
{
	m_step = nullptr;
	m_shader.clear();
	m_sources.clear();
}

void ImageProcessStepSimple::InstanceSimple::render(
	ImageProcess* imageProcess,
	RenderContext* renderContext,
	ProgramParameters* sharedParams,
	const RenderParams& params
)
{
	auto pp = renderContext->alloc< ProgramParameters >();
	pp->beginParameters(renderContext);
	pp->attachParameters(sharedParams);
	pp->setFloatParameter(m_handleTime, m_time);
	pp->setFloatParameter(m_handleDeltaTime, params.deltaTime);
	for (const auto& s : m_sources)
	{
		ISimpleTexture* source = imageProcess->getTarget(s.source);
		if (source)
			pp->setTextureParameter(s.param, source);		
	}
	pp->endParameters(renderContext);

	imageProcess->getScreenRenderer()->draw(renderContext, m_shader, pp);

	m_time += params.deltaTime;
}

	}
}
