#include <errno.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/System/Osx/ProcessOsX.h"

namespace traktor
{
	namespace
	{

class PipeStream : public IStream
{
public:
	PipeStream(pid_t pid, int pipe)
	:   m_pid(pid)
	,	m_pipe(pipe)
	{
		int flags = fcntl(m_pipe, F_GETFL, 0);
		fcntl(m_pipe, F_SETFL, flags | O_NONBLOCK);
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
		return 0;
	}

	virtual int64_t seek(SeekOriginType origin, int64_t offset)
	{
		return 0;
	}

	virtual int64_t read(void* block, int64_t nbytes)
	{
		int exitCode;
		int64_t ret;

		ret = ::read(m_pipe, block, nbytes);
		if (ret < 0)
		{
			bool processTerminated = !(waitpid(m_pid, &exitCode, WNOHANG) >= 0);
			if (!processTerminated && errno == EAGAIN)
				return 0;
			else
				return -1;
		}

		return ret;
	}

	virtual int64_t write(const void* block, int64_t nbytes)
	{
		return 0;
	}

	virtual void flush()
	{
	}

private:
	pid_t m_pid;
	int m_pipe;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ProcessOsX", ProcessOsX, IProcess)

ProcessOsX::~ProcessOsX()
{
	if (m_fileActions)
	{
		posix_spawn_file_actions_destroy(m_fileActions);
		delete m_fileActions;

		::close(m_childStdOut);
		::close(m_childStdErr);
	}
}

bool ProcessOsX::setPriority(Priority priority)
{
	return false;
}

Ref< IStream > ProcessOsX::getPipeStream(StdPipe pipe)
{
	if (m_fileActions)
	{
		switch (pipe)
		{
			case SpStdOut:
			return new PipeStream(m_pid, m_childStdOut);

			case SpStdErr:
			return new PipeStream(m_pid, m_childStdErr);

			default:
			return 0;
		}
	}
	else
		return 0;
}

bool ProcessOsX::signal(SignalType signalType)
{
	return false;
}

bool ProcessOsX::terminate(int32_t exitCode)
{
	return false;
}

int32_t ProcessOsX::exitCode() const
{
	return m_exitCode;
}

bool ProcessOsX::wait(int32_t timeout)
{
	if (timeout >= 0)
	{
		for (;;)
		{
			if (waitpid(m_pid, &m_exitCode, WNOHANG) > 0)
				return true;
			
			if ((timeout -= 10) < 0)
				return false;

			usleep(10 * 1000);
		}
	}
	else
		return waitpid(m_pid, &m_exitCode, 0) >= 0;
}

ProcessOsX::ProcessOsX(pid_t pid, posix_spawn_file_actions_t* fileActions, int childStdOut, int childStdErr)
:	m_pid(pid)
,	m_fileActions(fileActions)
,	m_childStdOut(childStdOut)
,	m_childStdErr(childStdErr)
,	m_exitCode(0)
{
}

}

