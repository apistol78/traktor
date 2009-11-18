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

	virtual double getDuration() const;

	virtual bool getBlock(double time, SoundBlock& outBlock);

private:
	Ref< IStreamDecoder > m_streamDecoder;
	double m_duration;
	double m_time;
};

	}
}

#endif	// traktor_sound_StreamSoundBuffer_H
