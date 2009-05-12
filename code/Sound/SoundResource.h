#ifndef traktor_sound_SoundResource_H
#define traktor_sound_SoundResource_H

#include "Core/Serialization/Serializable.h"

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

/*! \brief Sound resource.
 * \ingroup Sound
 */
class T_DLLCLASS SoundResource : public Serializable
{
	T_RTTI_CLASS(SoundResource)

public:
	virtual bool serialize(Serializer& s);
};

	}
}

#endif	// traktor_sound_SoundResource_H
