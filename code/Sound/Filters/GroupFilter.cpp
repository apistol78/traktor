#include "Sound/Filters/GroupFilter.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.GroupFilter", GroupFilter, Filter)

void GroupFilter::addFilter(Filter* filter)
{
	m_filters.push_back(filter);
}

void GroupFilter::apply(SoundBlock& outBlock)
{
	for (RefArray< Filter >::iterator i = m_filters.begin(); i != m_filters.end(); ++i)
		(*i)->apply(outBlock);
}

	}
}
