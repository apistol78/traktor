/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/ISoundBuffer.h"

namespace traktor
{
	namespace spark
	{

class Sound;

class SoundBuffer : public sound::ISoundBuffer
{
	T_RTTI_CLASS;

public:
	SoundBuffer(const Sound* sound);

	virtual Ref< sound::ISoundBufferCursor > createCursor() const override final;

	virtual bool getBlock(sound::ISoundBufferCursor* cursor, const sound::IAudioMixer* mixer, sound::SoundBlock& outBlock) const override final;

private:
	Ref< const Sound > m_sound;
};

	}
}

