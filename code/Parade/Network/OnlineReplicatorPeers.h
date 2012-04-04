#ifndef traktor_parade_OnlineReplicatorPeers_H
#define traktor_parade_OnlineReplicatorPeers_H

#include "Parade/Network/IReplicatorPeers.h"

namespace traktor
{
	namespace online
	{

class ILobby;
class ISessionManager;
class IUser;

	}

	namespace parade
	{

class OnlineReplicatorPeers : public IReplicatorPeers
{
	T_RTTI_CLASS;

public:
	bool create(
		online::ISessionManager* sessionManager,
		online::ILobby* lobby
	);

	virtual void destroy();

	virtual uint32_t getPeerCount() const;

	virtual bool receiveAnyPending();

	virtual bool receive(void* data, uint32_t size, uint32_t& outFromPeer);

	virtual bool sendReady(uint32_t peerId);

	virtual bool send(uint32_t peerId, const void* data, uint32_t size, bool reliable);

private:
	Ref< online::ISessionManager > m_sessionManager;
	Ref< online::ILobby > m_lobby;
	RefArray< online::IUser > m_users;
};

	}
}

#endif	// traktor_parade_OnlineReplicatorPeers_H
