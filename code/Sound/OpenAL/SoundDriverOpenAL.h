#ifndef traktor_sound_SoundDriverOpenAL_H
#define traktor_sound_SoundDriverOpenAL_H

#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include "Sound/ISoundDriver.h"
#include "Core/Misc/AutoPtr.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_OPENAL_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS(SoundDriverOpenAL)
	
public:
	SoundDriverOpenAL();
	
	virtual bool create(const SoundDriverCreateDesc& desc);
	
	virtual void destroy();
	
	virtual void wait();
	
	virtual void submit(const SoundBlock& soundBlock);
	
private:
	ALCdevice* m_device;
	ALCcontext* m_context;
	SoundDriverCreateDesc m_desc;
	ALuint m_format;
	ALuint m_buffers[3];
	ALuint m_source;
	uint32_t m_submitted;
	AutoArrayPtr< uint8_t > m_data;
};
	
	}
}

#endif	// traktor_sound_SoundDriverOpenAL_H
