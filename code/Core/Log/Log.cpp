/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#if defined(__ANDROID__)
#	include <android/log.h>
#endif
#include <cstdio>
#include "Core/Log/Log.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/TString.h"
#include "Core/System.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadLocal.h"
#include "Core/Thread/ThreadManager.h"

namespace traktor
{

#if defined(__IOS__)
extern void NSLogCpp(const wchar_t* s);
#endif

	namespace
	{

/*! Standard IO console target. */
class LogTargetConsole : public ILogTarget
{
public:
	virtual void log(uint32_t threadId, int32_t level, const wchar_t* str) override final
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		if (level == 0)
		{
			fwprintf(stdout, L"%ls\n", str);
		}
		else if (level == 1)
		{
			fwprintf(stdout, L"%ls\n", str);
			fflush(stdout);
		}
		else
		{
			fflush(stdout);
			fwprintf(stdout, L"%ls\n", str);
			fflush(stderr);
		}
#if defined(__IOS__)
		NSLogCpp(str);
#elif defined(__ANDROID__)
		__android_log_print(ANDROID_LOG_INFO, "Traktor", "%s", wstombs(str).c_str());
#elif defined(_WIN32)
		OutputDebugStringW(str);
		OutputDebugStringW(L"\n");
#endif
	}

private:
	Semaphore m_lock;
	bool m_colorStdOut;
	bool m_colorStdErr;
};

/*! Debug log target, OS specific debug channel. */
class LogTargetDebug : public ILogTarget
{
public:
	virtual void log(uint32_t threadId, int32_t level, const wchar_t* str) override final
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
#if defined(_WIN32)
		StringOutputStream ss;
		ss << L"(" << uint32_t(GetCurrentThreadId()) << L") " << str << Endl;
		OutputDebugStringW(ss.str().c_str());
#elif defined(__IOS__)
		NSLogCpp(str);
#elif defined(__ANDROID__)
		__android_log_print(ANDROID_LOG_DEBUG, "Traktor", "%s", wstombs(str).c_str());
#elif defined(_DEBUG)
		fwprintf(stdout, L"(DEBUG) %ls\n", str);
#endif
	}

private:
	Semaphore m_lock;
};

/*! Default local target which propagate logs into global target. */
class LogTargetGlobalSink : public ILogTarget
{
public:
	explicit LogTargetGlobalSink(const Ref< ILogTarget >& globalTarget)
	:	m_globalTarget(globalTarget)
	{
	}

	virtual void log(uint32_t threadId, int32_t level, const wchar_t* str) override final
	{
		if (m_globalTarget)
			m_globalTarget->log(threadId, level, str);
	}

private:
	const Ref< ILogTarget >& m_globalTarget;
};

class LogStreamLocalBuffer : public IOutputStreamBuffer
{
public:
	explicit LogStreamLocalBuffer(int32_t level, const Ref< ILogTarget >& globalTarget)
	:	m_level(level)
	,	m_localTarget(new LogTargetGlobalSink(globalTarget))
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

	virtual int32_t getIndent() const override final
	{
		return m_buffer.getIndent();
	}

	virtual void setIndent(int32_t indent) override final
	{
		m_buffer.setIndent(indent);
	}

	virtual int32_t getDecimals() const override final
	{
		return m_buffer.getDecimals();
	}

	virtual void setDecimals(int32_t decimals) override final
	{
		m_buffer.setDecimals(decimals);
	}

	virtual bool getPushIndent() const override final
	{
		return m_buffer.getPushIndent();
	}

	virtual void setPushIndent(bool pushIndent) override final
	{
		m_buffer.setPushIndent(pushIndent);
	}

	virtual int32_t overflow(const wchar_t* buffer, int32_t count) override final
	{
		const uint32_t threadId = m_localTarget ? ThreadManager::getInstance().getCurrentThread()->id() : 0;
		for (int32_t i = 0; i < count; ++i)
		{
			wchar_t c = buffer[i];
			if (c == L'\n')
			{
				if (m_localTarget)
					m_localTarget->log(threadId, m_level, m_buffer.c_str());
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
	Ref< ILogTarget > m_localTarget;
};

class LogStreamGlobalBuffer : public IOutputStreamBuffer
{
public:
	explicit LogStreamGlobalBuffer(int32_t level, ILogTarget* globalTarget)
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
		if ((os = static_cast< LogStreamLocalBuffer* >(m_buffers.get())) == nullptr)
		{
			os = new LogStreamLocalBuffer(m_level, m_globalTarget);
			T_SAFE_ADDREF(os);
			m_buffers.set(os);
		}
		return os;
	}

	virtual int32_t getIndent() const override final
	{
		return getThreadLocalBuffer()->getIndent();
	}

	virtual void setIndent(int32_t indent) override final
	{
		getThreadLocalBuffer()->setIndent(indent);
	}

	virtual int32_t getDecimals() const override final
	{
		return getThreadLocalBuffer()->getDecimals();
	}

	virtual void setDecimals(int32_t decimals) override final
	{
		getThreadLocalBuffer()->setDecimals(decimals);
	}

	virtual bool getPushIndent() const override final
	{
		return getThreadLocalBuffer()->getPushIndent();
	}

	virtual void setPushIndent(bool pushIndent) override final
	{
		getThreadLocalBuffer()->setPushIndent(pushIndent);
	}

	virtual int32_t overflow(const wchar_t* buffer, int32_t count) override final
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
:	OutputStream(new LogStreamGlobalBuffer(level, globalTarget), LineEnd::Unix)
{
}

LogStream::~LogStream()
{
	T_EXCEPTION_GUARD_BEGIN
	setBuffer(nullptr);
	T_EXCEPTION_GUARD_END
}

ILogTarget* LogStream::getGlobalTarget()
{
	auto globalBuffer = mandatory_non_null_type_cast< LogStreamGlobalBuffer* >(getBuffer());
	return globalBuffer->getTarget();
}

void LogStream::setGlobalTarget(ILogTarget* target)
{
	auto globalBuffer = mandatory_non_null_type_cast< LogStreamGlobalBuffer* >(getBuffer());
	globalBuffer->setTarget(target);
}

ILogTarget* LogStream::getLocalTarget()
{
	auto globalBuffer = mandatory_non_null_type_cast< LogStreamGlobalBuffer* >(getBuffer());
	auto localBuffer = globalBuffer->getThreadLocalBuffer();
	return localBuffer->getTarget();
}

void LogStream::setLocalTarget(ILogTarget* target)
{
	auto globalBuffer = mandatory_non_null_type_cast< LogStreamGlobalBuffer* >(getBuffer());
	auto localBuffer = globalBuffer->getThreadLocalBuffer();
	localBuffer->setTarget(target);
}

}

namespace traktor::log
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
