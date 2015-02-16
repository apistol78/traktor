#ifndef traktor_online_OnlinePeer2PeerProvider_H
#define traktor_online_OnlinePeer2PeerProvider_H

#include "Core/Containers/CircularVector.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Signal.h"
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

class Thread;

	namespace online
	{

class ILobby;
class ISessionManager;
class IUser;

class T_DLLCLASS OnlinePeer2PeerProvider : public net::IPeer2PeerProvider
{
	T_RTTI_CLASS;

public:
	struct P2PUser
	{
		Ref< IUser > user;
		int32_t timeout;
	};

	struct RxTxData
	{
		Ref< IUser > user;
		uint32_t size;
		uint8_t data[1200];
	};

	OnlinePeer2PeerProvider(ISessionManager* sessionManager, ILobby* lobby, bool asyncTx, bool asyncRx);

	virtual ~OnlinePeer2PeerProvider();

	virtual bool update();

	virtual net::net_handle_t getLocalHandle() const;

	virtual int32_t getPeerCount() const;

	virtual net::net_handle_t getPeerHandle(int32_t index) const;

	virtual std::wstring getPeerName(int32_t index) const;

	virtual Object* getPeerUser(int32_t index) const;

	virtual bool setPrimaryPeerHandle(net::net_handle_t node);

	virtual net::net_handle_t getPrimaryPeerHandle() const;

	virtual bool send(net::net_handle_t node, const void* data, int32_t size);

	virtual int32_t recv(void* data, int32_t size, net::net_handle_t& outNode);

private:
	Ref< ISessionManager > m_sessionManager;
	Ref< ILobby > m_lobby;
	std::vector< P2PUser > m_users;
	net::net_handle_t m_localHandle;
	net::net_handle_t m_primaryHandle;
	double m_whenUpdate;
	bool m_asyncTx;
	bool m_asyncRx;
	Thread* m_thread;
	Semaphore m_rxQueueLock;
	Semaphore m_txQueueLock;
	Signal m_txQueueSignal;
	AtomicRefCount m_rxQueuePending;
	CircularVector< RxTxData, 128 > m_rxQueue;
	CircularVector< RxTxData, 128 > m_txQueue;

	void transmissionThread();
};

	}
}

#endif	// traktor_online_OnlinePeer2PeerProvider_H
