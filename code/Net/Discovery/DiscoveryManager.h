#ifndef traktor_net_DiscoveryManager_H
#define traktor_net_DiscoveryManager_H

#include "Core/Object.h"
#include "Core/Guid.h"
#include "Core/Heap/Ref.h"
#include "Core/Thread/Thread.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS(DiscoveryManager)

public:
	DiscoveryManager();

	bool create();

	void destroy();

	void addService(IService* service);

	void removeService(IService* service);

	bool findServices(const Type& serviceType, RefArray< IService >& outServices, uint32_t timeout);

private:
	Ref< UdpSocket > m_sendSocket;
	Ref< MulticastUdpSocket > m_multicastSocket;
	Thread* m_threadMulticastListener;
	Guid m_sessionGuid;
	RefArray< IService > m_services;

	void threadMulticastListener();

	bool sendMessage(UdpSocket* socket, const SocketAddressIPv4& address, const IDiscoveryMessage* message);

	IDiscoveryMessage* recvMessage(UdpSocket* socket, SocketAddressIPv4* fromAddress, int32_t timeout);
};

	}
}

#endif	// traktor_net_DiscoveryManager_H
