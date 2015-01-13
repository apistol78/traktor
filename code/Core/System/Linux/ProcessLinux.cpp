#include <errno.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include "Core/Io/IStream.h"
#include "Core/System/Linux/ProcessLinux.h"

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
		int exitCode;
		int ret;

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

	virtual int write(const void* block, int nbytes)
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.ProcessLinux", ProcessLinux, IProcess)

ProcessLinux::~ProcessLinux()
{
	if (m_fileActions)
	{
		posix_spawn_file_actions_destroy(m_fileActions);
		delete m_fileActions;

		::close(m_childStdOut);
		::close(m_childStdErr);
	}
}

Ref< IStream > ProcessLinux::getPipeStream(StdPipe pipe)
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

bool ProcessLinux::signal(SignalType signalType)
{
	return false;
}

bool ProcessLinux::terminate(int32_t exitCode)
{
	return false;
}

int32_t ProcessLinux::exitCode() const
{
	return m_exitCode;
}

bool ProcessLinux::wait(int32_t timeout)
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

ProcessLinux::ProcessLinux(pid_t pid, posix_spawn_file_actions_t* fileActions, int childStdOut, int childStdErr)
:	m_pid(pid)
,	m_fileActions(fileActions)
,	m_childStdOut(childStdOut)
,	m_childStdErr(childStdErr)
,	m_exitCode(0)
{
}

}
