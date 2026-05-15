/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#if !defined(_WIN32)
#	include <cassert>
#endif
#if defined(__LINUX__) || defined(__RPI__)
#	include <signal.h>
#elif defined(__APPLE__)
#	include <stdbool.h>
#	include <sys/types.h>
#	include <unistd.h>
#	include <sys/sysctl.h>
#endif
#include <iostream>

#include "Core/Debug/CallStack.h"
#include "Core/Debug/Debugger.h"
#include "Core/Misc/String.h"

namespace traktor
{

Debugger& Debugger::getInstance()
{
	static Debugger instance;
	return instance;
}

void Debugger::assertionFailed(const char* const expression, const char* const file, int line, const wchar_t* const message)
{
	std::cerr << "ASSERT FAILED! " << expression << ", " << file << " (" << line << ")" << std::endl;
	if (message != nullptr && *message != '\0')
		std::wcerr << L"               " << message << std::endl;

#if defined(__LINUX__) || defined(__RPI__)
	void* at[16];
	const uint32_t depth = getCallStack(sizeof_array(at), at, 2);
	for (uint32_t i = 0; i < depth; ++i)
	{
		std::wstring source;
		int32_t line;

		if (getSourceFromAddress(at[i], source, line))
			std::wcerr << str(L"0x%016x : %S (%d)", (intptr_t)at[i], source.c_str(), line).c_str() << std::endl;
		else
			std::wcerr << str(L"0x%016x", (intptr_t)at[i]).c_str() << std::endl;
	}
#endif

#if defined(_WIN32)
	__debugbreak();
#elif defined(__LINUX__) || defined(__RPI__)
	raise(SIGTRAP);
#else
	// Fall back on assert macro
	assert (0);
#endif
}

bool Debugger::isDebuggerAttached() const
{
#if defined(__APPLE__)
	int junk;
    int mib[4];
    struct kinfo_proc info;
    size_t size;

    info.kp_proc.p_flag = 0;

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    size = sizeof(info);
    junk = sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
    T_ASSERT(junk == 0);

    return ((info.kp_proc.p_flag & P_TRACED) != 0);
#else
	return false;
#endif
}

void Debugger::breakDebugger()
{
#if defined(_WIN32)
	__debugbreak();
#else
	// Fall back on assert macro
	assert (0);
#endif
}

void Debugger::reportEvent(const wchar_t* const text, ...)
{
}

}
