#ifndef traktor_sound_StaticSoundBuffer_H
#define traktor_sound_StaticSoundBuffer_H

#include "Core/Misc/AutoPtr.h"
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

/*! \brief Static sound buffer.
 * \ingroup Sound
 */
class T_DLLCLASS StaticSoundBuffer : public ISoundBuffer
{
	T_RTTI_CLASS;

public:
	virtual ~StaticSoundBuffer();

	bool create(uint32_t sampleRate, uint32_t samplesCount, uint32_t channelsCount);

	void destroy();

	int16_t* getSamplesData(uint32_t channel);

	virtual Ref< ISoundBufferCursor > createCursor() const;

	virtual bool getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const;

private:
	uint32_t m_sampleRate;
	uint32_t m_samplesCount;
	uint32_t m_channelsCount;
	AutoArrayPtr< int16_t > m_samples[SbcMaxChannelCount];
};

	}
}

#endif	// traktor_sound_StaticSoundBuffer_H
