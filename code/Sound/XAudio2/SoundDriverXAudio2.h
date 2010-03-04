#ifndef traktor_sound_SoundDriverXAudio2_H
#define traktor_sound_SoundDriverXAudio2_H

#if !defined(_XBOX)
#define _WIN32_DCOM
#include <windows.h>
#endif
#include <xaudio2.h>
#include "Sound/ISoundDriver.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_XAUDIO2_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*!
 * \ingroup XA2
 */
class T_DLLCLASS SoundDriverXAudio2 : public ISoundDriver
{
	T_RTTI_CLASS;

public:
	SoundDriverXAudio2();

	virtual ~SoundDriverXAudio2();

	virtual bool create(const SoundDriverCreateDesc& desc);

	virtual void destroy();

	virtual void wait();

	virtual void submit(const SoundBlock& soundBlock);

private:
	ComRef< IXAudio2 > m_audio;
	IXAudio2VoiceCallback* m_voiceCallback;
	IXAudio2MasteringVoice* m_masteringVoice;
	IXAudio2SourceVoice* m_sourceVoice;
	WAVEFORMATEXTENSIBLE m_wfx;
	HANDLE m_eventNotify;
	uint32_t m_bufferSize;
	uint8_t* m_buffers[3];
	uint32_t m_nextSubmitBuffer;
};

	}
}

#endif	// traktor_sound_SoundDriverXAudio2_H
