#ifndef traktor_sound_StreamSoundBuffer_H
#define traktor_sound_StreamSoundBuffer_H

#include "Sound/ISoundBuffer.h"

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

	bool create(IStreamDecoder* streamDecoder);

	void destroy();

	virtual Ref< ISoundBufferCursor > createCursor() const;

	virtual bool getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const;

private:
	Ref< IStreamDecoder > m_streamDecoder;
	mutable double m_time;
};

	}
}

#endif	// traktor_sound_StreamSoundBuffer_H
