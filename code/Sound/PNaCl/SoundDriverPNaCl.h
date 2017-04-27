/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_SoundDriverPNaCl_H
#define traktor_sound_SoundDriverPNaCl_H

#include <ppapi/cpp/audio.h>
#include "Core/Thread/Event.h"
#include "Sound/ISoundDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_PNACL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*!
 * \ingroup PNaCl
 */
class T_DLLCLASS SoundDriverPNaCl : public ISoundDriver
{
	T_RTTI_CLASS;

public:
	SoundDriverPNaCl();

	virtual bool create(void* nativeHandle, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer);

	virtual void destroy();

	virtual void wait();

	virtual void submit(const SoundBlock& soundBlock);

private:
	SoundDriverCreateDesc m_desc;
	pp::Audio m_audio;
	Event m_eventPending;
	Event m_eventReady;
	const SoundBlock* m_pendingSoundBlock;

	static void streamCallback(void* samples, uint32_t bufferSize, void* data);
};

	}
}

#endif	// traktor_sound_SoundDriverPNaCl_H
