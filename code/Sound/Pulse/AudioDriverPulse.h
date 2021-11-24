#pragma once

#include <pulse/simple.h>
#include "Core/Misc/AutoPtr.h"
#include "Sound/IAudioDriver.h"

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
class T_DLLCLASS AudioDriverPulse : public IAudioDriver
{
	T_RTTI_CLASS;

public:
	virtual bool create(const SystemApplication& sysapp, const AudioDriverCreateDesc& desc, Ref< IAudioMixer >& outMixer) override final;

	virtual void destroy() override final;

	virtual void wait() override final;

	virtual void submit(const SoundBlock& soundBlock) override final;

private:
	pa_sample_spec m_ss;
	pa_simple* m_pa = nullptr;
	AutoArrayPtr< float > m_pending;
	uint32_t m_pendingSize = 0;
};

	}
}

