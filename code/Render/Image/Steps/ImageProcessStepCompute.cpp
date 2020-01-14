#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberStaticArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Render/Image/ImageProcessStepCompute.h"
#include "Render/Image/ImageProcess.h"

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

void ImageProcessStepCompute::InstanceCompute::render(
	ImageProcess* imageProcess,
	IRenderView* renderView,
	ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	imageProcess->prepareShader(m_shader);

	m_shader->setFloatParameter(m_handleTime, m_time);
	m_shader->setFloatParameter(m_handleDeltaTime, params.deltaTime);

	for (const auto& source : m_sources)
	{
		ISimpleTexture* texture = imageProcess->getTarget(source.source);
		if (texture)
			m_shader->setTextureParameter(source.param, texture);
	}

	renderView->compute(m_shader->getCurrentProgram(), m_step->m_workSize);

	m_time += params.deltaTime;
}

	}
}
