#include <iostream>
#include "Core/Platform.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"

namespace traktor
{

LogTargetConsole::LogTargetConsole(int color)
:	m_color(color)
,	m_defaultColorAttribs(0)
{
#if defined(_WIN32) && !defined(WINCE) && !defined(_XBOX)
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hStdOut, &csbiInfo);
	m_defaultColorAttribs = csbiInfo.wAttributes; 
#endif
}

void LogTargetConsole::log(const std::wstring& str)
{
#if defined(_WIN32)
	std::wstringstream ss;
	ss << str << std::endl;

	tstring tss = wstots(ss.str());

#	if !defined(WINCE) && !defined(_XBOX)

	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	switch (m_color)
	{
	case 1:
		SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		break;

	case 2:
		SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_INTENSITY);
	}
	
	WriteConsole(
		hStdOut,
		tss.c_str(),
		tss.length(),
		NULL,
		NULL
	);

	SetConsoleTextAttribute(hStdOut, m_defaultColorAttribs);

#	endif

	OutputDebugString(tss.c_str());
#else
	std::wcout << str << std::endl;
#endif
}

void LogTargetDebug::log(const std::wstring& str)
{
#if defined(_WIN32)
	std::wstringstream ss;
	ss << L"(" << GetCurrentProcessId() << L") " << str << std::endl;
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
:	m_streamBuffer(new LogStreamBuffer(target))
,	OutputStream(0, LeUnix)
{
	setBuffer(m_streamBuffer);
}

LogStream::~LogStream()
{
	T_EXCEPTION_GUARD_BEGIN

	setBuffer(0);

	T_EXCEPTION_GUARD_END
}

Ref< LogStreamBuffer > LogStream::getBuffer()
{
	return m_streamBuffer;
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
