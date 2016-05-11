#ifndef traktor_sound_SoundDriverPulse_H
#define traktor_sound_SoundDriverPulse_H

#include <pulse/simple.h>
#include "Core/Misc/AutoPtr.h"
#include "Sound/ISoundDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_PULSE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*!
 * \ingroup Sound Pulse
 */
class T_DLLCLASS SoundDriverPulse : public ISoundDriver
{
	T_RTTI_CLASS;

public:
	SoundDriverPulse();

	virtual bool create(const SystemApplication& sysapp, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void wait() T_OVERRIDE T_FINAL;

	virtual void submit(const SoundBlock& soundBlock) T_OVERRIDE T_FINAL;

private:
	pa_sample_spec m_ss;
	pa_simple* m_pa;
	AutoArrayPtr< float > m_pending;
	uint32_t m_pendingSize;
};

	}
}

#endif	// traktor_sound_SoundDriverPulse_H
