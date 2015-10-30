#include "Core/Io/BufferedStream.h"
#include "Core/Log/Log.h"
#include "Core/System/Win32/ProcessWin32.h"

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

	virtual int tell() const
	{
		return 0;
	}

	virtual int available() const
	{
		return 0;
	}

	virtual int seek(SeekOriginType origin, int offset)
	{
		return 0;
	}

	virtual int read(void* block, int nbytes)
	{
		bool processTerminated = (WaitForSingleObject(m_hProcess, 0) == WAIT_OBJECT_0);

		DWORD dwPending = 0;
		if (!PeekNamedPipe(m_hPipe, NULL, 0, NULL, &dwPending, NULL))
			return -1;

		DWORD dwSafeRead = min(nbytes, dwPending);
		DWORD dwRead = 0;

		if (dwSafeRead > 0)
		{
			if (!ReadFile(m_hPipe, block, dwSafeRead, &dwRead, NULL))
				return -1;
		}

		if (dwRead == 0 && processTerminated)
		{
			Sleep(100);

			if (!PeekNamedPipe(m_hPipe, NULL, 0, NULL, &dwPending, NULL))
				return -1;

			dwSafeRead = min(nbytes, dwPending);

			if (dwSafeRead > 0)
			{
				if (!ReadFile(m_hPipe, block, dwSafeRead, &dwRead, NULL))
					return -1;
			}

			if (dwRead == 0)
				return -1;
		}

		return int(dwRead);
	}

	virtual int write(const void* block, int nbytes)
	{
		return 0;
	}

	virtual void flush()
	{
	}

private:
	HANDLE m_hProcess;
	HANDLE m_hPipe;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ProcessWin32", ProcessWin32, IProcess)

ProcessWin32::ProcessWin32(
	HANDLE hProcess,
	DWORD dwProcessId,
	HANDLE hThread,
	HANDLE hStdInRead,
	HANDLE hStdInWrite,
	HANDLE hStdOutRead,
	HANDLE hStdOutWrite,
	HANDLE hStdErrRead,
	HANDLE hStdErrWrite
)
:	m_hProcess(hProcess)
,	m_dwProcessId(dwProcessId)
,	m_hThread(hThread)
,	m_hStdInRead(hStdInRead)
,	m_hStdInWrite(hStdInWrite)
,	m_hStdOutRead(hStdOutRead)
,	m_hStdOutWrite(hStdOutWrite)
,	m_hStdErrRead(hStdErrRead)
,	m_hStdErrWrite(hStdErrWrite)
{
	m_pipeStdOut = new PipeStream(m_hProcess, m_hStdOutRead);
	m_pipeStdErr = new PipeStream(m_hProcess, m_hStdErrRead);
}

ProcessWin32::~ProcessWin32()
{
	CloseHandle(m_hProcess);
	CloseHandle(m_hThread);
	CloseHandle(m_hStdInRead);
	CloseHandle(m_hStdInWrite);
	CloseHandle(m_hStdOutRead);
	CloseHandle(m_hStdOutWrite);
	CloseHandle(m_hStdErrRead);
	CloseHandle(m_hStdErrWrite);
}

bool ProcessWin32::wait(int32_t timeout)
{
	return WaitForSingleObject(m_hProcess, timeout >= 0 ? timeout : INFINITE) == WAIT_OBJECT_0;
}

Ref< IStream > ProcessWin32::getPipeStream(StdPipe pipe)
{
	if (pipe == SpStdOut)
		return m_pipeStdOut;
	else if (pipe == SpStdErr)
		return m_pipeStdErr;
	else
		return 0;
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
