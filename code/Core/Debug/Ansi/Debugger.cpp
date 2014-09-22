#if !defined(_WIN32)
#	include <cassert>
#endif
#if defined(__APPLE__)
#	include <stdbool.h>
#	include <sys/types.h>
#	include <unistd.h>
#	include <sys/sysctl.h>
#endif
#include <iostream>
#include "Core/Debug/Debugger.h"

namespace traktor
{

Debugger& Debugger::getInstance()
{
	static Debugger instance;
	return instance;
}

void Debugger::assertionFailed(const std::string& expression, const std::string& file, int line, const std::wstring& message)
{
	std::cerr << "ASSERT FAILED! " << expression << ", " << file << " (" << line << ")" << std::endl;
	if (!message.empty())
		std::wcerr << L"               " << message << std::endl;

#if defined(_WIN32)
	__debugbreak();
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
    T_ASSERT (junk == 0);

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

}
