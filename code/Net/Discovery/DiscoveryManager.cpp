#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Thread/Acquire.h"
#include "Core/Timer/Timer.h"
#include "Net/MulticastUdpSocket.h"
#include "Net/SocketAddressIPv4.h"
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
const uint16_t c_discoveryMulticastPort = 40000;

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

	if (!m_multicastRecvSocket->setTTL(10))
		log::warning << L"Unable to set multicast time-to-live option" << Endl;

	m_threadMulticastListener = ThreadManager::getInstance().create(makeFunctor(this, &DiscoveryManager::threadMulticastListener), L"Discovery listener");
	if (!m_threadMulticastListener)
	{
		log::error << L"Discovery setup failed; unable to create listener thread" << Endl;
		return false;
	}

	m_managerGuid = Guid::create();
	m_mode = mode;
	
	m_threadMulticastListener->start(Thread::Below);
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

	if (m_multicastRecvSocket)
	{
		m_multicastRecvSocket->close();
		m_multicastRecvSocket = 0;
	}

	if (m_multicastSendSocket)
	{
		m_multicastSendSocket->close();
		m_multicastSendSocket = 0;
	}
}

void DiscoveryManager::addService(IService* service)
{
	LocalService ls;
	ls.serviceGuid = Guid::create();
	ls.service = service;
	m_localServices.push_back(ls);
}

void DiscoveryManager::removeService(IService* service)
{
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

	timer.start();
	double beacon = timer.getElapsedTime() + 1.0;

	while (!m_threadMulticastListener->stopped())
	{
		if (
			((m_mode & MdFindServices) != 0) &&
			timer.getElapsedTime() > beacon
		)
		{
			DmFindServices msgFindServices(m_managerGuid);
			if (!sendMessage(m_multicastSendSocket, address, &msgFindServices))
			{
				if ((m_mode & MdVerbose) != 0)
					log::info << L"Discovery manager: Unable to send \"find services\" message" << Endl;
			}
			beacon = timer.getElapsedTime() + 1.0;
		}

		Ref< IDiscoveryMessage > message = recvMessage(m_multicastRecvSocket, &fromAddress, 100);
		if (!message)
			continue;

		if (DmFindServices* findServices = dynamic_type_cast< DmFindServices* >(message))
		{
			if ((m_mode & MdPublishServices) == 0)
				continue;

			// Do not respond to our self.
			Guid requestingManagerGuid = findServices->getManagerGuid();
			if (requestingManagerGuid == m_managerGuid)
				continue;

			if ((m_mode & MdVerbose) != 0)
				log::info << L"Discovery manager: Got \"find services\" request from " << fromAddress << Endl;

			for (std::list< LocalService >::const_iterator i = m_localServices.begin(); i != m_localServices.end(); ++i)
			{
				DmServiceInfo serviceInfo(requestingManagerGuid, i->serviceGuid, i->service);
				if (!sendMessage(m_multicastSendSocket, address, &serviceInfo))
					log::error << L"Unable to reply service to requesting manager" << Endl;
				else if ((m_mode & MdVerbose) != 0)
					log::info << L"Discovery manager: Reply sent to " << address << Endl;
			}
		}
		else if (DmServiceInfo* serviceInfo = dynamic_type_cast< DmServiceInfo* >(message))
		{
			// Response to an query issued from me?
			if (serviceInfo->getManagerGuid() != m_managerGuid)
				continue;

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
		else
			log::error << L"Got unknown discovery message" << Endl;
	}
}

bool DiscoveryManager::sendMessage(UdpSocket* socket, const SocketAddressIPv4& address, const IDiscoveryMessage* message)
{
	Ref< DynamicMemoryStream > dms = new DynamicMemoryStream(false, true, T_FILE_LINE);
	if (!BinarySerializer(dms).writeObject(message))
		return false;

	const std::vector< uint8_t >& buffer = dms->getBuffer();
	if (buffer.size() >= 1024)
        return false;

	return socket->sendTo(address, &buffer[0], uint32_t(buffer.size())) == buffer.size();
}

Ref< IDiscoveryMessage > DiscoveryManager::recvMessage(UdpSocket* socket, SocketAddressIPv4* fromAddress, int32_t timeout)
{
	uint8_t buffer[1024];

	if (socket->select(true, false, false, timeout) <= 0)
		return 0;

	int32_t nrecv = socket->recvFrom(buffer, sizeof(buffer), fromAddress);
	if (nrecv <= 0)
		return 0;

	MemoryStream ms(buffer, nrecv, true, false);
	return BinarySerializer(&ms).readObject< IDiscoveryMessage >();
}

	}
}
