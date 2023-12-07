/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/IdAllocator.h"
#include "Core/Containers/SmallSet.h"
#include "Core/Log/Log.h"
#include "Core/Test/CaseIdAllocator.h"

namespace traktor::test
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseIdAllocator", 0, CaseIdAllocator, Case)

void CaseIdAllocator::run()
{
	{
		IdAllocator id(10, 20);
		uint32_t a = id.alloc();
		uint32_t b = id.alloc();
		uint32_t c = id.alloc();
		id.free(b);
		uint32_t d = id.alloc();
		uint32_t e = id.alloc();
		CASE_ASSERT_EQUAL(a, 10);
		CASE_ASSERT_EQUAL(b, 11);
		CASE_ASSERT_EQUAL(c, 12);
		CASE_ASSERT_EQUAL(d, 11);
		CASE_ASSERT_EQUAL(e, 13);
		id.free(a);
		id.free(c);
		id.free(d);
		id.free(e);
		CASE_ASSERT(id.m_free.size() == 1);
		CASE_ASSERT(*id.m_free.begin() == IdAllocator::Interval(10, 20));
	}
	{
		IdAllocator id(10, 30);
		uint32_t a = id.allocSequential(4);
		uint32_t b = id.allocSequential(4);
		uint32_t c = id.allocSequential(4);
		id.freeSequential(b, 4);
		uint32_t d = id.allocSequential(4);
		uint32_t e = id.allocSequential(4);
		CASE_ASSERT_EQUAL(a, 10);
		CASE_ASSERT_EQUAL(b, 14);
		CASE_ASSERT_EQUAL(c, 18);
		CASE_ASSERT_EQUAL(d, 14);
		CASE_ASSERT_EQUAL(e, 22);
		id.freeSequential(a, 4);
		id.freeSequential(c, 4);
		id.freeSequential(d, 4);
		id.freeSequential(e, 4);
		CASE_ASSERT(id.m_free.size() == 1);
		CASE_ASSERT(*id.m_free.begin() == IdAllocator::Interval(10, 30));
	}
	{
		IdAllocator id(10, 30);

		SmallSet< uint32_t > ids;
		for (uint32_t i = 0; i < 10000; ++i)
		{
			const uint32_t a = id.alloc();
			CASE_ASSERT(a != ~0U);

			CASE_ASSERT(ids.find(a) == ids.end());
			ids.insert(a);

			if (ids.size() > 10)
			{
				const uint32_t b = ids[rand() % ids.size()];
				ids.erase(b);
				id.free(b);
			}
		}
		for (auto a : ids)
			id.free(a);

		CASE_ASSERT(id.m_free.size() == 1);
		CASE_ASSERT(*id.m_free.begin() == IdAllocator::Interval(10, 30));
	}
	{
		IdAllocator id(10, 100000);

		SmallSet< std::pair< uint32_t, uint32_t > > ids;
		for (uint32_t i = 0; i < 10000; ++i)
		{
			const uint32_t span = (rand() % 10) + 1;
			const uint32_t a = id.allocSequential(span);
			CASE_ASSERT(a != ~0U);

			const auto k = std::make_pair(a, span);
			CASE_ASSERT(ids.find(k) == ids.end());
			ids.insert(k);

			if (ids.size() > 10)
			{
				const auto b = ids[rand() % ids.size()];
				ids.erase(b);
				id.freeSequential(b.first, b.second);
			}
		}
		for (auto a : ids)
			id.freeSequential(a.first, a.second);

		CASE_ASSERT(id.m_free.size() == 1);
		CASE_ASSERT(*id.m_free.begin() == IdAllocator::Interval(10, 100000));
	}
}

}
