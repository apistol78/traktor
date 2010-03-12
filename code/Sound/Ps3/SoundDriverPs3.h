#ifndef traktor_sound_SoundDriverPs3_H
#define traktor_sound_SoundDriverPs3_H

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

class SoundMixerPs3;

/*!
 * \ingroup SoundPS3
 */
class T_DLLCLASS SoundDriverPs3 : public ISoundDriver
{
	T_RTTI_CLASS;

public:
	SoundDriverPs3();

	virtual ~SoundDriverPs3();

	virtual bool create(const SoundDriverCreateDesc& desc, Ref< ISoundMixer >& outMixer);

	virtual void destroy();

	virtual void wait();

	virtual void submit(const SoundBlock& soundBlock);

private:
	uint32_t m_port;
	uint8_t* m_blockPtr;
	uint64_t* m_readIndexPtr;
	uint64_t m_lastReadBlock;
	uint64_t m_writeBlock;
	uint32_t m_blockChannels;
	const int32_t* m_remap;
	Ref< SoundMixerPs3 > m_mixer;
};

	}
}

#endif	// traktor_sound_SoundDriverPs3_H
