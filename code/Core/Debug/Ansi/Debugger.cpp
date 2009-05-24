#if !defined(_WIN32)
#include <cassert>
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
#elif defined(__APPLE__)
#	if __ppc64__ || __ppc__
	asm("li r0, 20\nsc\nnop\nli r0, 37\nli r4, 2\nsc\nnop\n" : : : "memory","r0","r3","r4");
#	else
	asm("int 3\n" : : );
#	endif
#else
	// Fall back on assert macro
	assert (0);
#endif
}

void Debugger::breakDebugger()
{
#if defined(_WIN32)
	__debugbreak();
#elif defined(__APPLE__)
#	if __ppc64__ || __ppc__
	asm("li r0, 20\nsc\nnop\nli r0, 37\nli r4, 2\nsc\nnop\n" : : : "memory","r0","r3","r4");
#	else
	asm("int 3\n" : : );
#	endif
#else
	// Fall back on assert macro
	assert (0);
#endif
}

}
