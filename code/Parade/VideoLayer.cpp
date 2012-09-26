#include "Amalgam/IEnvironment.h"
#include "Amalgam/IUpdateInfo.h"
#include "Core/Log/Log.h"
#include "Parade/VideoLayer.h"
#include "Parade/Stage.h"
#include "Render/IRenderView.h"
#include "Render/ISimpleTexture.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Script/Any.h"
#include "Video/Video.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.VideoLayer", VideoLayer, Layer)

VideoLayer::VideoLayer(
	const std::wstring& name,
	amalgam::IEnvironment* environment,
	const resource::Proxy< script::IScriptContext >& scriptContext,
	const resource::Proxy< video::Video >& video,
	const resource::Proxy< render::Shader >& shader
)
:	Layer(name, scriptContext)
,	m_environment(environment)
,	m_video(video)
,	m_shader(shader)
{
}

bool VideoLayer::playing() const
{
	if (m_video)
		return m_video->playing();
	else
		return false;
}

void VideoLayer::prepare(Stage* stage)
{
}

void VideoLayer::update(Stage* stage, amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	invokeScriptUpdate(stage, control, info);

	if (!m_video->playing())
		return;

	if (!m_video->update(info.getSimulationDeltaTime()))
	{
		script::Any argv[] =
		{
			script::Any(stage)
		};
		invokeScriptMethod(stage, L"videoFinished", sizeof_array(argv), argv);
	}
}

void VideoLayer::build(Stage* stage, const amalgam::IUpdateInfo& info, uint32_t frame)
{
}

void VideoLayer::render(Stage* stage, render::EyeType eye, uint32_t frame)
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

void VideoLayer::leave(Stage* stage)
{
}

void VideoLayer::reconfigured(Stage* stage)
{
}

	}
}
