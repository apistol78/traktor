#include "Sound/Tracker/Play.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.Play", Play, Object)

Play::Play(const resource::Proxy< Sound >& sound, int32_t note, int32_t repeatFrom, int32_t repeatLength)
:	m_sound(sound)
,	m_note(note)
,	m_repeatFrom(repeatFrom)
,	m_repeatLength(repeatLength)
{
}

	}
}
