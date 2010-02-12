#include <iostream>
#include "Core/Platform.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"

namespace traktor
{

LogTargetConsole::LogTargetConsole(int color)
:	m_color(color)
{
}

void LogTargetConsole::log(const std::wstring& str)
{
	if (m_color == 0)
		std::wcout << str << std::endl;
	else if (m_color == 1)
		std::wcout << L"(WARN) " << str << std::endl;
	else
		std::wcerr << L"(ERROR) " << str << std::endl;
#if defined(_WIN32)
	tstring tss = wstots(str + L"\n");
	OutputDebugString(tss.c_str());
#endif
}

void LogTargetDebug::log(const std::wstring& str)
{
#if defined(_WIN32)
	std::wstringstream ss;
	ss << L"(" << GetCurrentThreadId() << L") " << str << std::endl;
	OutputDebugString(wstots(ss.str()).c_str());
#else
	std::wcout << L"(DEBUG) " << str << std::endl;
#endif
}

LogStreamBuffer::LogStreamBuffer(ILogTarget* target)
:	m_target(target)
{
}

ILogTarget* LogStreamBuffer::getTarget()
{
	return m_target;
}

void LogStreamBuffer::setTarget(ILogTarget* target)
{
	m_target = target;
}

int LogStreamBuffer::overflow(const wchar_t* buffer, int count)
{
	for (int i = 0; i < count; ++i)
	{
		wchar_t c = buffer[i];
		if (c == L'\n')
		{
			if (m_target)
				m_target->log(m_ss.str());
			m_ss.str(L"");
		}
		else if (c != L'\r')
			m_ss << c;
	}
	return count;
}

LogStream::LogStream(ILogTarget* target)
:	OutputStream(new LogStreamBuffer(target), LeUnix)
{
}

LogStream::~LogStream()
{
	T_EXCEPTION_GUARD_BEGIN

	setBuffer(0);

	T_EXCEPTION_GUARD_END
}

	namespace log
	{

static LogTargetConsole infoTarget(0);
static LogTargetConsole warningTarget(1);
static LogTargetConsole errorTarget(2);
static LogTargetDebug debugTarget;

LogStream info(&infoTarget);
LogStream warning(&warningTarget);
LogStream error(&errorTarget);
LogStream debug(&debugTarget);

	}
}
