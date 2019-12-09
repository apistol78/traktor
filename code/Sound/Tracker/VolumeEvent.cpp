#include "Sound/AudioChannel.h"
#include "Sound/Tracker/VolumeEvent.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.VolumeEvent", VolumeEvent, IEvent)

VolumeEvent::VolumeEvent(float volume)
:	m_volume(volume)
{
}

bool VolumeEvent::execute(AudioChannel* audioChannel, int32_t& bpm, int32_t& pattern, int32_t& row) const
{
	audioChannel->setVolume(m_volume);
	return true;
}

	}
}
