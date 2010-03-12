#ifndef traktor_sound_IGrain_H
#define traktor_sound_IGrain_H

#include "Core/Serialization/ISerializable.h"
#include "Sound/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace sound
	{

class ISoundBufferCursor;
class ISoundMixer;

/*! \brief
 * \ingroup Sound
 */
class T_DLLCLASS IGrain : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool bind(resource::IResourceManager* resourceManager) = 0;

	virtual Ref< ISoundBufferCursor > createCursor() const = 0;

	virtual void updateCursor(ISoundBufferCursor* cursor) const = 0;

	virtual const IGrain* getCurrentGrain(ISoundBufferCursor* cursor) const = 0;

	virtual bool getBlock(const ISoundMixer* mixer, ISoundBufferCursor* cursor, SoundBlock& outBlock) const = 0;
};

	}
}

#endif	// traktor_sound_IGrain_H
