/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_SoundPlayer_H
#define traktor_flash_SoundPlayer_H

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace flash
	{

class Sound;
class ISoundRenderer;

class SoundPlayer : public Object
{
	T_RTTI_CLASS;

public:
	SoundPlayer(ISoundRenderer* soundRenderer);

	void play(const Sound* sound);

private:
	Ref< ISoundRenderer > m_soundRenderer;
};

	}
}

#endif	// traktor_flash_SoundPlayer_H
