#include <cstdlib>
#include <iostream>
#include "Core/Debug/Debugger.h"

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
	if (message)
		std::wcerr << L"               " << message << std::endl;

	std::abort();
}

bool Debugger::isDebuggerAttached() const
{
	return false;
}

void Debugger::breakDebugger()
{
	std::abort();
}

void Debugger::reportEvent(const wchar_t* const text, ...)
{
}

}
