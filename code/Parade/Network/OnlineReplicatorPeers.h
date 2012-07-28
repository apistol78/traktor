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

	virtual void update();

	virtual uint32_t getPeerHandles(std::vector< handle_t >& outPeerHandles) const;

	virtual std::wstring getPeerName(handle_t handle) const;

	virtual bool receiveAnyPending();

	virtual bool receive(void* data, uint32_t size, handle_t& outFromHandle);

	virtual bool sendReady(handle_t handle);

	virtual bool send(handle_t handle, const void* data, uint32_t size, bool reliable);

private:
	Ref< online::ISessionManager > m_sessionManager;
	Ref< online::ILobby > m_lobby;
	RefArray< online::IUser > m_users;
};

	}
}

#endif	// traktor_parade_OnlineReplicatorPeers_H
