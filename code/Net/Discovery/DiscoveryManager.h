#ifndef traktor_net_DiscoveryManager_H
#define traktor_net_DiscoveryManager_H

#include <list>
#include <map>
#include "Core/Guid.h"
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
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

	bool findServices(const TypeInfo& serviceType, RefArray< IService >& outServices);
	
	template < typename ServiceType >
	bool findServices(RefArray< ServiceType >& outServices)
	{
		return findServices(type_of< ServiceType >(), (RefArray< IService >&)outServices);
	}

private:
	struct LocalService
	{
		Guid serviceGuid;
		Ref< IService > service;
	};

	struct ExternalService
	{
		int32_t tick;
		Ref< IService > service;
	};

	Ref< UdpSocket > m_multicastSendSocket;
	Ref< MulticastUdpSocket > m_multicastRecvSocket;
	Thread* m_threadMulticastListener;
	
	Guid m_managerGuid;
	bool m_verbose;
	
	std::list< LocalService > m_localServices;
	std::map< Guid, ExternalService > m_externalServices;
	Semaphore m_externalServicesLock;

	void threadMulticastListener();

	bool sendMessage(UdpSocket* socket, const SocketAddressIPv4& address, const IDiscoveryMessage* message);

	Ref< IDiscoveryMessage > recvMessage(UdpSocket* socket, SocketAddressIPv4* fromAddress, int32_t timeout);
};

	}
}

#endif	// traktor_net_DiscoveryManager_H
