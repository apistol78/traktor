#pragma once

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "Core/Misc/AutoPtr.h"
#include "Core/Thread/Event.h"
#include "Sound/IAudioDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_OPENSL_ES_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*!
 * \ingroup OpenSL
 */
class T_DLLCLASS AudioDriverOpenSL : public IAudioDriver
{
	T_RTTI_CLASS;

public:
	AudioDriverOpenSL();

	virtual bool create(const SystemApplication& sysapp, const AudioDriverCreateDesc& desc, Ref< IAudioMixer >& outMixer) override final;

	virtual void destroy() override final;

	virtual void wait() override final;

	virtual void submit(const SoundBlock& soundBlock) override final;

private:
	AudioDriverCreateDesc m_desc;
	SLObjectItf m_engineObject;
	SLEngineItf m_engineEngine;
	SLObjectItf m_outputMixObject;
	SLObjectItf m_playerObject;
	SLPlayItf m_playerPlay;
	SLAndroidSimpleBufferQueueItf m_playerBufferQueue;
	Event m_eventQueue;
	AutoArrayPtr< int16_t > m_queueBuffer;

	static void queueCallback(
		SLAndroidSimpleBufferQueueItf caller,
		void *pContext
	);
};

	}
}

