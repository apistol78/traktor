#ifndef traktor_sound_Mp3StreamDecoder_H
#define traktor_sound_Mp3StreamDecoder_H

#include "Core/Heap/Ref.h"
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
	T_RTTI_CLASS(Mp3StreamDecoder)

public:
	virtual bool create(Stream* stream);

	virtual void destroy();

	virtual double getDuration() const;

	virtual bool getBlock(SoundBlock& outSoundBlock);

	virtual void rewind();

private:
	Ref< Stream > m_stream;
	Ref< Mp3StreamDecoderImpl > m_decoderImpl;
};

	}
}

#endif	// traktor_sound_Mp3StreamDecoder_H
