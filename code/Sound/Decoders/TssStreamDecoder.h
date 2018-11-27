#pragma once

#include "Sound/IStreamDecoder.h"

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

/*! \ingroup Sound */
//@{

/*! \brief
 */
class T_DLLCLASS TssStreamDecoder : public IStreamDecoder
{
	T_RTTI_CLASS;

public:
	TssStreamDecoder();
	
	virtual bool create(IStream* stream) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual double getDuration() const T_OVERRIDE T_FINAL;

	virtual bool getBlock(SoundBlock& outSoundBlock) T_OVERRIDE T_FINAL;

	virtual void rewind() T_OVERRIDE T_FINAL;

private:
	Ref< IStream > m_stream;
	float* m_samplesBuffer[SbcMaxChannelCount];
};

//@}

	}
}
