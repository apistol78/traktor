#include <iostream>
#include "Core/Platform.h"
#include "Core/Log/Log.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/TString.h"

namespace traktor
{

LogTargetConsole::LogTargetConsole(int color)
:	m_color(color)
{
}

void LogTargetConsole::log(const std::wstring& str)
{
#if !defined(WINCE)
	if (m_color == 0)
		std::wcout << str << std::endl;
	else if (m_color == 1)
		std::wcout << L"(WARN) " << str << std::endl;
	else
		std::wcerr << L"(ERROR) " << str << std::endl;
#endif
#if defined(_WIN32)
	tstring tss = wstots(str + L"\n");
	OutputDebugString(tss.c_str());
#endif
}

void LogTargetDebug::log(const std::wstring& str)
{
#if defined(_WIN32)
	StringOutputStream ss;
	ss << L"(" << uint32_t(GetCurrentThreadId()) << L") " << str << Endl;
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
	StringOutputStream* ss = static_cast< StringOutputStream* >(m_buffers.get());
	if (!ss)
	{
		ss = new StringOutputStream();
		m_buffers.set(ss);
	}

	for (int i = 0; i < count; ++i)
	{
		wchar_t c = buffer[i];
		if (c == L'\n')
		{
			if (m_target)
				m_target->log(ss->str());
			ss->reset();
		}
		else if (c != L'\r')
			(*ss) << c;
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

ILogTarget* LogStream::getTarget()
{
	return checked_type_cast< LogStreamBuffer* >(getBuffer())->getTarget();
}

void LogStream::setTarget(ILogTarget* target)
{
	checked_type_cast< LogStreamBuffer* >(getBuffer())->setTarget(target);
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
