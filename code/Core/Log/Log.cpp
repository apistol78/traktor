#if defined(__ANDROID__)
#	include <android/log.h>
#endif
#include <cstdio>
#include "Core/Platform.h"
#include "Core/Log/Log.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"

namespace traktor
{

#if defined(__IOS__)
extern void NSLogCpp(const wchar_t* s);
#endif

	namespace
	{

class LogTargetConsole : public ILogTarget
{
public:
	virtual void log(uint32_t threadId, int32_t level, const std::wstring& str) T_OVERRIDE T_FINAL
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		if (level == 0)
		{
			fwprintf(stdout, L"%ls\n", str.c_str());
			fflush(stdout);
		}
		else if (level == 1)
		{
			fwprintf(stdout, L"(WARN) %ls\n", str.c_str());
			fflush(stdout);
		}
		else
		{
			fwprintf(stderr, L"(ERROR) %ls\n", str.c_str());
			fflush(stderr);
		}
#if defined(__IOS__)
		NSLogCpp(str.c_str());
#elif defined(__ANDROID__)
		__android_log_print(ANDROID_LOG_INFO, "Traktor", "%s", wstombs(str).c_str());
#elif defined(_WIN32) && !defined(_XBOX_ONE)
		tstring tss = wstots(str + L"\n");
		OutputDebugString(tss.c_str());
#endif
	}

private:
	Semaphore m_lock;
};

class LogTargetDebug : public ILogTarget
{
public:
	virtual void log(uint32_t threadId, int32_t level, const std::wstring& str) T_OVERRIDE T_FINAL
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
#if defined(_WIN32) && !defined(_XBOX_ONE)
		StringOutputStream ss;
		ss << L"(" << uint32_t(GetCurrentThreadId()) << L") " << str << Endl;
		OutputDebugString(wstots(ss.str()).c_str());
#elif defined(__IOS__)
		NSLogCpp(str.c_str());
#elif defined(__ANDROID__)
		__android_log_print(ANDROID_LOG_DEBUG, "Traktor", "%s", wstombs(str).c_str());
#elif defined(_DEBUG)
		fwprintf(stdout, L"(DEBUG) %ls\n", str.c_str());
#endif
	}

private:
	Semaphore m_lock;
};

class LogStreamLocalBuffer : public IOutputStreamBuffer
{
public:
	LogStreamLocalBuffer(int32_t level, const Ref< ILogTarget >& globalTarget)
	:	m_level(level)
	,	m_globalTarget(globalTarget)
	{
	}

	ILogTarget* getTarget() const
	{
		return m_localTarget;
	}

	void setTarget(ILogTarget* target)
	{
		m_localTarget = target;
	}

	virtual int32_t getIndent() const
	{
		return m_buffer.getIndent();
	}

	virtual void setIndent(int32_t indent)
	{
		m_buffer.setIndent(indent);
	}

	virtual int32_t getDecimals() const
	{
		return m_buffer.getDecimals();
	}

	virtual void setDecimals(int32_t decimals)
	{
		m_buffer.setDecimals(decimals);
	}

	virtual bool getPushIndent() const
	{
		return m_buffer.getPushIndent();
	}

	virtual void setPushIndent(bool pushIndent)
	{
		m_buffer.setPushIndent(pushIndent);
	}

	virtual int32_t overflow(const wchar_t* buffer, int32_t count)
	{
		for (int32_t i = 0; i < count; ++i)
		{
			wchar_t c = buffer[i];
			if (c == L'\n')
			{
				uint32_t threadId = ThreadManager::getInstance().getCurrentThread()->id();

				if (m_globalTarget)
					m_globalTarget->log(threadId, m_level, m_buffer.str());
				
				if (m_localTarget)
					m_localTarget->log(threadId, m_level, m_buffer.str());

				m_buffer.reset();
			}
			else if (c != L'\r')
				m_buffer.overflow(&c, 1);
		}
		return count;
	}

private:
	int32_t m_level;
	StringOutputStreamBuffer m_buffer;
	const Ref< ILogTarget >& m_globalTarget;
	Ref< ILogTarget > m_localTarget;
};

