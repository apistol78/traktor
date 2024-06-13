/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Thread/Acquire.h"
#include "Core/Timer/Timer.h"
#include "Net/MulticastUdpSocket.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/SocketSet.h"
#include "Net/UdpSocket.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/DmFindServices.h"
#include "Net/Discovery/DmServiceInfo.h"
#include "Net/Discovery/IService.h"

namespace traktor::net
{
	namespace
	{

const wchar_t* c_discoveryMulticastGroup = L"225.0.0.37";
const uint16_t c_discoveryMulticastPort = 41100;
const int32_t c_maxUnresponsiveTickCount = 30;

OutputStream& operator << (OutputStream& os, const net::SocketAddressIPv4& addr)
{
	return os << addr.getHostName() << L":" << addr.getPort();
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.DiscoveryManager", DiscoveryManager, Object)

bool DiscoveryManager::create(uint32_t mode)
{
	// Create multicast sockets.
	m_multicastSendSocket = new UdpSocket();
	if (!m_multicastSendSocket->bind(SocketAddressIPv4(c_discoveryMulticastPort)))
	{
		log::error << L"Discovery setup failed; unable to bind send socket port." << Endl;
		return false;
	}

	m_multicastRecvSocket = new MulticastUdpSocket();
	if (!m_multicastRecvSocket->bind(SocketAddressIPv4(c_discoveryMulticastPort)))
	{
		log::error << L"Discovery setup failed; unable to bind multicast socket port." << Endl;
		return false;
	}

	if (!m_multicastRecvSocket->joinGroup(SocketAddressIPv4(c_discoveryMulticastGroup, c_discoveryMulticastPort)))
	{
		log::error << L"Discovery setup failed; unable to join multicast group." << Endl;
		return false;
	}

	if (!m_multicastRecvSocket->setTTL(32))
		log::warning << L"Unable to set multicast time-to-live option." << Endl;

	// Create direct communication socket.
	net::SocketAddressIPv4::Interface itf;
	if (!net::SocketAddressIPv4::getBestInterface(itf))
	{
		log::error << L"Discovery setup failed; unable to enumerate interfaces." << Endl;
		return false;
	}

	m_directSocket = new UdpSocket();
	if (!m_directSocket->bind(SocketAddressIPv4(itf.addr->getAddr(), 0)))
	{
		log::error << L"Discovery setup failed; unable to bind receive socket." << Endl;
		return false;
	}

	m_replyToAddress = *dynamic_type_cast< net::SocketAddressIPv4* >(m_directSocket->getLocalAddress());
	if ((mode & MdVerbose) != 0)
		log::info << L"Discovery manager: receive address " << m_replyToAddress << L"." << Endl;

	// Create communication thread.
	m_threadMulticastListener = ThreadManager::getInstance().create([=, this](){ threadMulticastListener(); }, L"Discovery listener");
	if (!m_threadMulticastListener)
	{
		log::error << L"Discovery setup failed; unable to create listener thread." << Endl;
		return false;
	}

	m_managerGuid = Guid::create();
	m_mode = mode;

	m_threadMulticastListener->start();
	return true;
}

void DiscoveryManager::destroy()
{
	if (m_threadMulticastListener)
	{
		m_threadMulticastListener->stop();
		ThreadManager::getInstance().destroy(m_threadMulticastListener);
		m_threadMulticastListener = nullptr;
	}

	safeClose(m_directSocket);
	safeClose(m_multicastRecvSocket);
	safeClose(m_multicastSendSocket);
}

void DiscoveryManager::addService(IService* service)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_localServicesLock);
	LocalService& ls = m_localServices.push_back();
	ls.serviceGuid = Guid::create();
	ls.service = service;
}

void DiscoveryManager::replaceService(IService* oldService, IService* newService)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_localServicesLock);
	for (auto& localService : m_localServices)
	{
		if (localService.service == oldService)
		{
			localService.service = newService;
			break;
		}
	}
}

void DiscoveryManager::removeService(IService* service)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_localServicesLock);
	for (auto it = m_localServices.begin(); it != m_localServices.end(); ++it)
	{
		if (it->service == service)
		{
			m_localServices.erase(it);
			break;
		}
	}
}

void DiscoveryManager::removeAllServices()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_localServicesLock);
	m_localServices.clear();
}

bool DiscoveryManager::findServices(const TypeInfo& serviceType, RefArray< IService >& outServices)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_externalServicesLock);
	outServices.clear();
	for (const auto& externalService : m_externalServices)
	{
		if (
			externalService.second.tick <= c_maxUnresponsiveTickCount &&
			is_type_of(serviceType, type_of(externalService.second.service))
		)
			outServices.push_back(externalService.second.service);
	}
	return true;
}

