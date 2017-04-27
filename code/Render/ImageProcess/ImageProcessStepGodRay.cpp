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
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Render/ImageProcess/ImageProcessStepGodRay.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.ImageProcessStepGodRay", 0, ImageProcessStepGodRay, ImageProcessStep)

ImageProcessStepGodRay::ImageProcessStepGodRay()
:	m_lightDistance(100.0f)
{
}

Ref< ImageProcessStep::Instance > ImageProcessStepGodRay::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
) const
{
	resource::Proxy< Shader > shader;
	if (!resourceManager->bind(m_shader, shader))
		return 0;

	std::vector< InstanceGodRay::Source > sources(m_sources.size());
	for (uint32_t i = 0; i < m_sources.size(); ++i)
	{
		sources[i].param = getParameterHandle(m_sources[i].param);
		sources[i].source = getParameterHandle(m_sources[i].source);
		sources[i].index = m_sources[i].index;
	}

	return new InstanceGodRay(this, shader, sources);
}

void ImageProcessStepGodRay::serialize(ISerializer& s)
{
	s >> resource::Member< Shader >(L"shader", m_shader);
	s >> MemberStlVector< Source, MemberComposite< Source > >(L"sources", m_sources);
	s >> Member< float >(L"lightDistance", m_lightDistance);
}

ImageProcessStepGodRay::Source::Source()
:	index(0)
{
}

void ImageProcessStepGodRay::Source::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"param", param);
	s >> Member< std::wstring >(L"source", source);
	s >> Member< uint32_t >(L"index", index);
}

// Instance

ImageProcessStepGodRay::InstanceGodRay::InstanceGodRay(const ImageProcessStepGodRay* step, const resource::Proxy< Shader >& shader, const std::vector< Source >& sources)
:	m_step(step)
,	m_shader(shader)
,	m_sources(sources)
,	m_time(0.0f)
{
	m_handleTime = getParameterHandle(L"Time");
	m_handleDeltaTime = getParameterHandle(L"DeltaTime");
	m_handleAlpha = getParameterHandle(L"Alpha");
	m_handleScreenLightPosition = getParameterHandle(L"ScreenLightPosition");
}

void ImageProcessStepGodRay::InstanceGodRay::destroy()
{
}

void ImageProcessStepGodRay::InstanceGodRay::render(
	ImageProcess* imageProcess,
	IRenderView* renderView,
	ScreenRenderer* screenRenderer,
	const RenderParams& params
)
{
	Vector4 lightPositionView = params.view * (params.godRayDirection * Scalar(m_step->m_lightDistance)).xyz1();
	Vector4 lightPosition = params.projection * lightPositionView;
	
	// If light is behind camera near plane then skip god rays; but
	// ensure target is cleared so no lingering rays are kept.
	if (params.godRayDirection.length2() <= FUZZY_EPSILON || lightPosition.w() <= 0.0f)
	{
		const static Color4f c_clearColor(0.0f, 0.0f, 0.0f, 0.0f);
		renderView->clear(CfColor, &c_clearColor, 0.0f, 0);
		return;
	}

	lightPosition /= lightPosition.w();

	imageProcess->prepareShader(m_shader);

	m_shader->setFloatParameter(m_handleTime, m_time);
	m_shader->setFloatParameter(m_handleDeltaTime, params.deltaTime);
	m_shader->setFloatParameter(m_handleAlpha, abs(lightPositionView.xyz0().normalized().z()));
	m_shader->setVectorParameter(m_handleScreenLightPosition, lightPosition * Vector4(1.0f, -1.0f, 0.0f, 0.0f));

	for (std::vector< Source >::const_iterator i = m_sources.begin(); i != m_sources.end(); ++i)
	{
		RenderTargetSet* source = imageProcess->getTarget(i->source);
		if (source)
			m_shader->setTextureParameter(i->param, source->getColorTexture(i->index));
	}

	screenRenderer->draw(renderView, m_shader);

	m_time += params.deltaTime;
}

	}
}
