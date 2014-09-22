#include <cassert>
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

	assert (0);
}

bool Debugger::isDebuggerAttached() const
{
	return false;
}

void Debugger::breakDebugger()
{
	assert (0);
}

}
