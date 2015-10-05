#ifndef traktor_sound_SoundDriverWinMM_H
#define traktor_sound_SoundDriverWinMM_H

#include <windows.h>
#include "Sound/ISoundDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_WINMM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*!
 * \ingroup WMM
 */
class T_DLLCLASS SoundDriverWinMM : public ISoundDriver
{
	T_RTTI_CLASS;

public:
	SoundDriverWinMM();

	virtual bool create(void* nativeHandle, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void wait() T_OVERRIDE T_FINAL;

	virtual void submit(const SoundBlock& soundBlock) T_OVERRIDE T_FINAL;

private:
	HWAVEOUT m_wo;
	WAVEFORMATEX m_wfx;
	HANDLE m_eventNotify;
	uint8_t* m_buffer;
	WAVEHDR m_blocks[3];
	uint32_t m_nextPrepareBlock;
};

	}
}

#endif	// traktor_sound_SoundDriverWinMM_H
