#ifndef traktor_sound_IGrain_H
#define traktor_sound_IGrain_H

#include "Core/Object.h"
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
	namespace sound
	{

class ISoundBufferCursor;
class ISoundMixer;

/*! \brief
 * \ingroup Sound
 */
class T_DLLCLASS IGrain : public Object
{
	T_RTTI_CLASS;

public:
	virtual Ref< ISoundBufferCursor > createCursor() const = 0;

	virtual void updateCursor(ISoundBufferCursor* cursor) const = 0;

	virtual const IGrain* getCurrentGrain(const ISoundBufferCursor* cursor) const = 0;

	virtual bool getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const = 0;
};

	}
}

#endif	// traktor_sound_IGrain_H
