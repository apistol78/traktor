#include "Sound/Sound.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.Sound", Sound, Object)

Sound::Sound(ISoundBuffer* soundBuffer, float volume, float presence)
:	m_soundBuffer(soundBuffer)
,	m_volume(volume)
,	m_presence(presence)
{
}

	}
}
