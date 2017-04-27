/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_IStreamDecoder_H
#define traktor_sound_IStreamDecoder_H

#include "Core/Object.h"
#include "Sound/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace sound
	{

/*! \brief Sound stream decoder.
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

#endif	// traktor_sound_IStreamDecoder_H
