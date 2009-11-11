#ifndef traktor_sound_GroupFilter_H
#define traktor_sound_GroupFilter_H

#include "Core/Heap/RefArray.h"
#include "Sound/IFilter.h"

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
class T_DLLCLASS GroupFilter : public IFilter
{
	T_RTTI_CLASS(GroupFilter)

public:
	void addFilter(IFilter* filter);

	virtual void apply(SoundBlock& outBlock);

private:
	RefArray< IFilter > m_filters;
};

	}
}

#endif	// traktor_sound_GroupFilter_H
