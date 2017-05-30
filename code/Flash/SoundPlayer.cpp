/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/SoundPlayer.h"
#include "Flash/ISoundRenderer.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.SoundPlayer", SoundPlayer, Object)

SoundPlayer::SoundPlayer(ISoundRenderer* soundRenderer)
:	m_soundRenderer(soundRenderer)
{
}

void SoundPlayer::play(const Sound* sound)
{
	if (m_soundRenderer)
		m_soundRenderer->play(sound);
}

	}
}
