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

void SoundComponentInstance::eventKey(wchar_t unicode)
{
}

void SoundComponentInstance::eventKeyDown(int32_t keyCode)
{
}

void SoundComponentInstance::eventKeyUp(int32_t keyCode)
{
}

void SoundComponentInstance::eventMouseDown(int32_t x, int32_t y, int32_t button)
{
}

void SoundComponentInstance::eventMouseUp(int32_t x, int32_t y, int32_t button)
{
}

void SoundComponentInstance::eventMouseMove(int32_t x, int32_t y, int32_t button)
{
}

void SoundComponentInstance::eventMouseWheel(int32_t x, int32_t y, int32_t delta)
{
}

void SoundComponentInstance::eventViewResize(int32_t width, int32_t height)
{
}

	}
}
