#include "Sound/Sound.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.Sound", Sound, Object)

Sound::Sound(
	ISoundBuffer* buffer,
	handle_t category,
	float gain,
	float range
)
:	m_buffer(buffer)
,	m_category(category)
,	m_gain(gain)
,	m_range(range)
{
}

	}
}
