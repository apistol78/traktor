#ifndef traktor_sound_SoundDriverPs3_H
#define traktor_sound_SoundDriverPs3_H

#include <sys/event.h>
#include "Sound/ISoundDriver.h"

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
class T_DLLCLASS SoundDriverPs3 : public ISoundDriver
{
	T_RTTI_CLASS;

public:
	SoundDriverPs3();

	virtual ~SoundDriverPs3();

	virtual bool create(void* nativeHandle, const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer);

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

#endif	// traktor_sound_SoundDriverPs3_H
