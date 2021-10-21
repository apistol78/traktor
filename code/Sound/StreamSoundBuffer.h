#pragma once

#include "Sound/ISoundBuffer.h"

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

class IStreamDecoder;

/*! Stream sound buffer.
 * \ingroup Sound
 */
class T_DLLCLASS StreamSoundBuffer : public ISoundBuffer
{
	T_RTTI_CLASS;

public:
	virtual ~StreamSoundBuffer();

	bool create(IStreamDecoder* streamDecoder);

	void destroy();

	virtual Ref< ISoundBufferCursor > createCursor() const override final;

	virtual bool getBlock(ISoundBufferCursor* cursor, const IAudioMixer* mixer, SoundBlock& outBlock) const override final;

private:
	Ref< IStreamDecoder > m_streamDecoder;
	mutable uint64_t m_position = 0;
};

	}
}

