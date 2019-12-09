#include "Sound/AudioChannel.h"
#include "Sound/Tracker/VolumeSlideEvent.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.VolumeSlideEvent", VolumeSlideEvent, IEvent)

VolumeSlideEvent::VolumeSlideEvent(float amount)
:	m_amount(amount)
{
}

bool VolumeSlideEvent::execute(AudioChannel* audioChannel, int32_t& bpm, int32_t& pattern, int32_t& row) const
{
	audioChannel->setVolume(audioChannel->getVolume() + m_amount);
	return true;
}

	}
}
