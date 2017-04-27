/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <map>
#include "Core/Containers/SmallMap.h"
#include "Core/Log/Log.h"
#include "Core/Math/Random.h"
#include "Core/Timer/Timer.h"
#include "UnitTest/CaseSmallMap.h"

namespace traktor
{

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

	Timer t0;
	t0.start();

	{
		uint32_t hit = 0, miss = 0;
		for (uint32_t i = 0; i < 10000000; ++i)
		{
			uint32_t key = uint32_t(rnd.nextFloat() * 65535);
			if (m0.find(key) != m0.end())
				++hit;
			else
				++miss;
		}
		log::info << hit << L"/" << miss << Endl;
	}

	t0.stop();

	Timer t1;
	t1.start();

	{
		uint32_t hit = 0, miss = 0;
		for (uint32_t i = 0; i < 10000000; ++i)
		{
			uint32_t key = uint32_t(rnd.nextFloat() * 65535);
			if (m1.find(key) != m1.end())
				++hit;
			else
				++miss;
		}
		log::info << hit << L"/" << miss << Endl;
	}

	t1.stop();

	log::info << L"std::map " << uint32_t(t0.getElapsedTime() * 1000.0f) << L" ms" << Endl;
	log::info << L"SmallMap " << uint32_t(t1.getElapsedTime() * 1000.0f) << L" ms" << Endl;
}

}
