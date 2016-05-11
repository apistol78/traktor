#ifndef traktor_sound_SoundDriverWriteOut_H
#define traktor_sound_SoundDriverWriteOut_H

#include "Core/Misc/AutoPtr.h"
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

class IStream;

	namespace sound
	{

/*! \brief Write-out device sound driver.
 * \ingroup Sound
 */
class T_DLLCLASS SoundDriverWriteOut : public ISoundDriver
{
	T_RTTI_CLASS;

public:
	SoundDriverWriteOut(ISoundDriver* childDriver = 0);

	virtual bool create(const SystemApplication& sysapp, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void wait() T_OVERRIDE T_FINAL;

	virtual void submit(const SoundBlock& soundBlock) T_OVERRIDE T_FINAL;

private:
	Ref< ISoundDriver > m_childDriver;
	SoundDriverCreateDesc m_desc;
	Ref< IStream > m_stream;
	AutoArrayPtr< float > m_interleaved;
	float m_peek;
	bool m_wait;
};

	}
}

#endif	// traktor_sound_SoundDriverWriteOut_H
