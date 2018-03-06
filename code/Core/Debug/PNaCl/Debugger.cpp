/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

void Debugger::assertionFailed(const char* const expression, const char* const file, int line, const wchar_t* const message)
{
	std::cerr << "ASSERT FAILED! " << expression << ", " << file << " (" << line << ")" << std::endl;
	if (message)
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

void Debugger::reportEvent(const wchar_t* const text, ...)
{
}

}
