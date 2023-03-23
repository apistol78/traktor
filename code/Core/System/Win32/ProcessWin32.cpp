/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/BufferedStream.h"
#include "Core/Log/Log.h"
#include "Core/System/Win32/ProcessWin32.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/Thread.h"

namespace traktor
{
	namespace
	{

class PipeStream : public IStream
{
public:
	PipeStream(HANDLE hProcess, HANDLE hPipe)
	:	m_hProcess(hProcess)
	,	m_hPipe(hPipe)
	{
		m_thread = ThreadManager::getInstance().create([this]() {
			threadPipeReader();
		});
		m_thread->start();
	}

	virtual ~PipeStream()
	{
		T_FATAL_ASSERT(m_thread == nullptr);
	}

	virtual void close()
	{
	}

	virtual bool canRead() const
	{
		return true;
	}

	virtual bool canWrite() const
	{
		return false;
	}

	virtual bool canSeek() const
	{
		return false;
	}

	virtual int64_t tell() const
	{
		return 0;
	}

	virtual int64_t available() const
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		return (int32_t)m_buffer.size();
	}

	virtual int64_t seek(SeekOriginType origin, int64_t offset)
	{
		return 0;
	}

	virtual int64_t read(void* block, int64_t nbytes)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		uint32_t nread = std::min< uint32_t >(nbytes, m_buffer.size());
		if (nread == 0)
			return 0;

		std::memcpy(block, m_buffer.c_ptr(), nread);
		m_buffer.erase(m_buffer.begin(), m_buffer.begin() + nread);
		return nread;
	}

	virtual int64_t write(const void* block, int64_t nbytes)
	{
		return 0;
	}

	virtual void flush()
	{
	}

	void cancelThread()
	{
		T_FATAL_ASSERT(m_thread != nullptr);
		m_thread->stop();
		ThreadManager::getInstance().destroy(m_thread);
		m_thread = nullptr;
	}

private:
	HANDLE m_hProcess;
	HANDLE m_hPipe;
	Thread* m_thread;
	mutable Semaphore m_lock;
	AlignedVector< uint8_t > m_buffer;

	void threadPipeReader()
	{
		uint8_t block[64];
		while (!m_thread->stopped())
		{
			DWORD nread;
			if (!ReadFile(m_hPipe, block, (DWORD)sizeof(block), &nread, NULL))
				break;

			if (nread > 0)
			{
				T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
				m_buffer.insert(m_buffer.end(), &block[0], &block[nread]);
			}
		}
	}
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ProcessWin32", ProcessWin32, IProcess)

ProcessWin32::ProcessWin32(
	HANDLE hProcess,
	DWORD dwProcessId,
	HANDLE hThread,
	HANDLE hStdInWrite,
	HANDLE hStdOutRead,
	HANDLE hStdErrRead
)
:	m_hProcess(hProcess)
,	m_dwProcessId(dwProcessId)
,	m_hThread(hThread)
,	m_hStdInWrite(hStdInWrite)
,	m_hStdOutRead(hStdOutRead)
,	m_hStdErrRead(hStdErrRead)
{
	m_pipeStdOut = new PipeStream(m_hProcess, m_hStdOutRead);
	m_pipeStdErr = new PipeStream(m_hProcess, m_hStdErrRead);
}

ProcessWin32::~ProcessWin32()
{
	CloseHandle(m_hStdInWrite);
	CloseHandle(m_hStdOutRead);
	CloseHandle(m_hStdErrRead);

	static_cast< PipeStream* >(m_pipeStdOut.ptr())->cancelThread();
	static_cast< PipeStream* >(m_pipeStdErr.ptr())->cancelThread();

	CloseHandle(m_hProcess);
	CloseHandle(m_hThread);
}

bool ProcessWin32::setPriority(Priority priority)
{
	const DWORD c_priorityClasses[] =
	{
		IDLE_PRIORITY_CLASS,
		BELOW_NORMAL_PRIORITY_CLASS,
		NORMAL_PRIORITY_CLASS,
		ABOVE_NORMAL_PRIORITY_CLASS,
		HIGH_PRIORITY_CLASS
	};
	return SetPriorityClass(m_hProcess, c_priorityClasses[priority]) != 0;
}

bool ProcessWin32::wait(int32_t timeout)
{
	return WaitForSingleObject(m_hProcess, timeout >= 0 ? timeout : INFINITE) == WAIT_OBJECT_0;
}

IStream* ProcessWin32::getPipeStream(StdPipe pipe)
{
	if (pipe == SpStdOut)
		return m_pipeStdOut;
	else if (pipe == SpStdErr)
		return m_pipeStdErr;
	else
		return nullptr;
}

IProcess::WaitPipeResult ProcessWin32::waitPipeStream(int32_t timeout, Ref< IStream >& outPipe)
{
	T_FATAL_ASSERT(timeout >= 0);

	bool terminated = false;
	for (int32_t i = 0; i < timeout; i += 10)
	{
		if (m_pipeStdOut->available() > 0)
		{
			outPipe = m_pipeStdOut;
			return Ready;
		}
		
		if (m_pipeStdErr->available() > 0)
		{
			outPipe = m_pipeStdErr;
			return Ready;
		}

		if (terminated)
			return Terminated;

		terminated = wait(10);
	}

	return Timeout;
}

bool ProcessWin32::signal(SignalType signalType)
{
	switch (signalType)
	{
	case StCtrlC:
		if (!GenerateConsoleCtrlEvent(CTRL_C_EVENT, m_dwProcessId))
			return false;
		break;

	case StCtrlBreak:
		if (!GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, m_dwProcessId))
			return false;
		break;

	default:
		return false;
	}
	return true;
}

bool ProcessWin32::terminate(int32_t exitCode)
{
	return TerminateProcess(m_hProcess, 0) == TRUE;
}

int32_t ProcessWin32::exitCode() const
{
	DWORD code = 0;

	if (!GetExitCodeProcess(m_hProcess, &code))
		return 0;

	return (int32_t)code;
}

}
