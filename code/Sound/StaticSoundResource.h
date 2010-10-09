#ifndef traktor_sound_StaticSoundResource_H
#define traktor_sound_StaticSoundResource_H

#include "Sound/ISoundResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*! \brief Static sound resource.
 * \ingroup Sound
 */
class T_DLLCLASS StaticSoundResource : public ISoundResource
{
	T_RTTI_CLASS;

public:
	StaticSoundResource();

	virtual Ref< Sound > createSound(resource::IResourceManager* resourceManager, db::Instance* resourceInstance) const;

	virtual bool serialize(ISerializer& s);
	
	uint32_t getSampleRate() const { return m_sampleRate; }
	
	uint32_t getSamplesCount() const { return m_samplesCount; }
	
	uint32_t getChannelsCount() const { return m_channelsCount; }
	
	uint32_t getFlags() const { return m_flags; }
	
private:
	friend class SoundPipeline;

	uint32_t m_sampleRate;
	uint32_t m_samplesCount;
	uint32_t m_channelsCount;
	uint32_t m_flags;
};

	}
}

#endif	// traktor_sound_StaticSoundResource_H
