/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Sound/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace sound
	{

/*! Sound stream decoder.
 * \ingroup Sound
 *
 * Stream decoders are called frequently from the mixer thread
 * when it's time to decode a new sound block from the stream.
 * Thus it's not required for the decoder itself to keep track
 * of timing, just return a continuous stream of sound blocks.
 */
class T_DLLCLASS IStreamDecoder : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(IStream* stream) = 0;

	virtual void destroy() = 0;

	virtual double getDuration() const = 0;

	virtual bool getBlock(SoundBlock& outBlock) = 0;

	virtual void rewind() = 0;
};

	}
}

