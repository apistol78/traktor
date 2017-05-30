/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Sound/SoundBuffer.h"
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

void SoundRenderer::play(const Sound* sound)
{
	if (m_soundPlayer)
	{
		Ref< sound::Sound > snd = new sound::Sound(new SoundBuffer(sound), 0, 1.0f, 0.0f, 1.0f, 0.0f);
		m_soundPlayer->play(snd, 0);
	}
}

	}
}
