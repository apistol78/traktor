#include "Online/ILobby.h"
#include "Online/ISessionManager.h"
#include "Online/IUser.h"
#include "Net/Replication/OnlineReplicatorPeers.h"

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.OnlineReplicatorPeers", OnlineReplicatorPeers, IReplicatorPeers)

OnlineReplicatorPeers::OnlineReplicatorPeers()
:	m_timeUntilQuery(0.0f)
{
}

bool OnlineReplicatorPeers::create(
	online::ISessionManager* sessionManager,
	online::ILobby* lobby
)
{
	m_sessionManager = sessionManager;
	m_lobby = lobby;
	m_timeUntilQuery = 0.0f;
	return true;
}

void OnlineReplicatorPeers::destroy()
{
	m_lobby = 0;
}

void OnlineReplicatorPeers::update()
{
	m_timeUntilQuery -= 1.0f / 60.0f;
	if (m_timeUntilQuery > 0.0f)
		return;

	// Get users still in lobby.
	m_users = m_lobby->getParticipants();
	
	// Create map from tag to user instance.
	m_userMap.clear();
	for (RefArray< online::IUser >::const_iterator i = m_users.begin(); i != m_users.end(); ++i)
	{
		int32_t tag = (*i)->getTag();
		m_userMap[tag] = *i;
	}

	m_timeUntilQuery = 1.0f;
}

uint32_t OnlineReplicatorPeers::getPeerHandles(std::vector< handle_t >& outPeerHandles) const
{
	outPeerHandles.reserve(m_userMap.size());
	for (SmallMap< int32_t, online::IUser* >::const_iterator i = m_userMap.begin(); i != m_userMap.end(); ++i)
		outPeerHandles.push_back(handle_t(i->first));
	return m_userMap.size();
}

std::wstring OnlineReplicatorPeers::getPeerName(handle_t handle) const
{
	SmallMap< int32_t, online::IUser* >::const_iterator i = m_userMap.find(int32_t(handle));
	if (i == m_userMap.end() || i->second == 0)
		return L"";

	std::wstring name;
	if (i->second->getName(name))
		return name;
	else
		return L"";
}

int32_t OnlineReplicatorPeers::receive(void* data, int32_t size, handle_t& outFromHandle)
{
	Ref< online::IUser > fromUser;
	int32_t nrecv;
	
	nrecv = m_sessionManager->receiveP2PData(data, size, fromUser);
	if (!nrecv)
		return -1;

	if (!fromUser)
		return -1;

	outFromHandle = handle_t(fromUser->getTag());
	return nrecv;
}

bool OnlineReplicatorPeers::send(handle_t handle, const void* data, int32_t size, bool reliable)
{
	T_ASSERT (size < 1200);
	T_ASSERT (!reliable);

	SmallMap< int32_t, online::IUser* >::const_iterator i = m_userMap.find(int32_t(handle));
	if (i == m_userMap.end() || i->second == 0)
		return false;

	return i->second->sendP2PData(data, size);
}

bool OnlineReplicatorPeers::isPrimary() const
{
	return m_lobby->isOwner();
}

	}
}
