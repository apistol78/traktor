#include "Sound/Sound.h"
#include "Sound/SoundChannel.h"
#include "Sound/SoundSystem.h"
#include "Sound/Filters/SurroundFilter.h"
#include "Spray/SoundTriggerInstance.h"
#include "Spray/Types.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.SoundTriggerInstance", SoundTriggerInstance, ITriggerInstance)

void SoundTriggerInstance::perform(Context& context, const Transform& transform)
{
	if (!context.soundSystem)
		return;

	sound::SoundChannel* channel = context.soundSystem->play(m_sound, 16, false);
	if (channel)
	{
		if (context.surroundEnvironment)
		{
			Ref< sound::SurroundFilter > filter = new sound::SurroundFilter(context.surroundEnvironment);
			filter->setSpeakerPosition(transform.translation());
			channel->setFilter(filter);
		}
	}
}

SoundTriggerInstance::SoundTriggerInstance(const resource::Proxy< sound::Sound >& sound)
:	m_sound(sound)
{
}

	}
}
