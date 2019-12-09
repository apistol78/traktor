#pragma once

#include "Sound/IAudioDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_ALSA_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

struct _snd_pcm;
struct _snd_pcm_hw_params;
struct _snd_pcm_sw_params;

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS AudioDriverAlsa : public IAudioDriver
{
	T_RTTI_CLASS;

public:
	AudioDriverAlsa();

	virtual ~AudioDriverAlsa();

	virtual bool create(const SystemApplication& sysapp, const AudioDriverCreateDesc& desc, Ref< IAudioMixer >& outMixer) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void wait() T_OVERRIDE T_FINAL;

	virtual void submit(const SoundBlock& soundBlock) T_OVERRIDE T_FINAL;

private:
	AudioDriverCreateDesc m_desc;
	_snd_pcm* m_handle;
	_snd_pcm_hw_params* m_hw_params;
	_snd_pcm_sw_params* m_sw_params;
	int16_t* m_buffer;
	bool m_started;
};

	}
}

