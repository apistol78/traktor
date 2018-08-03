/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
#include "Render/ImageProcess/ImageProcessStepGrain.h"
#include "Render/ImageProcess/ImageProcess.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepGrain", 1, ImageProcessStepGrain, ImageProcessStep)

Ref< ImageProcessStep::Instance > ImageProcessStepGrain::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	resource::Proxy< Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return 0;

	std::vector< InstanceGrain::Source > sources(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		sources[i].param = getParameterHandle(m_sources[i].param);
		sources[i].source = getParameterHandle(m_sources[i].source);
	}

	return new InstanceGrain(this, shader, sources);
}

void ImageProcessStepGrain::serialize(ISerializer& s)
{
	s >> resource::Member< Shader >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
}

ImageProcessStepGrain::Source::Source()
{
}

void ImageProcessStepGrain::Source::serialize(ISerializer& s)
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

ImageProcessStepGrain::InstanceGrain::InstanceGrain(const ImageProcessStepGrain* step, const resource::Proxy< Shader >& shader, const std::vector< Source >& sources)
:	m_step(step)
,	m_shader(shader)
,	m_sources(sources)
,	m_time(0.0f)
{
	m_handleTime = getParameterHandle(L"Time");
	m_handleDeltaTime = getParameterHandle(L"DeltaTime");
	m_handleNoiseOffset = getParameterHandle(L"NoiseOffset");
}

void ImageProcessStepGrain::InstanceGrain::destroy()
{
}

void ImageProcessStepGrain::InstanceGrain::render(
	ImageProcess* imageProcess,
	IRenderView* renderView,
	ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	imageProcess->prepareShader(m_shader);

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
		ISimpleTexture* source = imageProcess->getTarget(i->source);
		if (source)
			m_shader->setTextureParameter(i->param, source);
	}

	screenRenderer->draw(renderView, m_shader);

	m_time += params.deltaTime;
}

	}
}
