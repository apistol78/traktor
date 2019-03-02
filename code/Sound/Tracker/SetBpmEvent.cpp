#include "Sound/SoundChannel.h"
#include "Sound/Tracker/SetBpmEvent.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SetBpmEvent", SetBpmEvent, IEvent)

SetBpmEvent::SetBpmEvent(int32_t bpm)
:	m_bpm(bpm)
{
}

bool SetBpmEvent::execute(SoundChannel* soundChannel, int32_t& bpm, int32_t& pattern, int32_t& row) const
{
	bpm = m_bpm;
	return true;
}

	}
}
