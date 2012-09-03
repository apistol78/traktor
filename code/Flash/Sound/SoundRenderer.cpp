#include "Flash/Sound/FlashSoundBuffer.h"
#include "Flash/Sound/SoundRenderer.h"
#include "Sound/Sound.h"
#include "Sound/SoundChannel.h"
#include "Sound/SoundSystem.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.SoundRenderer", SoundRenderer, ISoundRenderer)

bool SoundRenderer::create(
	sound::SoundSystem* soundSystem
)
{
	m_soundSystem = soundSystem;
	return true;
}

void SoundRenderer::destroy()
{
	if (m_soundChannel)
	{
		m_soundChannel->stop();
		m_soundChannel = 0;
	}
}

void SoundRenderer::play(const FlashSound* sound)
{
	m_soundChannel = m_soundSystem->play(
		new sound::Sound(new FlashSoundBuffer(sound), 1.0f),
		0,
		true
	);
}

	}
}
