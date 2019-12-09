#pragma once

#include "Sound/IAudioDriver.h"

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

/*! Null device sound driver.
 * \ingroup Sound
 */
class T_DLLCLASS AudioDriverNull : public IAudioDriver
{
	T_RTTI_CLASS;

public:
	virtual bool create(const SystemApplication& sysapp, const AudioDriverCreateDesc& desc, Ref< IAudioMixer >& outMixer) override final;

	virtual void destroy() override final;

	virtual void wait() override final;

	virtual void submit(const SoundBlock& soundBlock) override final;

private:
	AudioDriverCreateDesc m_desc;
};

	}
}

