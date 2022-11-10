/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Sound/IStreamDecoder.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class Mp3StreamDecoderImpl;

/*! MP3 stream decoder.
 * \ingroup Sound
 */
class T_DLLCLASS Mp3StreamDecoder : public IStreamDecoder
{
	T_RTTI_CLASS;

public:
	virtual bool create(IStream* stream);

	virtual void destroy();

	virtual double getDuration() const;

	virtual bool getBlock(SoundBlock& outSoundBlock);

	virtual void rewind();

private:
	Ref< IStream > m_stream;
	Ref< Mp3StreamDecoderImpl > m_decoderImpl;
};

	}
}

