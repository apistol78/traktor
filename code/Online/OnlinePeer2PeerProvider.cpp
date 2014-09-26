#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Online/ILobby.h"
#include "Online/ISessionManager.h"
#include "Online/IUser.h"
#include "Online/OnlinePeer2PeerProvider.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.OnlinePeer2PeerProvider", OnlinePeer2PeerProvider, net::IPeer2PeerProvider)

OnlinePeer2PeerProvider::OnlinePeer2PeerProvider(ISessionManager* sessionManager, ILobby* lobby)
:	m_sessionManager(sessionManager)
,	m_lobby(lobby)
,	m_localHandle(0)
,	m_primaryHandle(0)
{
	Ref< IUser > fromUser;
	uint8_t data[1024];

	// Purge pending data.
	while(m_sessionManager->receiveP2PData(data, sizeof(data), fromUser) > 0)
		;
}

bool OnlinePeer2PeerProvider::update()
{
	m_users = m_lobby->getParticipants();
	m_localHandle = net::net_handle_t(m_sessionManager->getUser()->getGlobalId());
	m_primaryHandle = net::net_handle_t(m_lobby->getOwner()->getGlobalId());
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
		m_sessionManager->getUser()->getName(name);
	else
		m_users[index - 1]->getName(name);
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
			return (*i)->sendP2PData(data, size, false);
	}
	return false;
}

int32_t OnlinePeer2PeerProvider::recv(void* data, int32_t size, net::net_handle_t& outNode)
{
	Ref< IUser > fromUser;

	if (!m_sessionManager->haveP2PData())
		return 0;

	uint32_t nrecv = m_sessionManager->receiveP2PData(data, size, fromUser);
	if (!nrecv || !fromUser)
		return 0;

	outNode = net::net_handle_t(fromUser->getGlobalId());
	return int32_t(nrecv);
}

	}
}
