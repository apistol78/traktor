#ifndef traktor_sound_ISoundResource_H
#define traktor_sound_ISoundResource_H

#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace sound
	{

class Sound;

/*! \brief Sound resource.
 * \ingroup Sound
 */
class T_DLLCLASS ISoundResource : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Ref< Sound > createSound(resource::IResourceManager* resourceManager, const db::Instance* resourceInstance) const = 0;
};

	}
}

#endif	// traktor_sound_ISoundResource_H
