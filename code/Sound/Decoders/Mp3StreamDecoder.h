/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_Mp3StreamDecoder_H
#define traktor_sound_Mp3StreamDecoder_H

#include "Sound/IStreamDecoder.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class Mp3StreamDecoderImpl;

/*! \brief MP3 stream decoder.
 * \ingroup Sound
 */
class T_DLLCLASS Mp3StreamDecoder : public IStreamDecoder
{
	T_RTTI_CLASS;

public:
	virtual bool create(IStream* stream) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual double getDuration() const T_OVERRIDE T_FINAL;

	virtual bool getBlock(SoundBlock& outSoundBlock) T_OVERRIDE T_FINAL;

	virtual void rewind() T_OVERRIDE T_FINAL;

private:
	Ref< IStream > m_stream;
	Ref< Mp3StreamDecoderImpl > m_decoderImpl;
};

	}
}

#endif	// traktor_sound_Mp3StreamDecoder_H
