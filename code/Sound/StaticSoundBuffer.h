#ifndef traktor_sound_StaticSoundBuffer_H
#define traktor_sound_StaticSoundBuffer_H

#include "Sound/ISoundBuffer.h"
#include "Core/Misc/AutoPtr.h"

namespace traktor
{
	namespace sound
	{

/*! \brief Static sound buffer.
 * \ingroup Sound
 */
class T_DLLCLASS StaticSoundBuffer : public ISoundBuffer
{
	T_RTTI_CLASS;

public:
	StaticSoundBuffer();

	virtual ~StaticSoundBuffer();

	bool create(uint32_t sampleRate, uint32_t samplesCount, uint32_t channelsCount);

	void destroy();

	int16_t* getSamplesData(uint32_t channel);

	virtual void reset();

	virtual bool getBlock(double time, SoundBlock& outBlock);

private:
	uint32_t m_sampleRate;
	uint32_t m_samplesCount;
	uint32_t m_channelsCount;
	AutoArrayPtr< int16_t > m_samples[SbcMaxChannelCount];
	float m_blocks[SbcMaxChannelCount][4096];
};

	}
}

#endif	// traktor_sound_StaticSoundBuffer_H
