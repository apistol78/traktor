#include "Sound/Sound.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.Sound", Sound, Object)

Sound::Sound(SoundBuffer* soundBuffer)
:	m_soundBuffer(soundBuffer)
{
}

SoundBuffer* Sound::getSoundBuffer()
{
	return m_soundBuffer;
}

	}
}
