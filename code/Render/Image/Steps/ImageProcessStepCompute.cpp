#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/IRenderTargetSet.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Render/Image/ImageProcess.h"
#include "Render/Image/Steps/ImageProcessStepCompute.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepCompute", 0, ImageProcessStepCompute, ImageProcessStep)

Ref< ImageProcessStep::Instance > ImageProcessStepCompute::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	resource::Proxy< Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return nullptr;

	std::vector< InstanceCompute::Source > sources(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		sources[i].param = getParameterHandle(m_sources[i].param);
		sources[i].source = getParameterHandle(m_sources[i].source);
	}

	return new InstanceCompute(this, shader, sources);
}

void ImageProcessStepCompute::serialize(ISerializer& s)
{
	s >> resource::Member< Shader >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
	s >> MemberStaticArray< int32_t, 3 >(L"workSize", m_workSize);
}

void ImageProcessStepCompute::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< std::wstring >(L"source", source);
}

// Instance

ImageProcessStepCompute::InstanceCompute::InstanceCompute(const ImageProcessStepCompute* step, const resource::Proxy< Shader >& shader, const std::vector< Source >& sources)
:	m_step(step)
,	m_shader(shader)
,	m_sources(sources)
,	m_time(0.0f)
{
	m_handleTime = getParameterHandle(L"Time");
	m_handleDeltaTime = getParameterHandle(L"DeltaTime");
}

void ImageProcessStepCompute::InstanceCompute::destroy()
{
	m_step = nullptr;
	m_shader.clear();
	m_sources.clear();
}

void ImageProcessStepCompute::InstanceCompute::build(
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

	// auto rb = renderContext->alloc< ComputeRenderBlock >();
	// rb->program = m_shader->getCurrentProgram();
	// rb->programParams = pp;
	// rb->workSize = m_step->m_workSize
	// renderContext->enqueue(rb);

	m_time += params.deltaTime;
}

	}
}
