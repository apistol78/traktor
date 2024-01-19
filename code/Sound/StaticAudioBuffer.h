/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Misc/AutoPtr.h"
#include "Sound/IAudioBuffer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

/*! Static audio buffer.
 * \ingroup Sound
 */
class T_DLLCLASS StaticAudioBuffer : public IAudioBuffer
{
	T_RTTI_CLASS;

public:
	virtual ~StaticAudioBuffer();

	bool create(uint32_t sampleRate, uint32_t samplesCount, uint32_t channelsCount);

	void destroy();

	int16_t* getSamplesData(uint32_t channel);

	virtual Ref< IAudioBufferCursor > createCursor() const override final;

	virtual bool getBlock(IAudioBufferCursor* cursor, const IAudioMixer* mixer, AudioBlock& outBlock) const override final;

private:
	int32_t m_sampleRate = 0;
	int32_t m_samplesCount = 0;
	int32_t m_channelsCount = 0;
	AutoArrayPtr< int16_t, AllocFreeAlign > m_samples[SbcMaxChannelCount];
};

}
