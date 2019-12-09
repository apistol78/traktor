#include "Sound/AudioChannel.h"
#include "Sound/Tracker/GotoEvent.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.GotoEvent", GotoEvent, IEvent)

GotoEvent::GotoEvent(int32_t pattern, int32_t row)
:	m_pattern(pattern)
,	m_row(row)
{
}

bool GotoEvent::execute(AudioChannel* audioChannel, int32_t& bpm, int32_t& pattern, int32_t& row) const
{
	pattern = m_pattern;
	row = m_row;
	return true;
}

	}
}
