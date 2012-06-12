#include "Online/ILobby.h"
#include "Online/ISessionManager.h"
#include "Online/IUser.h"
#include "Parade/Network/OnlineReplicatorPeers.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.OnlineReplicatorPeers", OnlineReplicatorPeers, IReplicatorPeers)

bool OnlineReplicatorPeers::create(
	online::ISessionManager* sessionManager,
	online::ILobby* lobby
)
{
	m_sessionManager = sessionManager;
	m_lobby = lobby;
	m_users = m_lobby->getParticipants();
	return true;
}

void OnlineReplicatorPeers::destroy()
{
	m_lobby = 0;
}

uint32_t OnlineReplicatorPeers::getPeerCount() const
{
	return m_users.size();
}

bool OnlineReplicatorPeers::receiveAnyPending()
{
	return m_sessionManager->haveP2PData();
}

bool OnlineReplicatorPeers::receive(void* data, uint32_t size, uint32_t& outFromPeer)
{
	Ref< online::IUser > fromUser;
	if (!m_sessionManager->receiveP2PData(data, size, fromUser))
		return false;

	RefArray< online::IUser >::iterator i = std::find(m_users.begin(), m_users.end(), fromUser);
	if (i == m_users.end())
		return false;

	outFromPeer = std::distance(m_users.begin(), i);
	return true;
}

bool OnlineReplicatorPeers::sendReady(uint32_t peerId)
{
	return peerId < m_users.size();
}

bool OnlineReplicatorPeers::send(uint32_t peerId, const void* data, uint32_t size, bool reliable)
{
	T_ASSERT (peerId < m_users.size());
	T_ASSERT (size < 1200);
	return m_users[peerId]->sendP2PData(data, size, reliable);
}

	}
}
