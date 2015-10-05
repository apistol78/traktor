#ifndef traktor_sound_StreamSoundResource_H
#define traktor_sound_StreamSoundResource_H

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

/*! \brief Stream sound resource.
 * \ingroup Sound
 */
class T_DLLCLASS StreamSoundResource : public ISoundResource
{
	T_RTTI_CLASS;

public:
	StreamSoundResource();

	virtual Ref< Sound > createSound(resource::IResourceManager* resourceManager, db::Instance* resourceInstance) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	friend class SoundPipeline;

	const TypeInfo* m_decoderType;
	std::wstring m_category;
	float m_volume;
	float m_presence;
	float m_presenceRate;
	float m_range;
	bool m_preload;
};

	}
}

#endif	// traktor_sound_StreamSoundResource_H
