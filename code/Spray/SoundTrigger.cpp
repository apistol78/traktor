#include "Core/Serialization/ISerializer.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Sound/ISoundResource.h"
#include "Sound/Sound.h"
#include "Spray/SoundTrigger.h"
#include "Spray/SoundTriggerInstance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spray.SoundTrigger", 0, SoundTrigger, ITrigger)

bool SoundTrigger::bind(resource::IResourceManager* resourceManager)
{
	return resourceManager->bind(m_sound);
}

Ref< ITriggerInstance > SoundTrigger::createInstance() const
{
	return new SoundTriggerInstance(m_sound);
}

bool SoundTrigger::serialize(ISerializer& s)
{
	return s >> resource::Member< sound::Sound, sound::ISoundResource >(L"sound", m_sound);
}

	}
}