class LogStreamGlobalBuffer : public IOutputStreamBuffer
{
public:
	LogStreamGlobalBuffer(int32_t level, ILogTarget* globalTarget)
	:	m_level(level)
	,	m_globalTarget(globalTarget)
	{
	}
	
	ILogTarget* getTarget() const
	{
		return m_globalTarget;
	}

	void setTarget(ILogTarget* target)
	{
		m_globalTarget = target;
	}

	LogStreamLocalBuffer* getThreadLocalBuffer() const
	{
		LogStreamLocalBuffer* os;
		if ((os = static_cast< LogStreamLocalBuffer* >(m_buffers.get())) == 0)
		{
			os = new LogStreamLocalBuffer(m_level, m_globalTarget);
			T_SAFE_ADDREF(os);
			m_buffers.set(os);
		}
		return os;
	}

	virtual int32_t getIndent() const
	{
		return getThreadLocalBuffer()->getIndent();
	}

	virtual void setIndent(int32_t indent)
	{
		getThreadLocalBuffer()->setIndent(indent);
	}

	virtual int32_t getDecimals() const
	{
		return getThreadLocalBuffer()->getDecimals();
	}

	virtual void setDecimals(int32_t decimals)
	{
		getThreadLocalBuffer()->setDecimals(decimals);
	}

	virtual bool getPushIndent() const
	{
		return getThreadLocalBuffer()->getPushIndent();
	}

	virtual void setPushIndent(bool pushIndent)
	{
		getThreadLocalBuffer()->setPushIndent(pushIndent);
	}

	virtual int32_t overflow(const wchar_t* buffer, int32_t count)
	{
		return getThreadLocalBuffer()->overflow(buffer, count);
	}

private:
	int32_t m_level;
	mutable ThreadLocal m_buffers;
	Ref< ILogTarget > m_globalTarget;
};

	}

LogStream::LogStream(int32_t level, ILogTarget* globalTarget)
:	OutputStream(new LogStreamGlobalBuffer(level, globalTarget), LeUnix)
{
}

LogStream::~LogStream()
{
	T_EXCEPTION_GUARD_BEGIN

	setBuffer(0);

	T_EXCEPTION_GUARD_END
}

ILogTarget* LogStream::getGlobalTarget()
{
	LogStreamGlobalBuffer* globalBuffer = checked_type_cast< LogStreamGlobalBuffer*, false >(getBuffer());
	return globalBuffer->getTarget();
}

void LogStream::setGlobalTarget(ILogTarget* target)
{
	LogStreamGlobalBuffer* globalBuffer = checked_type_cast< LogStreamGlobalBuffer*, false >(getBuffer());
	globalBuffer->setTarget(target);
}

ILogTarget* LogStream::getLocalTarget()
{
	LogStreamGlobalBuffer* globalBuffer = checked_type_cast< LogStreamGlobalBuffer*, false >(getBuffer());
	LogStreamLocalBuffer* localBuffer = globalBuffer->getThreadLocalBuffer();
	return localBuffer->getTarget();
}

void LogStream::setLocalTarget(ILogTarget* target)
{
	LogStreamGlobalBuffer* globalBuffer = checked_type_cast< LogStreamGlobalBuffer*, false >(getBuffer());
	LogStreamLocalBuffer* localBuffer = globalBuffer->getThreadLocalBuffer();
	localBuffer->setTarget(target);
}

	namespace log
	{

static LogTargetConsole infoTarget;
static LogTargetConsole warningTarget;
static LogTargetConsole errorTarget;
static LogTargetDebug debugTarget;

LogStream info(0, &infoTarget);
LogStream warning(1, &warningTarget);
LogStream error(2, &errorTarget);
LogStream debug(3, &debugTarget);

	}
}
