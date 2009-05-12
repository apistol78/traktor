#ifndef traktor_sound_WavStreamDecoder_H
#define traktor_sound_WavStreamDecoder_H

#include "Core/Heap/Ref.h"
#include "Sound/StreamDecoder.h"

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

/*! \ingroup Sound */
//@{

#pragma pack(1)

struct WaveFormat
{
	uint16_t compression;
	uint16_t channels;
	uint32_t sampleRate;
	uint32_t averageBytesPerSecond;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
};

#pragma pack()

/*! \brief WAV stream decoder.
 */
class T_DLLCLASS WavStreamDecoder : public StreamDecoder
{
	T_RTTI_CLASS(WavStreamDecoder)

public:
	virtual bool create(Stream* stream);

	virtual void destroy();

	virtual double getDuration() const;

	virtual bool getBlock(SoundBlock& outSoundBlock);

	virtual void rewind();

private:
	Ref< Stream > m_stream;
	WaveFormat m_format;
	float m_samplesBuffer[16384];

	bool readHeader();
};

//@}

	}
}

#endif	// traktor_sound_WavStreamDecoder_H
