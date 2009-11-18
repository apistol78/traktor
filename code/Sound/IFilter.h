#ifndef traktor_sound_IFilter_H
#define traktor_sound_IFilter_H

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

/*! \brief IFilter base class.
 * \ingroup Sound
 */
class T_DLLCLASS IFilter : public Object
{
	T_RTTI_CLASS;

public:
	virtual void apply(SoundBlock& outBlock) = 0;
};

	}
}

#endif	// traktor_sound_IFilter_H
