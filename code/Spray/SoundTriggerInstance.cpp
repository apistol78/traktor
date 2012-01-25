#include "Sound/SoundSystem.h"
#include "Spray/SoundTriggerInstance.h"
#include "Spray/Types.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SoundTriggerInstance", SoundTriggerInstance, ITriggerInstance)

void SoundTriggerInstance::perform(Context& context)
{
	if (context.soundSystem && m_sound.validate())
		context.soundSystem->play(m_sound, 0, false);
}

SoundTriggerInstance::SoundTriggerInstance(const resource::Proxy< sound::Sound >& sound)
:	m_sound(sound)
{
}

	}
}
