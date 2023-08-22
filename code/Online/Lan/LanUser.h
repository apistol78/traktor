/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Online/Provider/IUserProvider.h"

//#define T_INTERNET_SIMULATION

namespace traktor::net
{

class DiscoveryManager;
class UdpSocket;

}

namespace traktor::online
{

class LanUser : public IUserProvider
{
	T_RTTI_CLASS;

public:
	LanUser(net::DiscoveryManager* discoveryManager, net::UdpSocket* socket);

	virtual bool getName(uint64_t userHandle, std::wstring& outName) override final;

	virtual Ref< drawing::Image > getImage(uint64_t userHandle) const override final;

	virtual bool isFriend(uint64_t userHandle) override final;

	virtual bool isMemberOfGroup(uint64_t userHandle, const std::wstring& groupName) const override final;

	virtual bool joinGroup(uint64_t userHandle, const std::wstring& groupName) override final;

	virtual bool invite(uint64_t userHandle) override final;

	virtual bool setPresenceValue(uint64_t userHandle, const std::wstring& key, const std::wstring& value) override final;

	virtual bool getPresenceValue(uint64_t userHandle, const std::wstring& key, std::wstring& outValue) override final;

	virtual void setP2PEnable(uint64_t userHandle, bool enable) override final;

	virtual bool isP2PAllowed(uint64_t userHandle) const override final;

	virtual bool isP2PRelayed(uint64_t userHandle) const override final;

	virtual bool sendP2PData(uint64_t userHandle, const void* data, size_t size, bool reliable) override final;

	void update();

private:
	Ref< net::DiscoveryManager > m_discoveryManager;
	Ref< net::UdpSocket > m_socket;

#if defined(T_INTERNET_SIMULATION)
	struct Packet
	{
		double sendAt;
		uint64_t userHandle;
		uint8_t* data;
		size_t size;
	};

	std::list< Packet > m_packets;
#endif
};

}
