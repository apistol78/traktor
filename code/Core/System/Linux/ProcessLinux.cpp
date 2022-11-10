/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
		int64_t ret = ::read(m_pipe, block, nbytes);
		if (ret < 0)
		{
			// No data on pipe, would block if pipe was in block mode.
			if (errno == EAGAIN)
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

T_IMPLEMENT_RTTI_CLASS(L"traktor.ProcessLinux", ProcessLinux, IProcess)

ProcessLinux::~ProcessLinux()
{
	if (m_fileActions)
	{
		posix_spawn_file_actions_destroy(m_fileActions);
		delete m_fileActions;
	}
	if (m_spawnAttrp)
	{
		posix_spawnattr_destroy(m_spawnAttrp);
		delete m_spawnAttrp;
	}

	if (m_childStdOut != 0)
		::close(m_childStdOut);
	if (m_childStdErr != 0)
		::close(m_childStdErr);
}

bool ProcessLinux::setPriority(Priority priority)
{
	return false;
}

IStream* ProcessLinux::getPipeStream(StdPipe pipe)
{
	switch (pipe)
	{
	case SpStdOut:
		return m_streamStdOut;
	case SpStdErr:
		return m_streamStdErr;
	default:
		return nullptr;
	}
}

IStream* ProcessLinux::waitPipeStream(int32_t timeout)
{
	fd_set readSet = {};
	int nfd = 0;

	if (m_childStdOut == 0 && m_childStdErr == 0)
		return nullptr;

 	if (m_childStdOut != 0)
	{
		FD_SET(m_childStdOut, &readSet);
		nfd = std::max(nfd, m_childStdOut);
	}
	if (m_childStdErr != 0)
	{
		FD_SET(m_childStdErr, &readSet);
		nfd = std::max(nfd, m_childStdErr);
	}

	sigset_t sigmask;
	sigemptyset(&sigmask);

	int rc;
	if (timeout >= 0)
	{
		timespec to = { timeout / 1000, (timeout % 1000) * 1000000 };
		rc = ::pselect(nfd + 1, &readSet, nullptr, nullptr, &to, &sigmask);
	}
	else
		rc = ::pselect(nfd + 1, &readSet, nullptr, nullptr, nullptr, &sigmask);

	if (rc <= 0)
		return nullptr;

	if (FD_ISSET(m_childStdOut, &readSet))
		return m_streamStdOut;
	if (FD_ISSET(m_childStdErr, &readSet))
		return m_streamStdErr;

	return nullptr;
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

ProcessLinux::ProcessLinux(
	pid_t pid,
	posix_spawn_file_actions_t* fileActions,
	posix_spawnattr_t* spawnAttrp,
	int childStdOut,
	int childStdErr
)
:	m_pid(pid)
,	m_fileActions(fileActions)
,	m_spawnAttrp(spawnAttrp)
,	m_childStdOut(childStdOut)
,	m_childStdErr(childStdErr)
,	m_exitCode(0)
{
	if (m_childStdOut != 0)
		m_streamStdOut = new PipeStream(m_pid, m_childStdOut);
	if (m_childStdErr != 0)
		m_streamStdErr = new PipeStream(m_pid, m_childStdErr);
}

}
