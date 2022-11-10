/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Test/CaseThreadLocal.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadLocal.h"
#include "Core/Thread/ThreadManager.h"

namespace traktor::test
{
	namespace
	{

bool g_result = true;

ThreadLocal g_local;

void threadTest(int32_t number)
{
	intptr_t wn = (intptr_t)number;
	g_local.set((void*)wn);
	for (int32_t i = 0; i < 100; ++i)
	{
		ThreadManager::getInstance().getCurrentThread()->sleep(10);
		intptr_t check = (intptr_t)g_local.get();
		if (check != wn)
			g_result = false;
	}
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseThreadLocal", 0, CaseThreadLocal, Case)

void CaseThreadLocal::run()
{
	Thread* threads[16] = {};

	for (int32_t i = 0; i < 16; ++i)
		threads[i] = ThreadManager::getInstance().create([=](){ threadTest(i); }, L"Thread local test");
	for (int32_t i = 0; i < 16; ++i)
		threads[i]->start();
	for (int32_t i = 0; i < 16; ++i)
		threads[i]->wait();

	CASE_ASSERT(g_result);
}

}
