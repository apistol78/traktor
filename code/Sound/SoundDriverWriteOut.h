#ifndef traktor_sound_SoundDriverWriteOut_H
#define traktor_sound_SoundDriverWriteOut_H

#include "Sound/ISoundDriver.h"

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

/*! \brief Write-out device sound driver.
 * \ingroup Sound
 */
class T_DLLCLASS SoundDriverWriteOut : public ISoundDriver
{
	T_RTTI_CLASS;

public:
	SoundDriverWriteOut();

	virtual bool create(const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer);

	virtual void destroy();

	virtual void wait();

	virtual void submit(const SoundBlock& soundBlock);

private:
	SoundDriverCreateDesc m_desc;
	Ref< IStream > m_streams[SbcMaxChannelCount];
	float m_peek;
	bool m_wait;
};

	}
}

#endif	// traktor_sound_SoundDriverWriteOut_H
