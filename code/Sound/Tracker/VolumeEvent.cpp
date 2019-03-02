#include "Sound/SoundChannel.h"
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

bool VolumeEvent::execute(SoundChannel* soundChannel, int32_t& bpm, int32_t& pattern, int32_t& row) const
{
	soundChannel->setVolume(m_volume);
	return true;
}

	}
}
