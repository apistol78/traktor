#pragma once

#include <sys/event.h>
#include "Sound/IAudioDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_PS3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*!
 * \ingroup SoundPS3
 */
class T_DLLCLASS AudioDriverPs3 : public IAudioDriver
{
	T_RTTI_CLASS;

public:
	AudioDriverPs3();

	virtual ~AudioDriverPs3();

	virtual bool create(const SystemApplication& sysapp, const AudioDriverCreateDesc& desc, Ref< IAudioMixer >& outMixer);

	virtual void destroy();

	virtual void wait();

	virtual void submit(const SoundBlock& soundBlock);

private:
	uint32_t m_port;
	sys_event_queue_t m_eventQueue;
	uint64_t m_eventQueueKey;
	uint8_t* m_blockPtr;
	volatile uint64_t* m_readIndexPtr;
	volatile uint64_t m_writeCount;
	uint32_t m_blockChannels;
	uint32_t m_blocksPerFrame;
};

	}
}

