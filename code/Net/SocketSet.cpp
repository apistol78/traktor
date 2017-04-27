/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#if !defined(_WIN32) && !defined(__PS3__) && !defined(__PS4__)
#	include <poll.h>
#endif
#include "Core/Containers/StaticVector.h"
#include "Net/Socket.h"
#include "Net/SocketSet.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.SocketSet", SocketSet, Object)

void SocketSet::add(Socket* socket)
{
	RefArray< Socket >::iterator i = std::find(m_sockets.begin(), m_sockets.end(), socket);
	if (i == m_sockets.end())
		m_sockets.push_back(socket);
}

void SocketSet::remove(Socket* socket)
{
	RefArray< Socket >::iterator i = std::find(m_sockets.begin(), m_sockets.end(), socket);
	if (i != m_sockets.end())
		m_sockets.erase(i);
}

int SocketSet::count() const
{
	return int(m_sockets.size());
}

Ref< Socket > SocketSet::get(int index) const
{
	T_ASSERT (index >= 0 && index < int(m_sockets.size()));
	return m_sockets[index];
}

bool SocketSet::contain(Socket* socket) const
{
	return std::find(m_sockets.begin(), m_sockets.end(), socket) != m_sockets.end();
}

int SocketSet::select(bool read, bool write, bool except, int timeout, SocketSet& outResultSet)
{
#if !defined(_WIN32) && !defined(__PS3__) && !defined(__PS4__)
	StaticVector< struct pollfd, 128 > fds;

	for (uint32_t i = 0; i < m_sockets.size(); ++i)
	{
		struct pollfd& fd = fds.push_back();

		fd.fd = m_sockets[i]->handle();
		fd.events = 0;
		fd.revents = 0;

		if (read || except)
			fd.events |= POLLIN;
		if (write)
			fd.events |= POLLOUT;
	}

	int32_t rv = ::poll(fds, fds.size(), timeout);
	if (rv <= 0)
		return 0;

	for (uint32_t i = 0; i < m_sockets.size(); ++i)
	{
		if (fds[i].revents != 0)
			outResultSet.add(m_sockets[i]);
	}
#elif defined(_WIN32) && !defined(__PS3__) && !defined(__PS4__)
	timeval to = { timeout / 1000, (timeout % 1000) * 1000 };
	fd_set* fds[] = { 0, 0, 0 };
	fd_set readfds, writefds, exceptfds;

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	if (read == true)
	{
		for (RefArray< Socket >::iterator i = m_sockets.begin(); i != m_sockets.end(); ++i)
			FD_SET((*i)->handle(), &readfds);
		fds[0] = &readfds;
	}

	if (write == true)
	{
		for (RefArray< Socket >::iterator i = m_sockets.begin(); i != m_sockets.end(); ++i)
			FD_SET((*i)->handle(), &readfds);
		fds[1] = &writefds;
	}

	if (except == true)
	{
		for (RefArray< Socket >::iterator i = m_sockets.begin(); i != m_sockets.end(); ++i)
			FD_SET((*i)->handle(), &readfds);
		fds[2] = &exceptfds;
	}

	if (::select(0, fds[0], fds[1], fds[2], &to) > 0)
	{
		for (int ii = 0; ii < 3; ++ii)
		{
			if (!fds[ii])
				continue;
			for (RefArray< Socket >::iterator i = m_sockets.begin(); i != m_sockets.end(); ++i)
			{
				for (unsigned j = 0; j < fds[ii]->fd_count; ++j)
				{
					if ((*i)->handle() == fds[ii]->fd_array[j])
					{
						outResultSet.add(*i);
						break;
					}
				}
			}
		}
	}
#endif

	return outResultSet.count();
}

	}
}
