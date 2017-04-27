/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/FlashSoundPlayer.h"
#include "Flash/ISoundRenderer.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashSoundPlayer", FlashSoundPlayer, Object)

FlashSoundPlayer::FlashSoundPlayer(ISoundRenderer* soundRenderer)
:	m_soundRenderer(soundRenderer)
{
}

void FlashSoundPlayer::play(const FlashSound* sound)
{
	if (m_soundRenderer)
		m_soundRenderer->play(sound);
}

	}
}
