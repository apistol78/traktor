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

	resource::Proxy< script::IScriptContext > script;
	resource::Proxy< video::Video > video;
	resource::Proxy< render::Shader > shader;

	// Bind proxies to resource manager.
	if (
		!resourceManager->bind(m_video, video) ||
		!resourceManager->bind(m_shader, shader)
	)
		return 0;
	if (m_script && !resourceManager->bind(m_script, script))
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

	s >> resource::Member< video::Video >(L"video", m_video);
	s >> resource::Member< render::Shader >(L"shader", m_shader);

	return true;
}

	}
}
