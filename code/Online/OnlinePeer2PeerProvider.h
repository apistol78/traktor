#ifndef traktor_online_OnlinePeer2PeerProvider_H
#define traktor_online_OnlinePeer2PeerProvider_H

#include "Net/Replication/IPeer2PeerProvider.h"

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

class T_DLLCLASS OnlinePeer2PeerProvider : public net::IPeer2PeerProvider
{
	T_RTTI_CLASS;

public:
	OnlinePeer2PeerProvider(ISessionManager* sessionManager, ILobby* lobby);

	virtual bool update();

	virtual net::net_handle_t getLocalHandle() const;

	virtual int32_t getPeerCount() const;

	virtual net::net_handle_t getPeerHandle(int32_t index) const;

	virtual std::wstring getPeerName(int32_t index) const;

	virtual bool setPrimaryPeerHandle(net::net_handle_t node);

	virtual net::net_handle_t getPrimaryPeerHandle() const;

	virtual bool send(net::net_handle_t node, const void* data, int32_t size);

	virtual int32_t recv(void* data, int32_t size, net::net_handle_t& outNode);

	virtual bool pendingRecv();

private:
	Ref< ISessionManager > m_sessionManager;
	Ref< ILobby > m_lobby;
	RefArray< IUser > m_users;
	net::net_handle_t m_localHandle;
	net::net_handle_t m_primaryHandle;
	double m_whenUpdate;
};

	}
}

#endif	// traktor_online_OnlinePeer2PeerProvider_H
