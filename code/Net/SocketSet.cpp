#include <algorithm>
#include "Net/SocketSet.h"
#include "Net/Socket.h"

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

int SocketSet::select(bool read, bool write, bool except, int timeout, SocketSet& outResultSet)
{
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

	Ref< SocketSet > result = new SocketSet();
#if !defined(_PS3)
	if (::select(0, fds[0], fds[1], fds[2], &to) > 0)
#else
	if (socketselect(0, fds[0], fds[1], fds[2], &to) > 0)
#endif
	{
#if defined(_WIN32)
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
#else
#pragma warning("Not implemented")
#endif
	}

	return outResultSet.count();
}

	}
}
