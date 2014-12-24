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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.OnlinePeer2PeerProvider", OnlinePeer2PeerProvider, net::IPeer2PeerProvider)

OnlinePeer2PeerProvider::OnlinePeer2PeerProvider(ISessionManager* sessionManager, ILobby* lobby)
:	m_sessionManager(sessionManager)
,	m_lobby(lobby)
,	m_localHandle(sessionManager->getUser()->getGlobalId())
,	m_primaryHandle(0)
,	m_whenUpdate(0.0)
,	m_thread(0)
{
	Ref< IUser > fromUser;
	uint8_t data[1600];

	// Purge pending data.
	while(m_sessionManager->receiveP2PData(data, sizeof(data), fromUser) > 0)
		;

	m_thread = ThreadManager::getInstance().create(makeFunctor(this, &OnlinePeer2PeerProvider::transmissionThread), L"Online P2P");
	m_thread->start();

	s_timer.start();
}

OnlinePeer2PeerProvider::~OnlinePeer2PeerProvider()
{
	m_thread->stop();
	ThreadManager::getInstance().destroy(m_thread);
}

bool OnlinePeer2PeerProvider::update()
{
	if (s_timer.getElapsedTime() >= m_whenUpdate)
	{
		T_MEASURE_STATEMENT(m_lobby->getParticipants(m_users), 0.001)
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
		return net::net_handle_t(m_users[index - 1]->getGlobalId());
}

std::wstring OnlinePeer2PeerProvider::getPeerName(int32_t index) const
{
	std::wstring name;
	if (index <= 0)
		T_MEASURE_STATEMENT(m_sessionManager->getUser()->getName(name), 0.001)
	else
		T_MEASURE_STATEMENT(m_users[index - 1]->getName(name), 0.001)
	return name;
}

bool OnlinePeer2PeerProvider::setPrimaryPeerHandle(net::net_handle_t node)
{
	for (RefArray< IUser >::const_iterator i = m_users.begin(); i != m_users.end(); ++i)
	{
		if ((*i)->getGlobalId() == node)
		{
			log::info << L"[Online P2P] Migrating primary token to peer " << node << L"..." << Endl;
			return m_lobby->setOwner(*i);
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
	for (RefArray< IUser >::iterator i = m_users.begin(); i != m_users.end(); ++i)
	{
		if ((*i)->getGlobalId() == node)
		{
			m_txQueueLock.wait();

			RxTxData& tx = m_txQueue.push_back();

			tx.user = *i;
			tx.size = size;
			std::memcpy(tx.data, data, size);

			m_txQueueSignal.set();
			m_txQueueLock.release();
			return true;
		}
	}
	return false;
}

int32_t OnlinePeer2PeerProvider::recv(void* data, int32_t size, net::net_handle_t& outNode)
{
	int32_t nrecv = 0;

	if (m_rxQueuePending <= 0)
		return 0;

	m_rxQueueLock.wait();
	if (!m_rxQueue.empty())
	{
		RxTxData& rx = m_rxQueue.front();

		nrecv = std::min< int32_t >(size, rx.size);
		std::memcpy(data, rx.data, nrecv);
		outNode = net::net_handle_t(rx.user->getGlobalId());

		m_rxQueue.pop_front();
		--m_rxQueuePending;
	}
	m_rxQueueLock.release();

	return nrecv;
}

void OnlinePeer2PeerProvider::transmissionThread()
{
	while (!m_thread->stopped())
	{
		if (m_sessionManager->haveP2PData())
		{
			RxTxData rx;
			rx.size = m_sessionManager->receiveP2PData(rx.data, sizeof(rx.data), rx.user);
			if (rx.size > 0 && rx.user)
			{
				m_rxQueueLock.wait();
				m_rxQueue.push_back(rx);
				++m_rxQueuePending;
				m_rxQueueLock.release();
			}
			continue;
		}

		if (m_txQueueSignal.wait(0))
		{
			RxTxData tx;

			m_txQueueLock.wait();
			tx = m_txQueue.front();
			m_txQueue.pop_front();
			if (m_txQueue.empty())
				m_txQueueSignal.reset();
			m_txQueueLock.release();

			tx.user->sendP2PData(tx.data, tx.size, false);
			continue;
		}

		m_thread->sleep(4);
	}
}

	}
}
