/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/DynamicMemoryStream.h"
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

namespace traktor
{
	namespace net
	{
		namespace
		{

const wchar_t* c_discoveryMulticastGroup = L"225.0.0.37";
const uint16_t c_discoveryMulticastPort = 41100;

OutputStream& operator << (OutputStream& os, const net::SocketAddressIPv4& addr)
{
	return os << addr.getHostName() << L":" << addr.getPort();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.DiscoveryManager", DiscoveryManager, Object)

DiscoveryManager::DiscoveryManager()
:	m_threadMulticastListener(0)
,	m_mode(0)
{
}

bool DiscoveryManager::create(uint32_t mode)
{
	// Create multicast sockets.
	m_multicastSendSocket = new UdpSocket();
	if (!m_multicastSendSocket->bind(SocketAddressIPv4(c_discoveryMulticastPort)))
	{
		log::error << L"Discovery setup failed; unable to bind send socket port" << Endl;
		return false;
	}

	m_multicastRecvSocket = new MulticastUdpSocket();
	if (!m_multicastRecvSocket->bind(SocketAddressIPv4(c_discoveryMulticastPort)))
	{
		log::error << L"Discovery setup failed; unable to bind multicast socket port" << Endl;
		return false;
	}

	if (!m_multicastRecvSocket->joinGroup(SocketAddressIPv4(c_discoveryMulticastGroup, c_discoveryMulticastPort)))
	{
		log::error << L"Discovery setup failed; unable to join multicast group" << Endl;
		return false;
	}

	if (!m_multicastRecvSocket->setTTL(32))
		log::warning << L"Unable to set multicast time-to-live option" << Endl;

	// Create direct communication socket.
	net::SocketAddressIPv4::Interface itf;
	if (!net::SocketAddressIPv4::getBestInterface(itf))
	{
		log::error << L"Discovery setup failed; unable to enumerate interfaces" << Endl;
		return false;
	}

	m_directSocket = new UdpSocket();
	if (!m_directSocket->bind(SocketAddressIPv4(itf.addr->getAddr(), 0)))
	{
		log::error << L"Discovery setup failed; unable to bind receive socket" << Endl;
		return false;
	}

	m_replyToAddress = *dynamic_type_cast< net::SocketAddressIPv4* >(m_directSocket->getLocalAddress());
	if ((mode & MdVerbose) != 0)
		log::info << L"Discovery manager: receive address " << m_replyToAddress << Endl;

	// Create communication thread.
	m_threadMulticastListener = ThreadManager::getInstance().create(makeFunctor(this, &DiscoveryManager::threadMulticastListener), L"Discovery listener");
	if (!m_threadMulticastListener)
	{
		log::error << L"Discovery setup failed; unable to create listener thread" << Endl;
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
		m_threadMulticastListener = 0;
	}

	safeClose(m_directSocket);
	safeClose(m_multicastRecvSocket);
	safeClose(m_multicastSendSocket);
}

void DiscoveryManager::addService(IService* service)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_localServicesLock);
	LocalService ls;
	ls.serviceGuid = Guid::create();
	ls.service = service;
	m_localServices.push_back(ls);
}

void DiscoveryManager::replaceService(IService* oldService, IService* newService)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_localServicesLock);
	for (std::list< LocalService >::iterator i = m_localServices.begin(); i != m_localServices.end(); ++i)
	{
		if (i->service == oldService)
		{
			i->service = newService;
			break;
		}
	}
}

void DiscoveryManager::removeService(IService* service)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_localServicesLock);
	for (std::list< LocalService >::iterator i = m_localServices.begin(); i != m_localServices.end(); ++i)
	{
		if (i->service == service)
		{
			m_localServices.erase(i);
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
	for (std::map< Guid, ExternalService >::const_iterator i = m_externalServices.begin(); i != m_externalServices.end(); ++i)
	{
		if (is_type_of(serviceType, type_of(i->second.service)))
			outServices.push_back(i->second.service);
	}
	return true;
}

void DiscoveryManager::threadMulticastListener()
{
	SocketAddressIPv4 address(c_discoveryMulticastGroup, c_discoveryMulticastPort);
	SocketAddressIPv4 fromAddress;
	Timer timer;
	int32_t res;

	timer.start();
	double beacon = timer.getElapsedTime() + 1.0;

	while (!m_threadMulticastListener->stopped())
	{
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
					log::info << L"Discovery manager: \"find services\" broadcasted successfully" << Endl;
			}
			else
			{
				if ((m_mode & MdVerbose) != 0)
					log::info << L"Discovery manager: Unable to send \"find services\" message (" << res << L")" << Endl;
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
				log::info << L"Discovery manager: received multicast message" << Endl;

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
					log::info << L"Discovery manager: Got \"find services\" request from " << fromAddress << L", reply to " << findServices->getReplyTo() << Endl;

				{
					T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_localServicesLock);
					for (std::list< LocalService >::const_iterator i = m_localServices.begin(); i != m_localServices.end(); ++i)
					{
						DmServiceInfo serviceInfo(i->serviceGuid, i->service);
						if ((res = sendMessage(
							m_directSocket,
							findServices->getReplyTo(),
							&serviceInfo
						)) != 0)
							log::error << L"Unable to reply service to requesting manager (" << res << L")" << Endl;
						else if ((m_mode & MdVerbose) != 0)
							log::info << L"Discovery manager: Reply sent to " << address << Endl;
					}
				}
			}
		}

		// Did we receive a direct message?
		if (resultSet.contain(m_directSocket))
		{
			if ((m_mode & MdVerbose) != 0)
				log::info << L"Discovery manager: received direct message" << Endl;

			Ref< IDiscoveryMessage > message = recvMessage(m_directSocket, &fromAddress);
			if (DmServiceInfo* serviceInfo = dynamic_type_cast< DmServiceInfo* >(message))
			{
				if (serviceInfo->getService())
				{
					T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_externalServicesLock);

					if ((m_mode & MdVerbose) != 0)
						log::info << L"Discovery manager: Got \"service info\" from " << fromAddress << Endl;

					m_externalServices[serviceInfo->getServiceGuid()].tick = 0;
					m_externalServices[serviceInfo->getServiceGuid()].service = serviceInfo->getService();
				}
				else
				{
					if ((m_mode & MdVerbose) != 0)
						log::warning << L"Discovery manager: Got invalid \"service info\" from " << fromAddress << Endl;
				}
			}
		}
	}
}

int32_t DiscoveryManager::sendMessage(UdpSocket* socket, const SocketAddressIPv4& address, const IDiscoveryMessage* message)
{
	Ref< DynamicMemoryStream > dms = new DynamicMemoryStream(false, true, T_FILE_LINE);
	if (!BinarySerializer(dms).writeObject(message))
		return 1;

	const AlignedVector< uint8_t >& buffer = dms->getBuffer();
	if (buffer.size() >= 1024)
        return 2;

	if (socket->sendTo(address, &buffer[0], uint32_t(buffer.size())) != buffer.size())
		return 3;

	return 0;
}

Ref< IDiscoveryMessage > DiscoveryManager::recvMessage(UdpSocket* socket, SocketAddressIPv4* fromAddress)
{
	uint8_t buffer[1024];

	int32_t nrecv = socket->recvFrom(buffer, sizeof(buffer), fromAddress);
	if (nrecv <= 0)
		return 0;

	MemoryStream ms(buffer, nrecv, true, false);
	return BinarySerializer(&ms).readObject< IDiscoveryMessage >();
}

	}
}
