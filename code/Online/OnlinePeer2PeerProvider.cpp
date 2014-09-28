#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Timer/Timer.h"
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

#define T_WIDEN_X(x) L ## x
#define T_WIDEN(x) T_WIDEN_X(x)

#define T_MEASURE(statement, maxDuration) \
	{ \
		double start = s_timer.getElapsedTime(); \
		(statement); \
		double end = s_timer.getElapsedTime(); \
		if ((end - start) > maxDuration) \
			log::warning << L"Statement \"" << T_WIDEN(#statement) << L"\" exceeded max " << int32_t(maxDuration * 1000.0) << L" ms, " << int32_t((end - start) * 1000.0) << L" ms" << Endl; \
	}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.OnlinePeer2PeerProvider", OnlinePeer2PeerProvider, net::IPeer2PeerProvider)

OnlinePeer2PeerProvider::OnlinePeer2PeerProvider(ISessionManager* sessionManager, ILobby* lobby)
:	m_sessionManager(sessionManager)
,	m_lobby(lobby)
,	m_localHandle(sessionManager->getUser()->getGlobalId())
,	m_primaryHandle(0)
,	m_whenUpdate(0.0)
{
	Ref< IUser > fromUser;
	uint8_t data[1600];

	// Purge pending data.
	while(m_sessionManager->receiveP2PData(data, sizeof(data), fromUser) > 0)
		;

	s_timer.start();
}

bool OnlinePeer2PeerProvider::update()
{
	if (s_timer.getElapsedTime() >= m_whenUpdate)
	{
		T_MEASURE(m_lobby->getParticipants(m_users), 0.002)
		T_MEASURE(m_primaryHandle = net::net_handle_t(m_lobby->getOwner()->getGlobalId()), 0.002);
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
		T_MEASURE(m_sessionManager->getUser()->getName(name), 0.001)
	else
		T_MEASURE(m_users[index - 1]->getName(name), 0.001)
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
			bool result = false;
			T_MEASURE(result = (*i)->sendP2PData(data, size, false), 0.002);
			return result;
		}
	}
	return false;
}

int32_t OnlinePeer2PeerProvider::recv(void* data, int32_t size, net::net_handle_t& outNode)
{
	Ref< IUser > fromUser;

	uint32_t nrecv = 0;
	T_MEASURE(nrecv = m_sessionManager->receiveP2PData(data, size, fromUser), 0.002);
	if (!nrecv || !fromUser)
		return 0;

	outNode = net::net_handle_t(fromUser->getGlobalId());
	return int32_t(nrecv);
}

bool OnlinePeer2PeerProvider::pendingRecv()
{
	bool result = false;
	T_MEASURE(result = m_sessionManager->haveP2PData(), 0.002);
	return result;
}

	}
}
