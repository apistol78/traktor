/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
#include "Net/SocketAddressIPv4.h"

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

/*! \brief
 * \ingroup Net
 */
enum DiscoveryManagerMode
{
	MdFindServices = 1,
	MdPublishServices = 2,
	MdVerbose = 4
};

/*! \brief Network service publish and discovery manager.
 * \ingroup Net
 */
class T_DLLCLASS DiscoveryManager : public Object
{
	T_RTTI_CLASS;

public:
	DiscoveryManager();

	bool create(uint32_t mode);

	void destroy();

	void addService(IService* service);

	void replaceService(IService* oldService, IService* newService);

	void removeService(IService* service);

	void removeAllServices();

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
	Ref< UdpSocket > m_directSocket;

	SocketAddressIPv4 m_replyToAddress;

	Thread* m_threadMulticastListener;
	Guid m_managerGuid;
	uint32_t m_mode;
	std::list< LocalService > m_localServices;
	Semaphore m_localServicesLock;
	std::map< Guid, ExternalService > m_externalServices;
	Semaphore m_externalServicesLock;

	void threadMulticastListener();

	int32_t sendMessage(UdpSocket* socket, const SocketAddressIPv4& address, const IDiscoveryMessage* message);

	Ref< IDiscoveryMessage > recvMessage(UdpSocket* socket, SocketAddressIPv4* fromAddress);
};

	}
}

#endif	// traktor_net_DiscoveryManager_H
