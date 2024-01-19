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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class IStreamDecoder;

/*! Stream audio buffer.
 * \ingroup Sound
 */
class T_DLLCLASS StreamAudioBuffer : public IAudioBuffer
{
	T_RTTI_CLASS;

public:
	virtual ~StreamAudioBuffer();

	bool create(IStreamDecoder* streamDecoder);

	void destroy();

	virtual Ref< IAudioBufferCursor > createCursor() const override final;

	virtual bool getBlock(IAudioBufferCursor* cursor, const IAudioMixer* mixer, AudioBlock& outBlock) const override final;

private:
	Ref< IStreamDecoder > m_streamDecoder;
	mutable uint64_t m_position = 0;
};

}
