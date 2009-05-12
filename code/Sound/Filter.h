#ifndef traktor_sound_Filter_H
#define traktor_sound_Filter_H

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

/*! \brief Filter base class.
 * \ingroup Sound
 */
class T_DLLCLASS Filter : public Object
{
	T_RTTI_CLASS(Filter)

public:
	virtual void apply(SoundBlock& outBlock) = 0;
};

	}
}

#endif	// traktor_sound_Filter_H
