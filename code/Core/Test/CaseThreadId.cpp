/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <set>
#include "Core/Log/Log.h"
#include "Core/Test/CaseThreadId.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"

namespace traktor::test
{
	namespace
	{

int32_t g_ids[16];

void threadTest(int32_t number)
{
	int32_t id = ThreadManager::getInstance().getCurrentThread()->id();
	g_ids[number] = id;
	//log::info << number << L", id " << id << Endl;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseThreadId", 0, CaseThreadId, Case)

void CaseThreadId::run()
{
	Thread* threads[16] = {};

	for (int32_t i = 0; i < 16; ++i)
		g_ids[i] = -1;
	for (int32_t i = 0; i < 16; ++i)
		threads[i] = ThreadManager::getInstance().create([=](){ threadTest(i); }, L"Thread Id test");
	for (int32_t i = 0; i < 16; ++i)
		threads[i]->start();
	for (int32_t i = 0; i < 16; ++i)
		threads[i]->wait();
	for (int32_t i = 0; i < 16; ++i)
		ThreadManager::getInstance().destroy(threads[i]);

	std::set< int32_t > unique;
	for (int32_t i = 0; i < 16; ++i)
		unique.insert(g_ids[i]);

	CASE_ASSERT_EQUAL((int32_t)unique.size(), 16);
	CASE_ASSERT(unique.find(-1) == unique.end());
}

}
