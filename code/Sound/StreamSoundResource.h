#ifndef traktor_sound_StreamSoundResource_H
#define traktor_sound_StreamSoundResource_H

#include "Sound/SoundResource.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

/*! \brief Stream sound resource.
 * \ingroup Sound
 */
class T_DLLCLASS StreamSoundResource : public SoundResource
{
	T_RTTI_CLASS(StreamSoundResource)

public:
	StreamSoundResource(const Type* decoderType = 0);

	const Type* getDecoderType() const;

	virtual bool serialize(Serializer& s);

private:
	const Type* m_decoderType;
};

	}
}

#endif	// traktor_sound_StreamSoundResource_H
