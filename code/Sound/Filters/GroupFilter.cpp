#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
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

	void* operator new (size_t size) {
		return getAllocator()->alloc(size, 16, T_FILE_LINE);
	}

	void operator delete (void* ptr) {
		getAllocator()->free(ptr);
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.GroupFilter", 0, GroupFilter, IFilter)

GroupFilter::GroupFilter()
{
}

GroupFilter::GroupFilter(IFilter* filter1)
{
	m_filters.resize(1);
	m_filters[0] = filter1;
}

GroupFilter::GroupFilter(IFilter* filter1, IFilter* filter2)
{
	m_filters.resize(2);
	m_filters[0] = filter1;
	m_filters[1] = filter2;
}

GroupFilter::GroupFilter(IFilter* filter1, IFilter* filter2, IFilter* filter3)
{
	m_filters.resize(3);
	m_filters[0] = filter1;
	m_filters[1] = filter2;
	m_filters[2] = filter3;
}

void GroupFilter::addFilter(IFilter* filter)
{
	m_filters.push_back(filter);
}

Ref< IFilterInstance > GroupFilter::createInstance() const
{
	Ref< GroupFilterInstance > gfi = new GroupFilterInstance();
	for (RefArray< IFilter >::const_iterator i = m_filters.begin(); i != m_filters.end(); ++i)
	{
		Ref< IFilterInstance > instance = (*i)->createInstance();
		gfi->m_instances.push_back(instance);
	}
	return gfi;
}

void GroupFilter::apply(IFilterInstance* instance, SoundBlock& outBlock) const
{
	GroupFilterInstance* gfi = static_cast< GroupFilterInstance* >(instance);
	uint32_t nfilters = uint32_t(m_filters.size());
	for (uint32_t i = 0; i < nfilters; ++i)
	{
		if (gfi->m_instances[i])
			m_filters[i]->apply(gfi->m_instances[i], outBlock);
	}
}

void GroupFilter::serialize(ISerializer& s)
{
	s >> MemberRefArray< IFilter >(L"filters", m_filters);
}

	}
}
