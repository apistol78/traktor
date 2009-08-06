#include <iostream>
#include "Core/Platform.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"

namespace traktor
{

LogTargetConsole::LogTargetConsole(const std::wstring& prefix)
:	m_prefix(prefix)
{
}

void LogTargetConsole::log(const std::wstring& str)
{
	std::wstringstream ss; ss << m_prefix << str << std::endl;
#if !defined(WINCE) && !defined(_XBOX)
	std::wcout << ss.str() << std::flush;
#endif
#if defined(_WIN32)
	OutputDebugString(wstots(ss.str()).c_str());
#endif
}

LogTargetDebug::LogTargetDebug(const std::wstring& prefix)
:	m_prefix(prefix)
{
}

void LogTargetDebug::log(const std::wstring& str)
{
#if defined(_WIN32)
	std::wstringstream ss;
	ss << m_prefix << L"(" << GetCurrentProcessId() << L") " << str << std::endl;
	OutputDebugString(wstots(ss.str()).c_str());
#endif
}

LogStreamBuffer::LogStreamBuffer(LogTarget* target)
:	m_target(target)
{
}

LogTarget* LogStreamBuffer::getTarget()
{
	return m_target;
}

void LogStreamBuffer::setTarget(LogTarget* target)
{
	m_target = target;
}

int LogStreamBuffer::overflow(const wchar_t* buffer, int count)
{
	for (int i = 0; i < count; ++i)
	{
		wchar_t c = buffer[i];
#if defined(_WIN32)
		if (c == L'\r')
			continue;
#endif
#if !defined(__APPLE__)
		if (c == L'\n')
#else
		if (c == L'\r')
#endif
		{
			if (m_target)
				m_target->log(m_ss.str());
			m_ss.str(L"");
		}
		else
		{
			m_ss << c;
		}
	}
	return count;
}

LogStream::LogStream(LogTarget* target)
:	m_streamBuffer(target)
,	OutputStream(&m_streamBuffer)
{
}

LogStream::~LogStream()
{
	T_EXCEPTION_GUARD_BEGIN

	setBuffer(0);

	T_EXCEPTION_GUARD_END
}

LogStreamBuffer& LogStream::getBuffer()
{
	return m_streamBuffer;
}

	namespace log
	{

static LogTargetConsole infoTarget(L"Info    : ");
static LogTargetConsole warningTarget(L"Warning : ");
static LogTargetConsole errorTarget(L"Error   : ");
static LogTargetDebug debugTarget(L"Debug   : ");

LogStream info(&infoTarget);
LogStream warning(&warningTarget);
LogStream error(&errorTarget);
LogStream debug(&debugTarget);

	}
}
