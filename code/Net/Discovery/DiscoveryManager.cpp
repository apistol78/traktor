#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/DmFindServices.h"
#include "Net/Discovery/DmServiceInfo.h"
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
{
}

bool DiscoveryManager::create()
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

	m_threadMulticastListener = ThreadManager::getInstance().create(makeFunctor(this, &DiscoveryManager::threadMulticastListener), L"Discovery listener");
	if (!m_threadMulticastListener)
	{
		log::error << L"Discovery setup failed; unable to create listener thread" << Endl;
		return false;
	}

	m_sessionGuid = Guid::create();
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
		return false;

	// Accept services.
	for (;;)
	{
		Ref< IDiscoveryMessage > message = recvMessage(m_sendSocket, &fromAddress, timeout);
		if (!message)
			break;

		if (DmServiceInfo* serviceInfo = dynamic_type_cast< DmServiceInfo* >(message))
		{
			log::info << L"Got service info" << Endl;
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
				for (RefArray< IService >::const_iterator i = m_services.begin(); i != m_services.end(); ++i)
				{
					//DmServiceInfo serviceInfo(L"My service");
					//if (!sendMessage(m_multicastSocket, fromAddress, &serviceInfo))
					//	log::error << L"Unable to reply service to requesting manager" << Endl;
				}
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
