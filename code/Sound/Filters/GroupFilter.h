#ifndef traktor_sound_GroupFilter_H
#define traktor_sound_GroupFilter_H

#include "Core/Heap/Ref.h"
#include "Sound/Filter.h"

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

/*! \brief Group filter.
 * \ingroup Sound
 */
class T_DLLCLASS GroupFilter : public Filter
{
	T_RTTI_CLASS(GroupFilter)

public:
	void addFilter(Filter* filter);

	virtual void apply(SoundBlock& outBlock);

private:
	RefArray< Filter > m_filters;
};

	}
}

#endif	// traktor_sound_GroupFilter_H
