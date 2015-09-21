#include "Sound/Sound.h"
#include "Sound/Player/ISoundHandle.h"
#include "Sound/Player/ISoundPlayer.h"
#include "Spark/SoundComponentInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SoundComponentInstance", SoundComponentInstance, IComponentInstance)

SoundComponentInstance::SoundComponentInstance(sound::ISoundPlayer* soundPlayer, const SmallMap< std::wstring, resource::Proxy< sound::Sound > >& sounds)
:	m_soundPlayer(soundPlayer)
,	m_sounds(sounds)
{
}

Ref< sound::ISoundHandle > SoundComponentInstance::play(const std::wstring& id)
{
	SmallMap< std::wstring, resource::Proxy< sound::Sound > >::const_iterator i = m_sounds.find(id);
	if (i != m_sounds.end())
		return m_soundPlayer->play(i->second, 0);
	else
		return 0;
}

void SoundComponentInstance::update()
{
}

	}
}
