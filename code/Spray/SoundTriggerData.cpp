#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Sound/Sound.h"
#include "Spray/SoundTrigger.h"
#include "Spray/SoundTriggerData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.SoundTriggerData", 2, SoundTriggerData, ITriggerData)

SoundTriggerData::SoundTriggerData()
:	m_positional(true)
,	m_follow(false)
,	m_repeat(false)
{
}

Ref< ITrigger > SoundTriggerData::createTrigger(resource::IResourceManager* resourceManager) const
{
	resource::Proxy< sound::Sound > sound;
	if (!resourceManager->bind(m_sound, sound))
		return 0;

	return new SoundTrigger(sound, m_positional, m_follow, m_repeat);
}

bool SoundTriggerData::serialize(ISerializer& s)
{
	s >> resource::Member< sound::Sound >(L"sound", m_sound);

	if (s.getVersion() >= 2)
		s >> Member< bool >(L"positional", m_positional);

	if (s.getVersion() >= 1)
	{
		s >> Member< bool >(L"follow", m_follow);
		s >> Member< bool >(L"repeat", m_repeat);
	}

	return true;
}

	}
}
