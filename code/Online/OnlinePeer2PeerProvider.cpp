#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
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

Timer s_timer;

struct P2PUserFindPred
{
	const IUser* m_user;

	P2PUserFindPred(const IUser* user)
	:	m_user(user)
	{
	}

	bool operator () (const OnlinePeer2PeerProvider::P2PUser& p2pu) const
	{
		return p2pu.user == m_user;
	}
};

struct P2PUserTimeoutPred
{
	int32_t m_timeout;

	P2PUserTimeoutPred(int32_t timeout)
	:	m_timeout(timeout)
	{
	}

	bool operator () (const OnlinePeer2PeerProvider::P2PUser& p2pu) const
	{
		return p2pu.timeout >= m_timeout;
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
		m_thread = ThreadManager::getInstance().create(makeFunctor(this, &OnlinePeer2PeerProvider::transmissionThread), L"Online P2P");
		if (!m_thread->start())
		{
			m_thread = 0;
			m_asyncTx =
			m_asyncRx = false;
		}
	}

	s_timer.start();
}

OnlinePeer2PeerProvider::~OnlinePeer2PeerProvider()
{
	if (m_thread)
	{
		m_thread->stop();
		ThreadManager::getInstance().destroy(m_thread);
	}
}

bool OnlinePeer2PeerProvider::update()
{
	if (s_timer.getElapsedTime() >= m_whenUpdate)
	{
		RefArray< IUser > users;
		T_MEASURE_STATEMENT(m_lobby->getParticipants(users), 0.001);

		// Add new users which have entered the lobby.
		for (RefArray< IUser >::iterator i = users.begin(); i != users.end(); ++i)
		{
			if (std::find_if(m_users.begin(), m_users.end(), P2PUserFindPred(*i)) == m_users.end())
			{
				P2PUser p2pu;
				p2pu.user = *i;
				p2pu.timeout = 0;
				m_users.push_back(p2pu);
			}
		}

		// Increment timeout counter for users which have left.
		for (std::vector< P2PUser >::iterator i = m_users.begin(); i != m_users.end(); ++i)
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
		std::vector< P2PUser >::iterator i = std::remove_if(m_users.begin(), m_users.end(), P2PUserTimeoutPred(4));
		m_users.erase(i, m_users.end());

		// Cache primary handle.
		T_MEASURE_STATEMENT(m_primaryHandle = net::net_handle_t(m_lobby->getOwner()->getGlobalId()), 0.0005);
		m_whenUpdate = s_timer.getElapsedTime() + 0.5;
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

bool OnlinePeer2PeerProvider::setPrimaryPeerHandle(net::net_handle_t node)
{
	for (std::vector< P2PUser >::const_iterator i = m_users.begin(); i != m_users.end(); ++i)
	{
		if (i->user->getGlobalId() == node)
		{
			log::info << L"[Online P2P] Migrating primary token to peer " << node << L"..." << Endl;
			return m_lobby->setOwner(i->user);
		}
	}
	log::error << L"[Online P2P] Failed to migrate primary token to peer " << node << L"." << Endl;
	return false;
}

net::net_handle_t OnlinePeer2PeerProvider::getPrimaryPeerHandle() const
{
	return m_primaryHandle;
}

bool OnlinePeer2PeerProvider::send(net::net_handle_t node, const void* data, int32_t size)
{
	for (std::vector< P2PUser >::iterator i = m_users.begin(); i != m_users.end(); ++i)
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
			if (nrecv > 0 && fromUser != 0)
			{
				if (std::find_if(m_users.begin(), m_users.end(), P2PUserFindPred(fromUser)) != m_users.end())
					outNode = net::net_handle_t(fromUser->getGlobalId());
			}
			else
			{
				nrecv = 0;
				fromUser = 0;
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
