#include "Amalgam/IEnvironment.h"
#include "Amalgam/IUpdateInfo.h"
#include "Amalgam/Stage.h"
#include "Amalgam/VideoLayer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/IRenderView.h"
#include "Render/ISimpleTexture.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Video/Video.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.VideoLayer", VideoLayer, Layer)

VideoLayer::VideoLayer(
	Stage* stage,
	const std::wstring& name,
	amalgam::IEnvironment* environment,
	const resource::Proxy< video::Video >& video,
	const resource::Proxy< render::Shader >& shader
)
:	Layer(stage, name)
,	m_environment(environment)
,	m_video(video)
,	m_shader(shader)
{
}

VideoLayer::~VideoLayer()
{
	destroy();
}

void VideoLayer::destroy()
{
	safeDestroy(m_screenRenderer);
}

bool VideoLayer::playing() const
{
	if (m_video)
		return m_video->playing();
	else
		return false;
}

void VideoLayer::prepare()
{
}

void VideoLayer::update(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	if (!m_video->playing())
		return;

	if (!m_video->update(info.getSimulationDeltaTime()))
		getStage()->invokeScript("videoFinished", 0, 0);
}

void VideoLayer::build(const amalgam::IUpdateInfo& info, uint32_t frame)
{
}

void VideoLayer::render(render::EyeType eye, uint32_t frame)
{
	if (!m_screenRenderer)
	{
		m_screenRenderer = new render::ScreenRenderer();
		if (!m_screenRenderer->create(m_environment->getRender()->getRenderSystem()))
		{
			m_screenRenderer = 0;
			return;
		}
	}

	render::ISimpleTexture* texture = m_video->getTexture();
	if (texture)
	{
		m_shader->setTextureParameter(L"Texture", texture);
		m_shader->setFloatParameter(L"Format", float(m_video->getFormat()));

		m_screenRenderer->draw(
			m_environment->getRender()->getRenderView(),
			m_shader
		);
	}
}

void VideoLayer::preReconfigured()
{
}

void VideoLayer::postReconfigured()
{
}

	}
}
