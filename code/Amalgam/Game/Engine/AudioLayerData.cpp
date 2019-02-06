#include "Amalgam/Game/Engine/AudioLayer.h"
#include "Amalgam/Game/Engine/AudioLayerData.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Sound/Sound.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.AudioLayerData", 0, AudioLayerData, LayerData)

AudioLayerData::AudioLayerData()
:	m_autoPlay(true)
,	m_repeat(true)
{
}

Ref< Layer > AudioLayerData::createInstance(Stage* stage, IEnvironment* environment) const
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	resource::Proxy< sound::Sound > sound;

	// Bind proxies to resource manager.
	if (!resourceManager->bind(m_sound, sound))
		return 0;

	// Create layer instance.
	return new AudioLayer(
		stage,
		m_name,
		m_permitTransition,
		environment,
		sound,
		m_autoPlay,
		m_repeat
	);
}

void AudioLayerData::serialize(ISerializer& s)
{
	LayerData::serialize(s);

	s >> resource::Member< sound::Sound >(L"sound", m_sound);
	s >> Member< bool >(L"autoPlay", m_autoPlay);
	s >> Member< bool >(L"repeat", m_repeat);
}

	}
}
