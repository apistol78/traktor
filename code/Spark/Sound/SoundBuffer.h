/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/IAudioBuffer.h"

namespace traktor::spark
{

class Sound;

class SoundBuffer : public sound::IAudioBuffer
{
	T_RTTI_CLASS;

public:
	explicit SoundBuffer(const Sound* sound);

	virtual Ref< sound::IAudioBufferCursor > createCursor() const override final;

	virtual bool getBlock(sound::IAudioBufferCursor* cursor, const sound::IAudioMixer* mixer, sound::AudioBlock& outBlock) const override final;

private:
	Ref< const Sound > m_sound;
};

}
