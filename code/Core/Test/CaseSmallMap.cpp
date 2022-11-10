/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <map>
#include "Core/Containers/SmallMap.h"
#include "Core/Math/Random.h"
#include "Core/Test/CaseSmallMap.h"

namespace traktor::test
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseSmallMap", 0, CaseSmallMap, Case)

void CaseSmallMap::run()
{
	const uint32_t count = 50000;

	std::map< uint32_t, uint32_t > m0;
	SmallMap< uint32_t, uint32_t > m1;

	Random rnd;
	for (uint32_t i = 0; i < count; ++i)
	{
		uint32_t key = uint32_t(rnd.nextFloat() * 65535);

		m0.insert(std::make_pair(key, i));
		m1.insert(std::make_pair(key, i));
	}

	uint32_t hit0 = 0, miss0 = 0;
	{
		rnd = Random();
		for (uint32_t i = 0; i < 10000000; ++i)
		{
			uint32_t key = uint32_t(rnd.nextFloat() * 65535);
			if (m0.find(key) != m0.end())
				++hit0;
			else
				++miss0;
		}
	}

	uint32_t hit1 = 0, miss1 = 0;
	{
		rnd = Random();
		for (uint32_t i = 0; i < 10000000; ++i)
		{
			uint32_t key = uint32_t(rnd.nextFloat() * 65535);
			if (m1.find(key) != m1.end())
				++hit1;
			else
				++miss1;
		}
	}

	CASE_ASSERT(hit0 == hit1);
	CASE_ASSERT(miss0 == miss1);
}

}
