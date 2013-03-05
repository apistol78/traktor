#include "Flash/Sound/FlashSoundBuffer.h"
#include "Flash/Sound/SoundRenderer.h"
#include "Sound/Sound.h"
#include "Sound/Player/ISoundPlayer.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.SoundRenderer", SoundRenderer, ISoundRenderer)

bool SoundRenderer::create(
	sound::ISoundPlayer* soundPlayer
)
{
	m_soundPlayer = soundPlayer;
	return true;
}

void SoundRenderer::destroy()
{
	m_soundPlayer = 0;
}

void SoundRenderer::play(const FlashSound* sound)
{
	if (m_soundPlayer)
	{
		Ref< sound::Sound > snd = new sound::Sound(new FlashSoundBuffer(sound), 1.0f, 0.0f, 1.0f, 0.0f);
		m_soundPlayer->play(snd, 0);
	}
}

	}
}
