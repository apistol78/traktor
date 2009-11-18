#ifndef traktor_sound_ISoundBuffer_H
#define traktor_sound_ISoundBuffer_H

#include "Core/Object.h"
#include "Sound/Types.h"

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

/*! \brief Sound buffer base class.
 * \ingroup Sound
 */
class T_DLLCLASS ISoundBuffer : public Object
{
	T_RTTI_CLASS;

public:
	virtual double getDuration() const = 0;

	virtual bool getBlock(double time, SoundBlock& outBlock) = 0;
};

	}
}

#endif	// traktor_sound_ISoundBuffer_H
