#ifndef traktor_online_LanUser_H
#define traktor_online_LanUser_H

#include <map>
#include "Online/Provider/IUserProvider.h"

//#define T_INTERNET_SIMULATION

namespace traktor
{
	namespace net
	{

class DiscoveryManager;
class UdpSocket;

	}

	namespace online
	{

class LanUser : public IUserProvider
{
	T_RTTI_CLASS;

public:
	LanUser(net::DiscoveryManager* discoveryManager, net::UdpSocket* socket);

	virtual bool getName(uint64_t userHandle, std::wstring& outName);

	virtual Ref< drawing::Image > getImage(uint64_t userHandle) const;

	virtual bool isFriend(uint64_t userHandle);

	virtual bool isMemberOfGroup(uint64_t userHandle, const std::wstring& groupName) const;

	virtual bool joinGroup(uint64_t userHandle, const std::wstring& groupName);

	virtual bool invite(uint64_t userHandle);

	virtual bool setPresenceValue(uint64_t userHandle, const std::wstring& key, const std::wstring& value);

	virtual bool getPresenceValue(uint64_t userHandle, const std::wstring& key, std::wstring& outValue);

	virtual void setP2PEnable(uint64_t userHandle, bool enable);

	virtual bool isP2PAllowed(uint64_t userHandle) const;

	virtual bool isP2PRelayed(uint64_t userHandle) const;

	virtual bool sendP2PData(uint64_t userHandle, const void* data, size_t size, bool reliable);

	void update();

private:
	Ref< net::DiscoveryManager > m_discoveryManager;
	Ref< net::UdpSocket > m_socket;

#if defined(T_INTERNET_SIMULATION)
	struct Packet
	{
		double sendAt;
		uint64_t userHandle;
		uint8_t* data;
		size_t size;
	};

	std::list< Packet > m_packets;
#endif
};

	}
}

#endif	// traktor_online_LanUser_H
