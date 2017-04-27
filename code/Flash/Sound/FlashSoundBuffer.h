/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_FlashSoundBuffer_H
#define traktor_flash_FlashSoundBuffer_H

#include "Sound/ISoundBuffer.h"

namespace traktor
{
	namespace flash
	{

class FlashSound;

class FlashSoundBuffer : public sound::ISoundBuffer
{
	T_RTTI_CLASS;

public:
	FlashSoundBuffer(const FlashSound* sound);

	virtual Ref< sound::ISoundBufferCursor > createCursor() const T_OVERRIDE T_FINAL;

	virtual bool getBlock(sound::ISoundBufferCursor* cursor, const sound::ISoundMixer* mixer, sound::SoundBlock& outBlock) const T_OVERRIDE T_FINAL;

private:
	Ref< const FlashSound > m_sound;
};

	}
}

#endif	// traktor_flash_FlashSoundBuffer_H
