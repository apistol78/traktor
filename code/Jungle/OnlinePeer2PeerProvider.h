/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/CircularVector.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Signal.h"
#include "Core/Timer/Timer.h"
#include "Jungle/IPeer2PeerProvider.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_JUNGLE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Thread;

}

namespace traktor::online
{

class ILobby;
class ISessionManager;
class IUser;

}

namespace traktor::jungle
{

class T_DLLCLASS OnlinePeer2PeerProvider : public IPeer2PeerProvider
{
	T_RTTI_CLASS;

public:
	OnlinePeer2PeerProvider(online::ISessionManager* sessionManager, online::ILobby* lobby, bool asyncTx, bool asyncRx);

	virtual ~OnlinePeer2PeerProvider();

	virtual bool update() override final;

	virtual jungle::net_handle_t getLocalHandle() const override final;

	virtual int32_t getPeerCount() const override final;

	virtual jungle::net_handle_t getPeerHandle(int32_t index) const override final;

	virtual std::wstring getPeerName(int32_t index) const override final;

	virtual Object* getPeerUser(int32_t index) const override final;

	virtual bool setPrimaryPeerHandle(jungle::net_handle_t node) override final;

	virtual jungle::net_handle_t getPrimaryPeerHandle() const override final;

	virtual bool send(jungle::net_handle_t node, const void* data, int32_t size) override final;

	virtual int32_t recv(void* data, int32_t size, jungle::net_handle_t& outNode) override final;

private:
	struct P2PUser
	{
		Ref< online::IUser > user;
		int32_t timeout;
	};

	struct RxTxData
	{
		Ref< online::IUser > user;
		uint32_t size;
		uint8_t data[1200];
	};
	
	Ref< online::ISessionManager > m_sessionManager;
	Ref< online::ILobby > m_lobby;
	Timer m_timer;
	AlignedVector< P2PUser > m_users;
	jungle::net_handle_t m_localHandle;
	jungle::net_handle_t m_primaryHandle;
	double m_whenUpdate;
	bool m_asyncTx;
	bool m_asyncRx;
	Thread* m_thread;
	Semaphore m_rxQueueLock;
	Semaphore m_txQueueLock;
	Signal m_txQueueSignal;
	std::atomic< int32_t > m_rxQueuePending;
	CircularVector< RxTxData, 128 > m_rxQueue;
	CircularVector< RxTxData, 128 > m_txQueue;

	void transmissionThread();
};

}
