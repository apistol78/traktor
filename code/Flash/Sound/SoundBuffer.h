/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_SoundBuffer_H
#define traktor_flash_SoundBuffer_H

#include "Sound/ISoundBuffer.h"

namespace traktor
{
	namespace flash
	{

class Sound;

class SoundBuffer : public sound::ISoundBuffer
{
	T_RTTI_CLASS;

public:
	SoundBuffer(const Sound* sound);

	virtual Ref< sound::ISoundBufferCursor > createCursor() const T_OVERRIDE T_FINAL;

	virtual bool getBlock(sound::ISoundBufferCursor* cursor, const sound::ISoundMixer* mixer, sound::SoundBlock& outBlock) const T_OVERRIDE T_FINAL;

private:
	Ref< const Sound > m_sound;
};

	}
}

#endif	// traktor_flash_SoundBuffer_H
