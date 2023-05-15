/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Ref.h"
#include "Jungle/IPeer2PeerProvider.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_JUNGLE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::jungle
{

class T_DLLCLASS MeasureP2PProvider : public IPeer2PeerProvider
{
	T_RTTI_CLASS;

public:
	explicit MeasureP2PProvider(IPeer2PeerProvider* provider);

	virtual bool update() override final;

	virtual net_handle_t getLocalHandle() const override final;

	virtual int32_t getPeerCount() const override final;

	virtual net_handle_t getPeerHandle(int32_t index) const override final;

	virtual std::wstring getPeerName(int32_t index) const override final;

	virtual Object* getPeerUser(int32_t index) const override final;

	virtual bool setPrimaryPeerHandle(net_handle_t node) override final;

	virtual net_handle_t getPrimaryPeerHandle() const override final;

	virtual bool send(net_handle_t node, const void* data, int32_t size) override final;

	virtual int32_t recv(void* data, int32_t size, net_handle_t& outNode) override final;

	float getSendBitsPerSecond() const;

	float getRecvBitsPerSecond() const;

private:
	Ref< IPeer2PeerProvider > m_provider;
	double m_time;
	int32_t m_sentBytes;
	int32_t m_recvBytes;
	double m_sentBps;
	double m_recvBps;
};

}
