#include <cstring>
#include "Core/Functor/Functor.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadPool.h"
#include "Core/Timer/Measure.h"
#include "Online/ILobby.h"
#include "Online/ISessionManager.h"
#include "Online/IUser.h"
#include "Online/OnlinePeer2PeerProvider.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

struct P2PUserFindPred
{
	const IUser* m_user;

	P2PUserFindPred(const IUser* user)
	:	m_user(user)
	{
	}

	bool operator () (const OnlinePeer2PeerProvider::P2PUser& p2pu) const
	{
		return p2pu.user->getGlobalId() == m_user->getGlobalId();
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.OnlinePeer2PeerProvider", OnlinePeer2PeerProvider, net::IPeer2PeerProvider)

OnlinePeer2PeerProvider::OnlinePeer2PeerProvider(ISessionManager* sessionManager, ILobby* lobby, bool asyncTx, bool asyncRx)
:	m_sessionManager(sessionManager)
,	m_lobby(lobby)
,	m_localHandle(sessionManager->getUser()->getGlobalId())
,	m_primaryHandle(0)
,	m_whenUpdate(0.0)
,	m_asyncTx(asyncTx)
,	m_asyncRx(asyncRx)
,	m_thread(0)
{
	Ref< IUser > fromUser;
	uint8_t data[1600];

	// Purge pending data.
	while(m_sessionManager->receiveP2PData(data, sizeof(data), fromUser) > 0)
		;

	// Create transmission thread.
	if (asyncTx || asyncRx)
	{
		if (!ThreadPool::getInstance().spawn(makeFunctor(this, &OnlinePeer2PeerProvider::transmissionThread), m_thread))
		{
			m_thread = 0;
			m_asyncTx =
			m_asyncRx = false;
		}
	}

	m_timer.start();
}

OnlinePeer2PeerProvider::~OnlinePeer2PeerProvider()
{
	// Disable P2P with all connected users.
	for (AlignedVector< P2PUser >::iterator i = m_users.begin(); i != m_users.end(); ++i)
		i->user->setP2PEnable(false);

	// Terminate transmission thread.
	if (m_thread)
	{
		ThreadPool::getInstance().stop(m_thread);
		m_thread = 0;
	}
}

bool OnlinePeer2PeerProvider::update()
{
	if (m_timer.getElapsedTime() >= m_whenUpdate)
	{
		RefArray< IUser > users;
		T_MEASURE_STATEMENT(m_lobby->getParticipants(users), 0.001);

		// Add new users which have entered the lobby.
		for (RefArray< IUser >::iterator i = users.begin(); i != users.end(); ++i)
		{
			if (std::find_if(m_users.begin(), m_users.end(), P2PUserFindPred(*i)) == m_users.end())
			{
				(*i)->setP2PEnable(true);

				P2PUser p2pu;
				p2pu.user = *i;
				p2pu.timeout = 0;
				m_users.push_back(p2pu);

				log::info << L"[Online P2P] Peer " << (*i)->getGlobalId() << L" added." << Endl;
			}
		}

		// Increment timeout counter for users which have left.
		for (AlignedVector< P2PUser >::iterator i = m_users.begin(); i != m_users.end(); ++i)
		{
			if (std::find(users.begin(), users.end(), i->user) != users.end())
			{
				if (i->timeout > 0)
				{
					log::info << L"[Online P2P] Resurrected peer " << i->user->getGlobalId() << L"." << Endl;
					i->timeout = 0;
				}
			}
			else
				i->timeout++;
		}

		// Remove users which have a timeout greater than limit.
		for (size_t i = 0; i < m_users.size(); )
		{
			if (m_users[i].timeout >= 4 || !m_users[i].user)
			{
				if (m_users[i].user)
				{
					m_users[i].user->setP2PEnable(false);
					log::info << L"[Online P2P] Peer " << m_users[i].user->getGlobalId() << L" removed." << Endl;
				}
				m_users.erase(m_users.begin() + i);
			}
			else
				++i;
		}

		// Cache primary handle.
		T_MEASURE_STATEMENT(m_primaryHandle = net::net_handle_t(m_lobby->getOwner()->getGlobalId()), 0.0005);
		m_whenUpdate = m_timer.getElapsedTime() + 0.2;
	}
	return true;
}

net::net_handle_t OnlinePeer2PeerProvider::getLocalHandle() const
{
	return m_localHandle;
}

int32_t OnlinePeer2PeerProvider::getPeerCount() const
{
	return 1 + int32_t(m_users.size());
}

net::net_handle_t OnlinePeer2PeerProvider::getPeerHandle(int32_t index) const
{
	if (index <= 0)
		return getLocalHandle();
	else
		return net::net_handle_t(m_users[index - 1].user->getGlobalId());
}

std::wstring OnlinePeer2PeerProvider::getPeerName(int32_t index) const
{
	std::wstring name;
	if (index <= 0)
		T_MEASURE_STATEMENT(m_sessionManager->getUser()->getName(name), 0.001)
	else
		T_MEASURE_STATEMENT(m_users[index - 1].user->getName(name), 0.001)
	return name;
}

Object* OnlinePeer2PeerProvider::getPeerUser(int32_t index) const
{
	if (index <= 0)
		return m_sessionManager->getUser();
	else
		return m_users[index - 1].user;
}

bool OnlinePeer2PeerProvider::setPrimaryPeerHandle(net::net_handle_t node)
{
	for (AlignedVector< P2PUser >::const_iterator i = m_users.begin(); i != m_users.end(); ++i)
	{
		if (i->user->getGlobalId() == node)
		{
#if defined(_DEBUG)
			log::info << L"[Online P2P] Migrating primary token to peer " << node << L"..." << Endl;
#endif
			if (m_lobby->setOwner(i->user))
			{
				m_whenUpdate = 0.0f;
				return true;
			}
		}
	}
#if defined(_DEBUG)
	log::error << L"[Online P2P] Failed to migrate primary token to peer " << node << L"." << Endl;
#endif
	return false;
}

net::net_handle_t OnlinePeer2PeerProvider::getPrimaryPeerHandle() const
{
	return m_primaryHandle;
}

bool OnlinePeer2PeerProvider::send(net::net_handle_t node, const void* data, int32_t size)
{
	for (AlignedVector< P2PUser >::iterator i = m_users.begin(); i != m_users.end(); ++i)
	{
		if (i->user->getGlobalId() == node)
		{
			if (m_asyncTx)
			{
				T_ASSERT (m_thread);
				m_txQueueLock.wait();

				RxTxData& tx = m_txQueue.push_back();

				tx.user = i->user;
				tx.size = size;
				std::memcpy(tx.data, data, size);

				m_txQueueSignal.set();
				m_txQueueLock.release();

				return true;
			}
			else
			{
				return i->user->sendP2PData(data, size, false);
			}
		}
	}
	log::error << L"[Online P2P] Unable to send to " << node << L"; no such node." << Endl;
	return false;
}

int32_t OnlinePeer2PeerProvider::recv(void* data, int32_t size, net::net_handle_t& outNode)
{
	int32_t nrecv = 0;

	outNode = 0;

	if (m_asyncRx)
	{
		T_ASSERT (m_thread);

		if (m_rxQueuePending <= 0)
			return 0;

		m_rxQueueLock.wait();
		if (!m_rxQueue.empty())
		{
			RxTxData& rx = m_rxQueue.front();

			if (std::find_if(m_users.begin(), m_users.end(), P2PUserFindPred(rx.user)) != m_users.end())
			{
				nrecv = std::min< int32_t >(size, rx.size);
				std::memcpy(data, rx.data, nrecv);
				outNode = net::net_handle_t(rx.user->getGlobalId());
			}
			else
				log::warning << L"[Online P2P] Received data from unknown user " << rx.user->getGlobalId() << L" (1)." << Endl;

			m_rxQueue.pop_front();
			--m_rxQueuePending;
		}
		m_rxQueueLock.release();
	}
	else
	{
		if (m_sessionManager->haveP2PData())
		{
			Ref< IUser > fromUser;
			nrecv = m_sessionManager->receiveP2PData(data, size, fromUser);
			if (nrecv <= 0)
				return nrecv;

			if (fromUser != 0 && std::find_if(m_users.begin(), m_users.end(), P2PUserFindPred(fromUser)) != m_users.end())
				outNode = net::net_handle_t(fromUser->getGlobalId());
			else
			{
				log::warning << L"[Online P2P] Received data from unknown user " << fromUser->getGlobalId() << L" (2)." << Endl;
				nrecv = 0;
			}
		}
	}

	return nrecv;
}

void OnlinePeer2PeerProvider::transmissionThread()
{
	T_ASSERT (m_asyncTx || m_asyncRx);
	RxTxData rxtx;

	while (!m_thread->stopped())
	{
		if (m_asyncRx)
		{
			if (m_sessionManager->haveP2PData())
			{
				rxtx.size = m_sessionManager->receiveP2PData(rxtx.data, sizeof(rxtx.data), rxtx.user);
				if (rxtx.size > 0 && rxtx.user)
				{
					m_rxQueueLock.wait();
					m_rxQueue.push_back(rxtx);
					++m_rxQueuePending;
					m_rxQueueLock.release();
				}
				continue;
			}
		}

		if (m_asyncTx)
		{
			if (m_txQueueSignal.wait(m_asyncRx ? 4 : 100))
			{
				m_txQueueLock.wait();
				rxtx = m_txQueue.front();
				m_txQueue.pop_front();
				if (m_txQueue.empty())
					m_txQueueSignal.reset();
				m_txQueueLock.release();

				rxtx.user->sendP2PData(rxtx.data, rxtx.size, false);
			}
		}
		else
		{
			T_ASSERT (m_asyncRx);
			m_thread->sleep(4);
		}
	}
}

	}
}
