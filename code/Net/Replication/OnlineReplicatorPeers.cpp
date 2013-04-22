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

int32_t OnlineReplicatorPeers::update()
{
	m_timeUntilQuery -= 1.0f / 60.0f;
	if (m_timeUntilQuery > 0.0f)
		return 0;

	// Get users still in lobby.
	m_users = m_lobby->getParticipants();
	
	// Create map from tag to user instance.
	m_userMap.clear();
	for (RefArray< online::IUser >::const_iterator i = m_users.begin(); i != m_users.end(); ++i)
	{
		uint64_t handle = (*i)->getGlobalId();
		m_userMap[handle] = *i;
	}

	m_timeUntilQuery = 1.0f;
	return 0;
}

std::wstring OnlineReplicatorPeers::getName() const
{
	std::wstring name;
	m_sessionManager->getUser()->getName(name);
	return name;
}

uint64_t OnlineReplicatorPeers::getGlobalId() const
{
	return m_sessionManager->getUser()->getGlobalId();
}

handle_t OnlineReplicatorPeers::getPrimaryPeerHandle() const
{
	return handle_t(m_lobby->getOwner()->getGlobalId());
}

uint32_t OnlineReplicatorPeers::getPeerHandles(std::vector< handle_t >& outPeerHandles) const
{
	outPeerHandles.reserve(m_userMap.size());
	for (SmallMap< uint64_t, online::IUser* >::const_iterator i = m_userMap.begin(); i != m_userMap.end(); ++i)
	{
		if (i->second->isP2PAllowed())
			outPeerHandles.push_back(handle_t(i->first));
	}
	return m_userMap.size();
}

std::wstring OnlineReplicatorPeers::getPeerName(handle_t handle) const
{
	SmallMap< uint64_t, online::IUser* >::const_iterator i = m_userMap.find(handle);
	if (i == m_userMap.end() || i->second == 0)
		return L"";

	std::wstring name;
	if (i->second->getName(name))
		return name;
	else
		return L"";
}

uint64_t OnlineReplicatorPeers::getPeerGlobalId(handle_t handle) const
{
	SmallMap< uint64_t, online::IUser* >::const_iterator i = m_userMap.find(handle);
	if (i == m_userMap.end() || i->second == 0)
		return 0;

	return i->second->getGlobalId();
}

int32_t OnlineReplicatorPeers::receive(void* data, int32_t size, handle_t& outFromHandle)
{
	Ref< online::IUser > fromUser;
	int32_t nrecv;

	if (!m_sessionManager->haveP2PData())
		return 0;
	
	nrecv = m_sessionManager->receiveP2PData(data, size, fromUser);
	if (!nrecv)
		return 0;

	if (!fromUser)
		return 0;

	outFromHandle = handle_t(fromUser->getGlobalId());
	return nrecv;
}

bool OnlineReplicatorPeers::send(handle_t handle, const void* data, int32_t size, bool reliable)
{
	T_ASSERT (size < 1200);
	T_ASSERT (!reliable);

	SmallMap< uint64_t, online::IUser* >::const_iterator i = m_userMap.find(handle);
	if (i == m_userMap.end() || i->second == 0)
		return false;

	return i->second->sendP2PData(data, size);
}

	}
}
