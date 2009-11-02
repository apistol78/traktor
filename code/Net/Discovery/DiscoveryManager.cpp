#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/DmFindServices.h"
#include "Net/Discovery/DmServiceInfo.h"
#include "Net/Discovery/IService.h"
#include "Net/UdpSocket.h"
#include "Net/MulticastUdpSocket.h"
#include "Net/SocketAddressIPv4.h"
#include "Core/Heap/GcNew.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace net
	{
		namespace
		{

const wchar_t* c_discoveryGroup = L"225.0.0.37";
const uint16_t c_discoveryPort = 12345;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.DiscoveryManager", DiscoveryManager, Object)

DiscoveryManager::DiscoveryManager()
:	m_threadMulticastListener(0)
,	m_verbose(false)
{
}

bool DiscoveryManager::create(bool verbose)
{
	m_sendSocket = gc_new< UdpSocket >();
	if (!m_sendSocket->bind(SocketAddressIPv4(c_discoveryPort)))
	{
		log::error << L"Discovery setup failed; unable to bind send socket port" << Endl;
		return false;
	}

	m_multicastSocket = gc_new< MulticastUdpSocket >();
	if (!m_multicastSocket->bind(SocketAddressIPv4(c_discoveryPort)))
	{
		log::error << L"Discovery setup failed; unable to bind multicast socket port" << Endl;
		return false;
	}
	if (!m_multicastSocket->joinGroup(SocketAddressIPv4(c_discoveryGroup, c_discoveryPort)))
	{
		log::error << L"Discovery setup failed; unable to join multicast group" << Endl;
		return false;
	}

	if (!m_multicastSocket->setTTL(1))
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
	if (m_multicastSocket)
	{
		m_multicastSocket->close();
		m_multicastSocket = 0;
	}
	if (m_sendSocket)
	{
		m_sendSocket->close();
		m_sendSocket = 0;
	}
}

void DiscoveryManager::addService(IService* service)
{
	m_services.push_back(service);
	if (m_verbose)
		log::info << L"Discovery manager: Service \"" << service->getDescription() << L" added" << Endl;
}

void DiscoveryManager::removeService(IService* service)
{
}

bool DiscoveryManager::findServices(const Type& serviceType, RefArray< IService >& outServices, uint32_t timeout)
{
	SocketAddressIPv4 address(c_discoveryGroup, c_discoveryPort);
	SocketAddressIPv4 fromAddress;

	// Request services from all listening discovery peers.
	DmFindServices msgFindServices(m_sessionGuid, &serviceType);
	if (!sendMessage(m_sendSocket, address, &msgFindServices))
	{
		if (m_verbose)
			log::info << L"Discovery manager: Unable to send \"find services\" message" << Endl;
		return false;
	}

	if (m_verbose)
		log::info << L"Discovery manager: \"Find services\" message sent, waiting for replies..." << Endl;

	// Accept services.
	for (;;)
	{
		Ref< IDiscoveryMessage > message = recvMessage(m_sendSocket, &fromAddress, timeout);
		if (!message)
		{
			if (m_verbose)
				log::info << L"Discovery manager: No message received" << Endl;
			break;
		}

		if (IService* service = dynamic_type_cast< IService* >(message))
		{
			if (m_verbose)
				log::info << L"Discovery manager: Got service \"" << service->getDescription() << L"\"" << Endl;
			outServices.push_back(service);
		}
		else
		{
			if (m_verbose)
				log::info << L"Discovery manager: Got unknown message" << Endl;
		}
	}

	return true;
}

void DiscoveryManager::threadMulticastListener()
{
	SocketAddressIPv4 fromAddress;
	while (!m_threadMulticastListener->stopped())
	{
		Ref< IDiscoveryMessage > message = recvMessage(m_multicastSocket, &fromAddress, 100);
		if (!message)
			continue;

		if (DmFindServices* findServices = dynamic_type_cast< DmFindServices* >(message))
		{
			if (findServices->getSessionGuid() != m_sessionGuid)
			{
				if (m_verbose)
					log::info << L"Discovery manager: Got \"find services\" request" << Endl;

				const Type* serviceType = findServices->getServiceType();
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

							if (!sendMessage(m_multicastSocket, fromAddress, *i))
								log::error << L"Unable to reply service to requesting manager" << Endl;
						}
					}
				}
				else
					log::error << L"Got find services message with invalid service type" << Endl;
			}
		}
		else
			log::error << L"Got unknown discovery message" << Endl;
	}
}

bool DiscoveryManager::sendMessage(UdpSocket* socket, const SocketAddressIPv4& address, const IDiscoveryMessage* message)
{
	DynamicMemoryStream dms(false, true);
	if (!BinarySerializer(&dms).writeObject(message))
		return false;

	const std::vector< uint8_t >& buffer = dms.getBuffer();
	return socket->sendTo(address, &buffer[0], buffer.size()) == buffer.size();
}

IDiscoveryMessage* DiscoveryManager::recvMessage(UdpSocket* socket, SocketAddressIPv4* fromAddress, int32_t timeout)
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
