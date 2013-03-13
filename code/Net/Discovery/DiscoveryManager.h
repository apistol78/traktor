#ifndef traktor_net_DiscoveryManager_H
#define traktor_net_DiscoveryManager_H

#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class MulticastUdpSocket;
class UdpSocket;
class SocketAddressIPv4;
class IDiscoveryMessage;
class IService;

/*! \brief Network service publish and discovery manager.
 * \ingroup Net
 */
class T_DLLCLASS DiscoveryManager : public Object
{
	T_RTTI_CLASS;

public:
	DiscoveryManager();

	bool create(bool verbose);

	void destroy();

	void addService(IService* service);

	void removeService(IService* service);

	bool beginFindServices(const TypeInfo& serviceType);
	
	void endFindServices(RefArray< IService >& outServices);
	
	template < typename ServiceType >
	bool beginFindServices()
	{
		return beginFindServices(type_of< ServiceType >());
	}

	template < typename ServiceType >
	void endFindServices(RefArray< ServiceType >& outServices)
	{
		endFindServices((RefArray< IService >&)outServices);
	}

	template < typename ServiceType >
	bool findServices(RefArray< ServiceType >& outServices, int32_t timeout = 1000)
	{
		if (!beginFindServices< ServiceType >())
			return false;

		ThreadManager::getInstance().getCurrentThread()->sleep(timeout);

		endFindServices< ServiceType >(outServices);
		return true;
	}

private:
	Ref< UdpSocket > m_multicastSendSocket;
	Ref< MulticastUdpSocket > m_multicastRecvSocket;
	Thread* m_threadMulticastListener;
	Guid m_sessionGuid;
	bool m_verbose;
	bool m_accept;
	RefArray< IService > m_services;
	RefArray< IService > m_foundServices;

	void threadMulticastListener();

	bool sendMessage(UdpSocket* socket, const SocketAddressIPv4& address, const IDiscoveryMessage* message);

	Ref< IDiscoveryMessage > recvMessage(UdpSocket* socket, SocketAddressIPv4* fromAddress, int32_t timeout);
};

	}
}

#endif	// traktor_net_DiscoveryManager_H
