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
	return true;
}

void OnlineReplicatorPeers::destroy()
{
	m_lobby = 0;
}

void OnlineReplicatorPeers::update()
{
	// Get users still in lobby.
	m_users = m_lobby->getParticipants();
	
	// Create map from tag to user instance.
	m_userMap.clear();
	for (RefArray< online::IUser >::const_iterator i = m_users.begin(); i != m_users.end(); ++i)
	{
		int32_t tag = (*i)->getTag();
		m_userMap[tag] = *i;
	}
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

bool OnlineReplicatorPeers::receiveAnyPending()
{
	return m_sessionManager->haveP2PData();
}

bool OnlineReplicatorPeers::receive(void* data, uint32_t size, handle_t& outFromHandle)
{
	Ref< online::IUser > fromUser;
	if (!m_sessionManager->receiveP2PData(data, size, fromUser))
		return false;

	if (!fromUser)
		return false;

	outFromHandle = handle_t(fromUser->getTag());
	return true;
}

bool OnlineReplicatorPeers::sendReady(handle_t handle)
{
	return true;
}

bool OnlineReplicatorPeers::send(handle_t handle, const void* data, uint32_t size, bool reliable)
{
	T_ASSERT (size < 1200);

	SmallMap< int32_t, online::IUser* >::const_iterator i = m_userMap.find(int32_t(handle));
	if (i == m_userMap.end() || i->second == 0)
		return false;

	return i->second->sendP2PData(data, size, reliable);
}

bool OnlineReplicatorPeers::isPrimary() const
{
	return m_lobby->isOwner();
}

	}
}
