#ifndef traktor_sound_SoundDriverOpenSL_H
#define traktor_sound_SoundDriverOpenSL_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "Core/Misc/AutoPtr.h"
#include "Core/Thread/Event.h"
#include "Sound/ISoundDriver.h"

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
class T_DLLCLASS SoundDriverOpenSL : public ISoundDriver
{
	T_RTTI_CLASS;

public:
	SoundDriverOpenSL();

	virtual bool create(const SystemApplication& sysapp, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void wait() T_OVERRIDE T_FINAL;

	virtual void submit(const SoundBlock& soundBlock) T_OVERRIDE T_FINAL;

private:
	SoundDriverCreateDesc m_desc;
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

#endif	// traktor_sound_SoundDriverOpenSL_H
