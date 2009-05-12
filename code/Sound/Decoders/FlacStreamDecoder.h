#ifndef traktor_sound_FlacStreamDecoder_H
#define traktor_sound_FlacStreamDecoder_H

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

/*! \brief FLAC stream decoder.
 * \ingroup Sound
 */
class T_DLLCLASS FlacStreamDecoder : public StreamDecoder
{
	T_RTTI_CLASS(FlacStreamDecoder)

public:
	virtual bool create(Stream* stream);

	virtual void destroy();

	virtual double getDuration() const;

	virtual bool getBlock(SoundBlock& outSoundBlock);

	virtual void rewind();

private:
	Ref< Stream > m_stream;
	Ref< class FlacStreamDecoderImpl > m_decoderImpl;
};

	}
}

#endif	// traktor_sound_FlacStreamDecoder_H
