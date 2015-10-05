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

	virtual Ref< Sound > createSound(resource::IResourceManager* resourceManager, db::Instance* resourceInstance) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;
	
private:
	friend class SoundPipeline;

	std::wstring m_category;
	uint32_t m_sampleRate;
	uint32_t m_samplesCount;
	uint32_t m_channelsCount;
	float m_volume;
	float m_presence;
	float m_presenceRate;
	float m_range;
	const TypeInfo* m_decoderType;
};

	}
}

#endif	// traktor_sound_StaticSoundResource_H
