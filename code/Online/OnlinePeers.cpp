#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Online/ILobby.h"
#include "Online/ISessionManager.h"
#include "Online/IUser.h"
#include "Online/OnlinePeers.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

#define T_ONLINE_PEERS_DEBUG(x) traktor::log::info << x << traktor::Endl

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.OnlinePeers", OnlinePeers, net::IReplicatorPeers)

OnlinePeers::OnlinePeers(ISessionManager* sessionManager, ILobby* lobby)
:	m_sessionManager(sessionManager)
,	m_lobby(lobby)
,	m_handle(0x00)
{
}

void OnlinePeers::destroy()
{
	m_lobby = 0;
}

int32_t OnlinePeers::update()
{
	// Get lobby users.
	RefArray< IUser > users = m_lobby->getParticipants();

	// Am I lobby owner then I need to "baptize" new peers.
	if (m_lobby->getOwner() == m_sessionManager->getUser())
	{
		if (m_handle == 0x00)
		{
			uint64_t globalId = m_sessionManager->getUser()->getGlobalId();
			T_ASSERT (globalId != 0);

			m_handle = getFreeHandle();
			if (m_handle != 0x00)
			{
				if (m_lobby->setMetaValue(L"__ID__" + toString< uint64_t >(globalId), toString< int32_t >(m_handle))->succeeded())
				{
					T_ONLINE_PEERS_DEBUG(L"OK: Assigned my global handle " << int32_t(m_handle));
					m_idMap[globalId] = m_handle;
				}
				else
				{
					T_ONLINE_PEERS_DEBUG(L"ERROR: Unable to assign global handle to myself; set meta value failed");
					m_handle = 0x00;
				}
			}
			else
				T_ONLINE_PEERS_DEBUG(L"ERROR: Unable to assign global handle to myself; no free handles");
		}

		for (RefArray< IUser >::const_iterator i = users.begin(); i != users.end(); ++i)
		{
			uint64_t globalId = (*i)->getGlobalId();
			T_ASSERT (globalId != 0);

			SmallMap< uint64_t, net::handle_t >::const_iterator j = m_idMap.find(globalId);
			if (j == m_idMap.end())
			{
				// New user connected to lobby.
				T_ONLINE_PEERS_DEBUG(L"OK: New user in lobby; need to baptize user");
				uint8_t handle = getFreeHandle();
				if (handle != 0x00)
				{
					if (m_lobby->setMetaValue(L"__ID__" + toString< uint64_t >(globalId), toString< int32_t >(handle))->succeeded())
					{
						T_ONLINE_PEERS_DEBUG(L"OK: Assigned global handle " << int32_t(handle) << L" to peer");
						m_idMap[globalId] = handle;
						m_userMap[handle] = *i;
					}
					else
						T_ONLINE_PEERS_DEBUG(L"ERROR: Unable to assign global handle to peer; set meta value failed");
				}
				else
					T_ONLINE_PEERS_DEBUG(L"ERROR: Unable to assign global handle to peer; no free handles");
			}
			else if (m_userMap.find(j->second) == m_userMap.end())
			{
				// User re-connected to lobby; use same id as before.
				T_ONLINE_PEERS_DEBUG(L"OK: User re-connected in lobby; need to baptize user");
				uint8_t handle = j->second;
				if (handle != 0x00)
				{
					if (m_lobby->setMetaValue(L"__ID__" + toString< uint64_t >(globalId), toString< int32_t >(handle))->succeeded())
					{
						T_ONLINE_PEERS_DEBUG(L"OK: Assigned global handle " << int32_t(handle) << L" to peer");
						m_idMap[globalId] = handle;
						m_userMap[handle] = *i;
					}
					else
						T_ONLINE_PEERS_DEBUG(L"ERROR: Unable to assign global handle to peer; set meta value failed");
				}
				else
					T_ONLINE_PEERS_DEBUG(L"ERROR: Unable to assign global handle to peer; no free handles");
			}
		}
	}
	// If I'm not lobby owner then I need to read peer ID from user's meta.
	else
	{
		if (m_handle == 0x00)
		{
			uint64_t globalId = m_sessionManager->getUser()->getGlobalId();
			T_ASSERT (globalId != 0);

			std::wstring tmp;
			if (m_lobby->getMetaValue(L"__ID__" + toString< uint64_t >(globalId), tmp))
			{
				m_handle = parseString< uint8_t >(tmp);
				if (m_handle != 0x00)
				{
					T_ONLINE_PEERS_DEBUG(L"OK: Got my global handle " << int32_t(m_handle));
					m_idMap[globalId] = m_handle;
				}
			}
			else
				T_ONLINE_PEERS_DEBUG(L"ERROR: Unable to get global handle to myself; get meta value failed");
		}

		for (RefArray< IUser >::const_iterator i = users.begin(); i != users.end(); ++i)
		{
			uint64_t globalId = (*i)->getGlobalId();
			T_ASSERT (globalId != 0);

			SmallMap< uint64_t, net::handle_t >::const_iterator j = m_idMap.find(globalId);
			if (j == m_idMap.end())
			{
				// New user connected to lobby.
				T_ONLINE_PEERS_DEBUG(L"OK: New user in lobby");
				std::wstring tmp;
				if (m_lobby->getMetaValue(L"__ID__" + toString< uint64_t >(globalId), tmp))
				{
					uint8_t handle = parseString< uint8_t >(tmp);
					if (handle != 0x00)
					{
						T_ONLINE_PEERS_DEBUG(L"OK: Get peer global handle " << int32_t(handle));
						m_idMap[globalId] = handle;
						m_userMap[handle] = *i;
					}
				}
				else
					T_ONLINE_PEERS_DEBUG(L"ERROR: Unable to get global handle to peer; get meta value failed");
			}
			else if (m_userMap.find(j->second) == m_userMap.end())
			{
				// User re-connected to lobby; use same id as before.
				T_ONLINE_PEERS_DEBUG(L"OK: User re-connected in lobby");
				uint8_t handle = j->second;
				if (handle != 0x00)
					m_userMap[handle] = *i;
				else
					T_ONLINE_PEERS_DEBUG(L"ERROR: Unable to get global handle to peer; invalid handle");
			}
		}
	}

	// Erase entries in user map which no longer is in lobby.
	for (SmallMap< net::handle_t, Ref< IUser > >::iterator i = m_userMap.begin(); i != m_userMap.end(); )
	{
		if (std::find(users.begin(), users.end(), i->second) != users.end())
			++i;
		else
			i = m_userMap.erase(i);
	}

	return 0;
}

