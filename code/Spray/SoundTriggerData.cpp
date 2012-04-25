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

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.SoundTriggerData", 0, SoundTriggerData, ITriggerData)

Ref< ITrigger > SoundTriggerData::createTrigger(resource::IResourceManager* resourceManager) const
{
	resource::Proxy< sound::Sound > sound;
	if (!resourceManager->bind(m_sound, sound))
		return 0;

	return new SoundTrigger(sound);
}

bool SoundTriggerData::serialize(ISerializer& s)
{
	return s >> resource::Member< sound::Sound >(L"sound", m_sound);
}

	}
}
