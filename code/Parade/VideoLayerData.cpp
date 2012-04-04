#include "Amalgam/IEnvironment.h"
#include "Core/Serialization/ISerializer.h"
#include "Parade/VideoLayer.h"
#include "Parade/VideoLayerData.h"
#include "Render/Shader.h"
#include "Render/Shader/ShaderGraph.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Script/IScriptContext.h"
#include "Video/Video.h"
#include "Video/VideoResource.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.parade.VideoLayerData", 0, VideoLayerData, LayerData)

Ref< Layer > VideoLayerData::createInstance(amalgam::IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();

	// Create a copy of all immutable proxies.
	resource::Proxy< script::IScriptContext > script = m_script;
	resource::Proxy< video::Video > video = m_video;
	resource::Proxy< render::Shader > shader = m_shader;

	// Bind proxies to resource manager.
	if (
		!resourceManager->bind(script) ||
		!resourceManager->bind(video) ||
		!resourceManager->bind(shader)
	)
		return 0;

	// Create layer instance.
	return new VideoLayer(
		m_name,
		environment,
		script,
		video,
		shader
	);
}

bool VideoLayerData::serialize(ISerializer& s)
{
	if (!LayerData::serialize(s))
		return false;

	s >> resource::Member< video::Video, video::VideoResource >(L"video", m_video);
	s >> resource::Member< render::Shader, render::ShaderGraph >(L"shader", m_shader);

	return true;
}

	}
}
