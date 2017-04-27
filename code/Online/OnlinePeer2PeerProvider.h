/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_OnlinePeer2PeerProvider_H
#define traktor_online_OnlinePeer2PeerProvider_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/CircularVector.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Signal.h"
#include "Core/Timer/Timer.h"
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

	virtual bool update() T_OVERRIDE T_FINAL;

	virtual net::net_handle_t getLocalHandle() const T_OVERRIDE T_FINAL;

	virtual int32_t getPeerCount() const T_OVERRIDE T_FINAL;

	virtual net::net_handle_t getPeerHandle(int32_t index) const T_OVERRIDE T_FINAL;

	virtual std::wstring getPeerName(int32_t index) const T_OVERRIDE T_FINAL;

	virtual Object* getPeerUser(int32_t index) const T_OVERRIDE T_FINAL;

	virtual bool setPrimaryPeerHandle(net::net_handle_t node) T_OVERRIDE T_FINAL;

	virtual net::net_handle_t getPrimaryPeerHandle() const T_OVERRIDE T_FINAL;

	virtual bool send(net::net_handle_t node, const void* data, int32_t size) T_OVERRIDE T_FINAL;

	virtual int32_t recv(void* data, int32_t size, net::net_handle_t& outNode) T_OVERRIDE T_FINAL;

private:
	Ref< ISessionManager > m_sessionManager;
	Ref< ILobby > m_lobby;
	Timer m_timer;
	AlignedVector< P2PUser > m_users;
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
