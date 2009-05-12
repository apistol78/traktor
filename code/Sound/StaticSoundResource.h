#ifndef traktor_sound_StaticSoundResource_H
#define traktor_sound_StaticSoundResource_H

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

/*! \brief Static sound resource.
 * \ingroup Sound
 */
class T_DLLCLASS StaticSoundResource : public SoundResource
{
	T_RTTI_CLASS(StaticSoundResource)
};

	}
}

#endif	// traktor_sound_StaticSoundResource_H
