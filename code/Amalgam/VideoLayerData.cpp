#include "Amalgam/IEnvironment.h"
#include "Amalgam/VideoLayer.h"
#include "Amalgam/VideoLayerData.h"
#include "Core/Serialization/ISerializer.h"
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

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.VideoLayerData", 0, VideoLayerData, LayerData)

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
		environment,
		video,
		shader
	);
}

void VideoLayerData::serialize(ISerializer& s)
{
	LayerData::serialize(s);

	s >> resource::Member< video::Video >(L"video", m_video);
	s >> resource::Member< render::Shader >(L"shader", m_shader);
}

	}
}
