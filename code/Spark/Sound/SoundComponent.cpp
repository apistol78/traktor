/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Sound/Sound.h"
#include "Sound/Player/ISoundHandle.h"
#include "Sound/Player/ISoundPlayer.h"
#include "Spark/Sound/SoundComponent.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SoundComponent", SoundComponent, IComponent)

SoundComponent::SoundComponent(sound::ISoundPlayer* soundPlayer, const SmallMap< std::wstring, resource::Proxy< sound::Sound > >& sounds)
:	m_soundPlayer(soundPlayer)
,	m_sounds(sounds)
{
}

Ref< sound::ISoundHandle > SoundComponent::play(const std::wstring& id)
{
	SmallMap< std::wstring, resource::Proxy< sound::Sound > >::const_iterator i = m_sounds.find(id);
	if (i != m_sounds.end())
		return m_soundPlayer->play(i->second, 0);
	else
		return 0;
}

void SoundComponent::update()
{
}

void SoundComponent::eventKey(wchar_t unicode)
{
}

void SoundComponent::eventKeyDown(int32_t keyCode)
{
}

void SoundComponent::eventKeyUp(int32_t keyCode)
{
}

void SoundComponent::eventMouseDown(const Vector2& position, int32_t button)
{
}

void SoundComponent::eventMouseUp(const Vector2& position, int32_t button)
{
}

void SoundComponent::eventMousePress(const Vector2& position, int32_t button)
{
}

void SoundComponent::eventMouseRelease(const Vector2& position, int32_t button)
{
}

void SoundComponent::eventMouseMove(const Vector2& position, int32_t button)
{
}

void SoundComponent::eventMouseEnter(const Vector2& position, int32_t button)
{
}

void SoundComponent::eventMouseLeave(const Vector2& position, int32_t button)
{
}

void SoundComponent::eventMouseWheel(const Vector2& position, int32_t delta)
{
}

void SoundComponent::eventViewResize(int32_t width, int32_t height)
{
}

	}
}
