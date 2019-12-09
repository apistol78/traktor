#pragma once

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

/*! \brief Null device sound driver.
 * \ingroup Sound
 */
class T_DLLCLASS SoundDriverNull : public ISoundDriver
{
	T_RTTI_CLASS;

public:
	virtual bool create(const SystemApplication& sysapp, const SoundDriverCreateDesc& desc, Ref< IAudioMixer >& outMixer) override final;

	virtual void destroy() override final;

	virtual void wait() override final;

	virtual void submit(const SoundBlock& soundBlock) override final;

private:
	SoundDriverCreateDesc m_desc;
};

	}
}

