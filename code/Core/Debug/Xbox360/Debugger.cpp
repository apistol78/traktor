/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <xtl.h>
#include "Core/Debug/Debugger.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/TString.h"

namespace traktor
{

Debugger& Debugger::getInstance()
{
	static Debugger instance;
	return instance;
}

void Debugger::assertionFailed(const char* const expression, const char* const file, int line, const wchar_t* const message)
{
	wchar_t* buttons[] = { L"Break", L"Ignore", L"Abort" };

	StringOutputStream ss;
	ss << mbstows(expression) << Endl;
	if (message)
		ss << message << Endl;
	ss << mbstows(file) << L" (" << line << L")" << Endl;

	HANDLE hOverlappedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	MESSAGEBOX_RESULT result;
	for (;;)
	{
		XOVERLAPPED overlapped;

		memset(&overlapped, 0, sizeof(overlapped));
		overlapped.hEvent = hOverlappedEvent;

		DWORD ret = XShowMessageBoxUI(
			0,
			L"Assert failed",
			ss.str().c_str(),
			3,
			(LPCWSTR*)buttons,
			0,
			XMB_ERRORICON,
			&result,
			&overlapped
		);
		if (ret != ERROR_SUCCESS)
			continue;

		WaitForSingleObject(hOverlappedEvent, INFINITE);
	}

	switch (result.dwButtonPressed)
	{
	case 0:
		DebugBreak();
		break;
	case 1:
		break;
	case 2:
		exit(0);
		break;
	}
}

bool Debugger::isDebuggerAttached() const
{
	return false;
}

void Debugger::breakDebugger()
{
	DebugBreak();
}

void Debugger::reportEvent(const wchar_t* const text, ...)
{
	static wchar_t buffer[1024];
	va_list arg;

	va_start(arg, text);
	vswprintf(buffer, text, arg);
	va_end(arg);

	PIXSetMarker(0xffffff, wstombs(buffer).c_str());
}

}
