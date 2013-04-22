#ifndef traktor_net_OnlineReplicatorPeers_H
#define traktor_net_OnlineReplicatorPeers_H

#include "Core/Containers/SmallMap.h"
#include "Net/Replication/IReplicatorPeers.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace online
	{

class ILobby;
class ISessionManager;
class IUser;

	}

	namespace net
	{

class T_DLLCLASS OnlineReplicatorPeers : public IReplicatorPeers
{
	T_RTTI_CLASS;

public:
	OnlineReplicatorPeers();

	bool create(
		online::ISessionManager* sessionManager,
		online::ILobby* lobby
	);

	virtual void destroy();

	virtual int32_t update();

	virtual std::wstring getName() const;

	virtual uint64_t getGlobalId() const;

	virtual handle_t getPrimaryPeerHandle() const;

	virtual uint32_t getPeerHandles(std::vector< handle_t >& outPeerHandles) const;

	virtual std::wstring getPeerName(handle_t handle) const;

	virtual uint64_t getPeerGlobalId(handle_t handle) const;

	virtual int32_t receive(void* data, int32_t size, handle_t& outFromHandle);

	virtual bool send(handle_t handle, const void* data, int32_t size, bool reliable);

private:
	Ref< online::ISessionManager > m_sessionManager;
	Ref< online::ILobby > m_lobby;
	RefArray< online::IUser > m_users;
	SmallMap< uint64_t, online::IUser* > m_userMap;
	float m_timeUntilQuery;
};

	}
}

#endif	// traktor_net_OnlineReplicatorPeers_H
