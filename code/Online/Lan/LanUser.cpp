/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cstring>
#include "Core/Math/Random.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Timer/Timer.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/UdpSocket.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/NetworkService.h"
#include "Online/Lan/LanUser.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

#if defined(T_INTERNET_SIMULATION)
Random s_random;
Timer s_timer;
#endif

const wchar_t* c_keyServiceTypeUser = L"U";
const wchar_t* c_keyUserHandle = L"UH";
const wchar_t* c_keyUserAddr = L"UA";
const wchar_t* c_keyUserPort = L"UP";

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.LocalUser", LanUser, IUserProvider)

LanUser::LanUser(net::DiscoveryManager* discoveryManager, net::UdpSocket* socket)
:	m_discoveryManager(discoveryManager)
,	m_socket(socket)
{
}

bool LanUser::getName(uint64_t userHandle, std::wstring& outName)
{
	outName = std::wstring(L"User_") + toString(int32_t(userHandle));
	return true;
}

Ref< drawing::Image > LanUser::getImage(uint64_t userHandle) const
{
	return 0;
}

bool LanUser::isFriend(uint64_t userHandle)
{
	return false;
}

bool LanUser::isMemberOfGroup(uint64_t userHandle, const std::wstring& groupName) const
{
	return false;
}

bool LanUser::joinGroup(uint64_t userHandle, const std::wstring& groupName)
{
	return false;
}

bool LanUser::invite(uint64_t userHandle)
{
	return false;
}

bool LanUser::setPresenceValue(uint64_t userHandle, const std::wstring& key, const std::wstring& value)
{
	return false;
}

bool LanUser::getPresenceValue(uint64_t userHandle, const std::wstring& key, std::wstring& outValue)
{
	return false;
}

void LanUser::setP2PEnable(uint64_t userHandle, bool enable)
{
}

bool LanUser::isP2PAllowed(uint64_t userHandle) const
{
	return true;
}

bool LanUser::isP2PRelayed(uint64_t userHandle) const
{
	return false;
}

bool LanUser::sendP2PData(uint64_t userHandle, const void* data, size_t size, bool reliable)
{
#if defined(T_INTERNET_SIMULATION)
	// Drop random packets of data.
	if ((s_random.next() % 10) == 0)
		return true;

	// Enqueue packets, at slight random to shuffle packets out of order.
	Packet p;
	p.sendAt = s_timer.getElapsedTime() + s_random.nextDouble() * 0.2;
	p.userHandle = userHandle;
	p.data = new uint8_t [size];
	p.size = size;

	std::memcpy(p.data, data, size);

	m_packets.push_back(p);
	return true;
#else
	RefArray< net::NetworkService > userServices;
	m_discoveryManager->findServices< net::NetworkService >(userServices);

	for (RefArray< net::NetworkService >::const_iterator i = userServices.begin(); i != userServices.end(); ++i)
	{
		if ((*i)->getType() != c_keyServiceTypeUser)
			continue;

		const PropertyGroup* propertyGroup = (*i)->getProperties();
		T_ASSERT (propertyGroup);

		if (propertyGroup->getProperty< int32_t >(c_keyUserHandle) != userHandle)
			continue;

		uint32_t addr = propertyGroup->getProperty< int32_t >(c_keyUserAddr);
		uint16_t port = propertyGroup->getProperty< int32_t >(c_keyUserPort);

		int32_t result = m_socket->sendTo(
			net::SocketAddressIPv4(addr, port),
			data,
			int(size)
		);
		return bool(result == int32_t(size));
	}

	return false;
#endif
}

void LanUser::update()
{
#if defined(T_INTERNET_SIMULATION)
	RefArray< net::NetworkService > userServices;
	m_discoveryManager->findServices< net::NetworkService >(userServices);

	double T = s_timer.getElapsedTime();
	for (std::list< Packet >::iterator i = m_packets.begin(); i != m_packets.end(); )
	{
		if (T >= i->sendAt)
		{
			for (RefArray< net::NetworkService >::const_iterator j = userServices.begin(); j != userServices.end(); ++j)
			{
				if ((*j)->getType() != c_keyServiceTypeUser)
					continue;

				const PropertyGroup* propertyGroup = (*j)->getProperties();
				T_ASSERT (propertyGroup);

				if (propertyGroup->getProperty< int32_t >(c_keyUserHandle) != i->userHandle)
					continue;

				uint32_t addr = propertyGroup->getProperty< int32_t >(c_keyUserAddr);
				uint16_t port = propertyGroup->getProperty< int32_t >(c_keyUserPort);

				int32_t result = m_socket->sendTo(
					net::SocketAddressIPv4(addr, port),
					i->data,
					int(i->size)
				);
			}

			delete[] i->data;
			i = m_packets.erase(i);
		}
		else
			++i;
	}
#endif
}

	}
}
