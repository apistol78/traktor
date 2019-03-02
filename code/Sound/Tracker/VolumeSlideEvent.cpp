#include "Sound/SoundChannel.h"
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

bool VolumeSlideEvent::execute(SoundChannel* soundChannel, int32_t& bpm, int32_t& pattern, int32_t& row) const
{
	soundChannel->setVolume(soundChannel->getVolume() + m_amount);
	return true;
}

	}
}