void OnlinePeers::setStatus(uint8_t status)
{
	m_lobby->setParticipantMetaValue(L"__STATUS__", toString< int32_t >(status));
}

net::handle_t OnlinePeers::getHandle() const
{
	return m_handle;
}

std::wstring OnlinePeers::getName() const
{
	std::wstring name;
	m_sessionManager->getUser()->getName(name);
	return name;
}

net::handle_t OnlinePeers::getPrimaryPeerHandle() const
{
	SmallMap< uint64_t, net::handle_t >::const_iterator i = m_idMap.find(m_lobby->getOwner()->getGlobalId());
	return i != m_idMap.end() ? i->second : 0x00;
}

bool OnlinePeers::setPrimaryPeerHandle(net::handle_t handle)
{
	SmallMap< net::handle_t, Ref< IUser > >::const_iterator i = m_userMap.find(handle);
	if (i == m_userMap.end() || i->second == 0)
		return false;

	return m_lobby->setOwner(i->second);
}

uint32_t OnlinePeers::getPeers(std::vector< PeerInfo >& outPeers) const
{
	PeerInfo info;
	std::wstring tmp;

	outPeers.resize(0);
	outPeers.reserve(m_userMap.size());

	for (SmallMap< net::handle_t, Ref< IUser > >::const_iterator i = m_userMap.begin(); i != m_userMap.end(); ++i)
	{
		info.handle = i->first;
		info.relayed = false;

		i->second->getName(info.name);

		if (m_lobby->getParticipantMetaValue(i->second, L"__STATUS__", tmp))
			info.status = parseString< int32_t >(tmp);

		outPeers.push_back(info);
	}

	return outPeers.size();
}

int32_t OnlinePeers::receive(void* data, int32_t size, net::handle_t& outFromHandle)
{
	Ref< IUser > fromUser;
	int32_t nrecv;

	if (!m_sessionManager->haveP2PData())
		return 0;
	
	nrecv = m_sessionManager->receiveP2PData(data, size, fromUser);
	if (!nrecv)
		return 0;

	if (!fromUser)
		return 0;

	SmallMap< uint64_t, uint8_t >::const_iterator i = m_idMap.find(fromUser->getGlobalId());
	if (i == m_idMap.end())
		return 0;

	outFromHandle = i->second;
	return nrecv;
}

bool OnlinePeers::send(net::handle_t handle, const void* data, int32_t size, bool reliable)
{
	T_ASSERT (size < 1200);
	T_ASSERT (!reliable);

	SmallMap< net::handle_t, Ref< IUser > >::const_iterator i = m_userMap.find(handle);
	if (i == m_userMap.end() || i->second == 0)
		return false;

	return i->second->sendP2PData(data, size);
}

uint8_t OnlinePeers::getFreeHandle()
{
	net::handle_t handle = 0x01;
	for (;;)
	{
		bool handleInUse = false;
		for (SmallMap< uint64_t, net::handle_t >::const_iterator j = m_idMap.begin(); j != m_idMap.end(); ++j)
		{
			if (handle == j->second)
			{
				handleInUse = true;
				break;
			}
		}
		if (handleInUse)
			++handle;
		else
			break;
	}
	return handle;
}

	}
}
