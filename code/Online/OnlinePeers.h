#ifndef traktor_online_OnlinePeers_H
#define traktor_online_OnlinePeers_H

#include "Core/Containers/SmallMap.h"
#include "Net/Replication/IReplicatorPeers.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
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

class T_DLLCLASS OnlinePeers : public net::IReplicatorPeers
{
	T_RTTI_CLASS;

public:
	OnlinePeers(ISessionManager* sessionManager, ILobby* lobby);

	virtual void destroy();

	virtual bool update();

	virtual net::handle_t getHandle() const;

	virtual std::wstring getName() const;

	virtual net::handle_t getPrimaryPeerHandle() const;

	virtual bool setPrimaryPeerHandle(net::handle_t handle);

	virtual uint32_t getPeers(std::vector< PeerInfo >& outPeers) const;

	virtual int32_t receive(void* data, int32_t size, net::handle_t& outFromHandle);

	virtual bool send(net::handle_t handle, const void* data, int32_t size, bool reliable);

private:
	Ref< ISessionManager > m_sessionManager;
	Ref< ILobby > m_lobby;
	SmallMap< uint64_t, net::handle_t > m_idMap;
	SmallMap< net::handle_t, Ref< IUser > > m_userMap;
	net::handle_t m_handle;

	net::handle_t getFreeHandle();
};

	}
}

#endif	// traktor_online_OnlinePeers_H
