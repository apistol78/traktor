#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/Filters/GroupFilter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct GroupFilterInstance : public RefCountImpl< IFilterInstance >
{
	RefArray< IFilterInstance > m_instances;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.GroupFilter", 0, GroupFilter, IFilter)

void GroupFilter::addFilter(IFilter* filter)
{
	m_filters.push_back(filter);
}

Ref< IFilterInstance > GroupFilter::createInstance() const
{
	Ref< GroupFilterInstance > gfi = new GroupFilterInstance();
	for (RefArray< IFilter >::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
		gfi->m_instances.push_back((*i)->createInstance());
	return gfi;
}

void GroupFilter::apply(const ISoundMixer* mixer, IFilterInstance* instance, SoundBlock& outBlock) const
{
	GroupFilterInstance* gfi = static_cast< GroupFilterInstance* >(instance);
	uint32_t nfilters = m_filters.size();
	for (uint32_t i = 0; i < nfilters; ++i)
		m_filters[i]->apply(mixer, gfi->m_instances[i], outBlock);
}

bool GroupFilter::serialize(ISerializer& s)
{
	return s >> MemberRefArray< IFilter >(L"filters", m_filters);
}

	}
}
