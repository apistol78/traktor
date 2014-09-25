#include <unistd.h>
#include <sys/ioctl.h>
#include "Core/Io/BufferedStream.h"
#include "Core/Log/Log.h"
#include "Core/System/Osx/ProcessOsX.h"

namespace traktor
{
	namespace
	{

class PipeStream : public IStream
{
public:
	PipeStream(int pipe)
	:	m_pipe(pipe)
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
		int avail = 0;
		::ioctl(m_pipe, FIONREAD, &avail);
		return avail;
	}

	virtual int seek(SeekOriginType origin, int offset)
	{
		return 0;
	}

	virtual int read(void* block, int nbytes)
	{
		int ret = ::read(m_pipe, block, nbytes);
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

Ref< IStream > ProcessOsX::getPipeStream(StdPipe pipe)
{
	if (m_fileActions)
	{
		switch (pipe)
		{
		case SpStdOut:
			return new BufferedStream(new PipeStream(m_childStdOut));

		case SpStdErr:
			return new BufferedStream(new PipeStream(m_childStdErr));

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

