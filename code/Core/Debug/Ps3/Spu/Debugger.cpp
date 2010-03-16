#include <cassert>
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
	assert (0);
}

void Debugger::breakDebugger()
{
	assert (0);
}

}
