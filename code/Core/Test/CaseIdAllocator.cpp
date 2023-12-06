/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/IdAllocator.h"
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
		dump(id);
		uint32_t b = id.allocSequential(4);
		dump(id);
		uint32_t c = id.allocSequential(4);
		dump(id);
		id.freeSequential(b, 4);
		dump(id);
		uint32_t d = id.allocSequential(4);
		dump(id);
		uint32_t e = id.allocSequential(4);
		dump(id);
		CASE_ASSERT_EQUAL(a, 10);
		CASE_ASSERT_EQUAL(b, 14);
		CASE_ASSERT_EQUAL(c, 18);
		CASE_ASSERT_EQUAL(d, 14);
		CASE_ASSERT_EQUAL(e, 22);		
	}	
}

void CaseIdAllocator::dump(const IdAllocator& id) const
{
	log::info << L"--------------" << Endl;
	for (auto p : id.m_free)
		log::info << p.left << L" - " << p.right << Endl;
	log::info << L"--------------" << Endl;
}

}
