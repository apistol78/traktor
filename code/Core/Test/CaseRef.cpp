/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/RefArray.h"
#include "Core/Test/CaseRef.h"

namespace traktor::test
{

int32_t g_totalRefs = 0;

class A : public Object
{
public:
	virtual void addRef(void* owner) const noexcept override
	{
		Object::addRef(owner);
		g_totalRefs++;
	}

	virtual void release(void* owner) const noexcept override
	{
		g_totalRefs--;
		Object::release(owner);
	}

};

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseRef", 0, CaseRef, Case)

void CaseRef::run()
{
	{
		RefArray< A > ra;
		for (int i = 0; i < 10; ++i)
			ra.push_back(new A());
		CASE_ASSERT_EQUAL((int32_t)ra.size(), 10);
		CASE_ASSERT_EQUAL(g_totalRefs, 10);

		int c = 0;
		for (auto r : ra)
		{
			CASE_ASSERT_EQUAL(r->getReferenceCount(), 1);
			++c;
		}
		CASE_ASSERT_EQUAL(c, 10);
		CASE_ASSERT_EQUAL(g_totalRefs, 10);
	}
	CASE_ASSERT_EQUAL(g_totalRefs, 0);
}

}
