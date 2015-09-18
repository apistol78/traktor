#include "Sound/Sound.h"
#include "Sound/Player/ISoundHandle.h"
#include "Sound/Player/ISoundPlayer.h"
#include "Spark/SoundComponentInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SoundComponentInstance", SoundComponentInstance, IComponentInstance)

SoundComponentInstance::SoundComponentInstance(sound::ISoundPlayer* soundPlayer, const resource::Proxy< sound::Sound >& sound)
:	m_soundPlayer(soundPlayer)
,	m_sound(sound)
{
}

Ref< sound::ISoundHandle > SoundComponentInstance::play()
{
	return m_soundPlayer->play(m_sound, 0);
}

void SoundComponentInstance::update()
{
}

	}
}
