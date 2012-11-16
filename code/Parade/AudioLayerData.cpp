#include "Parade/AudioLayer.h"
#include "Parade/AudioLayerData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Script/IScriptContext.h"
#include "Sound/Sound.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.parade.AudioLayerData", 0, AudioLayerData, LayerData)

Ref< Layer > AudioLayerData::createInstance(Stage* stage, amalgam::IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();

	resource::Proxy< script::IScriptContext > script;
	resource::Proxy< sound::Sound > sound;

	// Bind proxies to resource manager.
	if (!resourceManager->bind(m_sound, sound))
		return 0;
	if (m_script && !resourceManager->bind(m_script, script))
		return 0;

	// Create layer instance.
	return new AudioLayer(
		stage,
		m_name,
		environment,
		script,
		sound
	);
}

bool AudioLayerData::serialize(ISerializer& s)
{
	if (!LayerData::serialize(s))
		return false;

	s >> resource::Member< sound::Sound >(L"sound", m_sound);

	return true;
}

	}
}
