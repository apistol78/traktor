#ifndef traktor_sound_StreamSoundBuffer_H
#define traktor_sound_StreamSoundBuffer_H

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

/*! \brief Stream sound buffer.
 * \ingroup Sound
 */
class T_DLLCLASS StreamSoundBuffer : public ISoundBuffer
{
	T_RTTI_CLASS;

public:
	StreamSoundBuffer();
	
	virtual ~StreamSoundBuffer();

	bool create(IStreamDecoder* streamDecoder);

	void destroy();

	virtual Ref< ISoundBufferCursor > createCursor() const T_OVERRIDE T_FINAL;

	virtual bool getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const T_OVERRIDE T_FINAL;

private:
	Ref< IStreamDecoder > m_streamDecoder;
	mutable uint64_t m_position;
};

	}
}

#endif	// traktor_sound_StreamSoundBuffer_H
