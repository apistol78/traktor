#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
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
const uint16_t c_discoveryMulticastPort = 12345;

OutputStream& operator << (OutputStream& os, const net::SocketAddressIPv4& addr)
{
	return os << addr.getHostName() << L":" << addr.getPort();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.DiscoveryManager", DiscoveryManager, Object)

DiscoveryManager::DiscoveryManager()
:	m_threadMulticastListener(0)
,	m_verbose(false)
,	m_accept(false)
{
}

bool DiscoveryManager::create(bool verbose)
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

	m_sessionGuid = Guid::create();
	m_verbose = verbose;
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
	m_services.push_back(service);
}

void DiscoveryManager::removeService(IService* service)
{
}

bool DiscoveryManager::beginFindServices(const TypeInfo& serviceType)
{
	SocketAddressIPv4 address(c_discoveryMulticastGroup, c_discoveryMulticastPort);
	SocketAddressIPv4 fromAddress;
	m_accept = true;

	// Multi-cast services request.
	DmFindServices msgFindServices(m_sessionGuid, &serviceType);
	if (!sendMessage(m_multicastSendSocket, address, &msgFindServices))
	{
		m_accept = false;
		if (m_verbose)
			log::info << L"Discovery manager: Unable to send \"find services\" message" << Endl;
		return false;
	}

	if (m_verbose)
		log::info << L"Discovery manager: \"Find services\" message sent to " << address << L", waiting for replies..." << Endl;

	return true;
}

void DiscoveryManager::endFindServices(RefArray< IService >& outServices)
{
	outServices = m_foundServices;
	m_accept = false;
	m_foundServices.resize(0);
}

void DiscoveryManager::threadMulticastListener()
{
	SocketAddressIPv4 address(c_discoveryMulticastGroup, c_discoveryMulticastPort);
	SocketAddressIPv4 fromAddress;

	while (!m_threadMulticastListener->stopped())
	{
		Ref< IDiscoveryMessage > message = recvMessage(m_multicastRecvSocket, &fromAddress, 100);
		if (!message)
			continue;

		if (DmFindServices* findServices = dynamic_type_cast< DmFindServices* >(message))
		{
			Guid requestingSessionGuid = findServices->getSessionGuid();
			if (requestingSessionGuid != m_sessionGuid)
			{
				if (m_verbose)
					log::info << L"Discovery manager: Got \"find services\" request from " << fromAddress << Endl;

				const TypeInfo* serviceType = findServices->getServiceType();
				if (serviceType)
				{
					if (m_verbose)
						log::info << L"Discovery manager: Find services of \"" << serviceType->getName() << L"\" type" << Endl;

					for (RefArray< IService >::const_iterator i = m_services.begin(); i != m_services.end(); ++i)
					{
						if (is_type_of(*serviceType, type_of(*i)))
						{
							if (m_verbose)
								log::info << L"Discovery manager: Found registered local service of requested type" << Endl;

							DmServiceInfo serviceInfo(requestingSessionGuid, *i);
							if (!sendMessage(m_multicastSendSocket, address, &serviceInfo))
								log::error << L"Unable to reply service to requesting manager" << Endl;
							else if (m_verbose)
								log::info << L"Discovery manager: Reply sent to " << address << Endl;
						}
					}
				}
				else
					log::error << L"Got find services message with invalid service type" << Endl;
			}
		}
		else if (DmServiceInfo* serviceInfo = dynamic_type_cast< DmServiceInfo* >(message))
		{
			if (serviceInfo->getSessionGuid() == m_sessionGuid)
			{
				if (serviceInfo->getService())
				{
					if (m_verbose)
						log::info << L"Discovery manager: Got \"service info\" from " << fromAddress << Endl;

					if (m_accept)
						m_foundServices.push_back(serviceInfo->getService());
				}
				else
				{
					if (m_verbose)
						log::warning << L"Discovery manager: Got invalid \"service info\" from " << fromAddress << Endl;
				}
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
	T_ASSERT (buffer.size() <= 1024);

	return socket->sendTo(address, &buffer[0], buffer.size()) == buffer.size();
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
