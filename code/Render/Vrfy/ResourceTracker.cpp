/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Vrfy/ResourceTracker.h"

#include "Core/Debug/CallStack.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Acquire.h"

namespace traktor::render
{

void ResourceTracker::add(const Object* resource)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	auto& d = m_data[resource];
	d.callstackDepth = (int32_t)getCallStack(32, d.callstack, 2);
}

void ResourceTracker::remove(const Object* resource)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	auto it = m_data.find(resource);
	if (it != m_data.end())
		m_data.erase(it);
}

void ResourceTracker::snapshot()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_snapshot = m_data;
}

void ResourceTracker::alive(const TypeInfo& resourceType, bool diff) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	std::wstring symbol;
	std::wstring fileName;
	int32_t line;

	for (const auto& it : m_data)
	{
		if (!is_type_of(resourceType, type_of(it.first)))
			continue;

		if (diff)
		{
			// Check if this resource exist in snapshot; if so then
			// there is no difference.
			if (m_snapshot.find(it.first) != m_snapshot.end())
				continue;
		}

		log::info << L"Resource \"" << type_name(it.first) << L"\" (" << it.first->getReferenceCount() << L"):" << Endl;
		for (int32_t i = 0; i < it.second.callstackDepth; ++i)
		{
			getSymbolFromAddress(it.second.callstack[i], symbol);
			getSourceFromAddress(it.second.callstack[i], fileName, line);
			log::info << L"\t" << str(L"0x%016x", it.second.callstack[i]) << L"    " << symbol << L"    " << fileName << L"(" << line << L")" << Endl;
		}
	}
}

uint32_t ResourceTracker::count(const TypeInfo& resourceType) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	uint32_t cnt = 0;
	for (const auto& it : m_data)
	{
		if (is_type_of(resourceType, type_of(it.first)))
			++cnt;
	}
	return cnt;
}

}
