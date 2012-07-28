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
	m_users = m_lobby->getParticipants();
}

uint32_t OnlineReplicatorPeers::getPeerHandles(std::vector< handle_t >& outPeerHandles) const
{
	outPeerHandles.reserve(m_users.size());
	for (RefArray< online::IUser >::const_iterator i = m_users.begin(); i != m_users.end(); ++i)
	{
		T_ASSERT (*i);
		outPeerHandles.push_back(handle_t(*i));
	}
	return m_users.size();
}

std::wstring OnlineReplicatorPeers::getPeerName(handle_t handle) const
{
	online::IUser* user = reinterpret_cast< online::IUser* >(handle);
	T_ASSERT (user);

	std::wstring name;
	if (user->getName(name))
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

	RefArray< online::IUser >::iterator i = std::find(m_users.begin(), m_users.end(), fromUser);
	if (i == m_users.end())
		return false;

	outFromHandle = handle_t((*i).ptr());
	return true;
}

bool OnlineReplicatorPeers::sendReady(handle_t handle)
{
	return true;
}

bool OnlineReplicatorPeers::send(handle_t handle, const void* data, uint32_t size, bool reliable)
{
	online::IUser* user = reinterpret_cast< online::IUser* >(handle);
	T_ASSERT (user);
	T_ASSERT (size < 1200);

	return user->sendP2PData(data, size, reliable);
}

	}
}
