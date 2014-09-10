#ifndef traktor_sound_SoundDriverOpenAL_H
#define traktor_sound_SoundDriverOpenAL_H

#if defined(__APPLE__)
#   include <OpenAL/al.h>
#   include <OpenAL/alc.h>
#else
#   include <AL/al.h>
#   include <AL/alc.h>
#endif
#include "Core/Misc/AutoPtr.h"
#include "Sound/ISoundDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_OPENAL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*!
 * \ingroup OpenAL
 */
class T_DLLCLASS SoundDriverOpenAL : public ISoundDriver
{
	T_RTTI_CLASS;

public:
	SoundDriverOpenAL();

	virtual bool create(void* nativeHandle, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer);

	virtual void destroy();

	virtual void wait();

	virtual void submit(const SoundBlock& soundBlock);

private:
	ALCdevice* m_device;
	ALCcontext* m_context;
	SoundDriverCreateDesc m_desc;
	ALuint m_format;
	ALuint m_buffers[4];
	ALuint m_source;
	uint32_t m_submitted;
	AutoArrayPtr< uint8_t > m_data;
};

	}
}

#endif	// traktor_sound_SoundDriverOpenAL_H
