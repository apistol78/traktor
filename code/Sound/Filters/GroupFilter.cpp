/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Memory/IAllocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/Filters/GroupFilter.h"

namespace traktor::sound
{
	namespace
	{

struct GroupFilterInstance : public RefCountImpl< IAudioFilterInstance >
{
	RefArray< IAudioFilterInstance > m_instances;

	void* operator new (size_t size) {
		return getAllocator()->alloc(size, 16, T_FILE_LINE);
	}

	void operator delete (void* ptr) {
		getAllocator()->free(ptr);
	}
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.GroupFilter", 0, GroupFilter, IAudioFilter)

GroupFilter::GroupFilter(IAudioFilter* filter1)
{
	m_filters.resize(1);
	m_filters[0] = filter1;
}

GroupFilter::GroupFilter(IAudioFilter* filter1, IAudioFilter* filter2)
{
	m_filters.resize(2);
	m_filters[0] = filter1;
	m_filters[1] = filter2;
}

GroupFilter::GroupFilter(IAudioFilter* filter1, IAudioFilter* filter2, IAudioFilter* filter3)
{
	m_filters.resize(3);
	m_filters[0] = filter1;
	m_filters[1] = filter2;
	m_filters[2] = filter3;
}

void GroupFilter::addFilter(IAudioFilter* filter)
{
	m_filters.push_back(filter);
}

Ref< IAudioFilterInstance > GroupFilter::createInstance() const
{
	Ref< GroupFilterInstance > gfi = new GroupFilterInstance();
	for (auto filter : m_filters)
	{
		Ref< IAudioFilterInstance > instance = filter->createInstance();
		gfi->m_instances.push_back(instance);
	}
	return gfi;
}

void GroupFilter::apply(IAudioFilterInstance* instance, AudioBlock& outBlock) const
{
	GroupFilterInstance* gfi = static_cast< GroupFilterInstance* >(instance);
	const uint32_t nfilters = uint32_t(m_filters.size());
	for (uint32_t i = 0; i < nfilters; ++i)
	{
		if (gfi->m_instances[i])
			m_filters[i]->apply(gfi->m_instances[i], outBlock);
	}
}

void GroupFilter::serialize(ISerializer& s)
{
	s >> MemberRefArray< IAudioFilter >(L"filters", m_filters);
}

}