void DiscoveryManager::threadMulticastListener()
{
	SocketAddressIPv4 address(c_discoveryMulticastGroup, c_discoveryMulticastPort);
	SocketAddressIPv4 fromAddress;
	Timer timer;
	int32_t res;

	timer.reset();
	double beacon = timer.getElapsedTime() + 1.0;

	while (!m_threadMulticastListener->stopped())
	{
		// Count number of "ticks" since last response.
		for (auto& externalService : m_externalServices)
			externalService.second.tick++;

		// Broadcast "find service" message to all multicast listeners.
		if (
			((m_mode & MdFindServices) != 0) &&
			timer.getElapsedTime() > beacon
		)
		{
			DmFindServices msgFindServices(m_managerGuid, m_replyToAddress);
			if ((res = sendMessage(m_multicastSendSocket, address, &msgFindServices)) == 0)
			{
				if ((m_mode & MdVerbose) != 0)
					log::info << L"Discovery manager: \"find services\" broadcasted successfully." << Endl;
			}
			else
			{
				if ((m_mode & MdVerbose) != 0)
					log::info << L"Discovery manager: Unable to send \"find services\" message (" << res << L")." << Endl;
			}
			beacon = timer.getElapsedTime() + 1.0;
		}

		// Wait for any received data on any socket.
		SocketSet waitSet;
		SocketSet resultSet;
		waitSet.add(m_multicastRecvSocket);
		waitSet.add(m_directSocket);
		if (waitSet.select(true, false, false, 500, resultSet) <= 0)
			continue;

		// Did we receive any multicast message?
		if (resultSet.contain(m_multicastRecvSocket))
		{
			if ((m_mode & MdVerbose) != 0)
				log::info << L"Discovery manager: received multicast message." << Endl;

			Ref< IDiscoveryMessage > message = recvMessage(m_multicastRecvSocket, &fromAddress);
			if (DmFindServices* findServices = dynamic_type_cast< DmFindServices* >(message))
			{
				if ((m_mode & MdPublishServices) == 0)
					continue;

				// Do not respond to ourself.
				Guid requestingManagerGuid = findServices->getManagerGuid();
				if (requestingManagerGuid == m_managerGuid)
					continue;

				if ((m_mode & MdVerbose) != 0)
					log::info << L"Discovery manager: Got \"find services\" request from " << fromAddress << L", reply to " << findServices->getReplyTo() << L"." << Endl;

				{
					T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_localServicesLock);
					for (const auto& localService : m_localServices)
					{
						DmServiceInfo serviceInfo(localService.serviceGuid, localService.service);
						if ((res = sendMessage(
							m_directSocket,
							findServices->getReplyTo(),
							&serviceInfo
						)) != 0)
							log::error << L"Unable to reply service to requesting manager (" << res << L")." << Endl;
						else if ((m_mode & MdVerbose) != 0)
							log::info << L"Discovery manager: Reply sent to " << address << L"." << Endl;
					}
				}
			}
		}

		// Did we receive a direct message?
		if (resultSet.contain(m_directSocket))
		{
			if ((m_mode & MdVerbose) != 0)
				log::info << L"Discovery manager: received direct message." << Endl;

			Ref< IDiscoveryMessage > message = recvMessage(m_directSocket, &fromAddress);
			if (DmServiceInfo* serviceInfo = dynamic_type_cast< DmServiceInfo* >(message))
			{
				if (serviceInfo->getService())
				{
					T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_externalServicesLock);

					if ((m_mode & MdVerbose) != 0)
						log::info << L"Discovery manager: Got \"service info\" from " << fromAddress << L"." << Endl;

					m_externalServices[serviceInfo->getServiceGuid()].tick = 0;
					m_externalServices[serviceInfo->getServiceGuid()].service = serviceInfo->getService();
				}
				else
				{
					if ((m_mode & MdVerbose) != 0)
						log::warning << L"Discovery manager: Got invalid \"service info\" from " << fromAddress << L"." << Endl;
				}
			}
		}
	}
}

int32_t DiscoveryManager::sendMessage(UdpSocket* socket, const SocketAddressIPv4& address, const IDiscoveryMessage* message)
{
	uint8_t buffer[1024];

	MemoryStream ms(buffer, sizeof(buffer), false, true);
	if (!BinarySerializer(&ms).writeObject(message))
		return 1;

	const uint32_t written = ms.tell();
	if (written >= 1024)
        return 2;

	if (socket->sendTo(address, buffer, written) != written)
		return 3;

	return 0;
}

Ref< IDiscoveryMessage > DiscoveryManager::recvMessage(UdpSocket* socket, SocketAddressIPv4* fromAddress)
{
	uint8_t buffer[1024];

	const int32_t nrecv = socket->recvFrom(buffer, sizeof(buffer), fromAddress);
	if (nrecv <= 0)
		return nullptr;

	MemoryStream ms(buffer, nrecv, true, false);
	return BinarySerializer(&ms).readObject< IDiscoveryMessage >();
}

}
