#ifndef traktor_sound_StreamSoundBuffer_H
#define traktor_sound_StreamSoundBuffer_H

#include "Core/Heap/Ref.h"
#include "Sound/SoundBuffer.h"

namespace traktor
{
	namespace sound
	{

class StreamDecoder;

/*! \brief Stream sound buffer.
 * \ingroup Sound
 */
class T_DLLCLASS StreamSoundBuffer : public SoundBuffer
{
	T_RTTI_CLASS(StreamSoundBuffer)

public:
	StreamSoundBuffer();

	bool create(StreamDecoder* streamDecoder);

	void destroy();

	virtual double getDuration() const;

	virtual bool getBlock(double time, SoundBlock& outBlock);

private:
	Ref< StreamDecoder > m_streamDecoder;
	double m_duration;
	double m_time;
};

	}
}

#endif	// traktor_sound_StreamSoundBuffer_H
