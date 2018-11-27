#include "Sound/Tracker/Pattern.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.Pattern", Pattern, Object)

Pattern::Pattern(int32_t duration, const RefArray< Track >& tracks)
:	m_duration(duration)
,	m_tracks(tracks)
{
}

	}
}
