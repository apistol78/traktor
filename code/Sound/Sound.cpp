#include "Sound/Sound.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.Sound", Sound, Object)

Sound::Sound(
	ISoundBuffer* buffer,
	float volume,
	float presence,
	float presenceRate
)
:	m_buffer(buffer)
,	m_volume(volume)
,	m_presence(presence)
,	m_presenceRate(presenceRate)
{
}

	}
}
