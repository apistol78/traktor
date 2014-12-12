#include "Amalgam/IEnvironment.h"
#include "Amalgam/Engine/VideoLayer.h"
#include "Amalgam/Engine/VideoLayerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Render/Shader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Video/Video.h"
#include "Video/VideoResource.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.VideoLayerData", LayerData::Version, VideoLayerData, LayerData)

VideoLayerData::VideoLayerData()
:	m_screenBounds(Vector2(0.0f, 0.0f), Vector2(1.0f, 1.0f))
,	m_visible(true)
,	m_autoPlay(true)
,	m_repeat(true)
{
}

Ref< Layer > VideoLayerData::createInstance(Stage* stage, amalgam::IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< video::Video > video;
	resource::Proxy< render::Shader > shader;

	// Bind proxies to resource manager.
	if (
		!resourceManager->bind(m_video, video) ||
		!resourceManager->bind(m_shader, shader)
	)
		return 0;

	// Create layer instance.
	return new VideoLayer(
		stage,
		m_name,
		m_permitTransition,
		environment,
		video,
		shader,
		m_screenBounds,
		m_visible,
		m_autoPlay,
		m_repeat
	);
}

void VideoLayerData::serialize(ISerializer& s)
{
	LayerData::serialize(s);

	s >> resource::Member< video::Video >(L"video", m_video);
	s >> resource::Member< render::Shader >(L"shader", m_shader);
	s >> MemberAabb2(L"screenBounds", m_screenBounds);
	s >> Member< bool >(L"visible", m_visible);
	s >> Member< bool >(L"autoPlay", m_autoPlay);
	s >> Member< bool >(L"repeat", m_repeat);
}

	}
}
