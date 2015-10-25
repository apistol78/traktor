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

	virtual bool getName(uint64_t userHandle, std::wstring& outName) T_OVERRIDE T_FINAL;

	virtual Ref< drawing::Image > getImage(uint64_t userHandle) const T_OVERRIDE T_FINAL;

	virtual bool isFriend(uint64_t userHandle) T_OVERRIDE T_FINAL;

	virtual bool isMemberOfGroup(uint64_t userHandle, const std::wstring& groupName) const T_OVERRIDE T_FINAL;

	virtual bool joinGroup(uint64_t userHandle, const std::wstring& groupName) T_OVERRIDE T_FINAL;

	virtual bool invite(uint64_t userHandle) T_OVERRIDE T_FINAL;

	virtual bool setPresenceValue(uint64_t userHandle, const std::wstring& key, const std::wstring& value) T_OVERRIDE T_FINAL;

	virtual bool getPresenceValue(uint64_t userHandle, const std::wstring& key, std::wstring& outValue) T_OVERRIDE T_FINAL;

	virtual void setP2PEnable(uint64_t userHandle, bool enable) T_OVERRIDE T_FINAL;

	virtual bool isP2PAllowed(uint64_t userHandle) const T_OVERRIDE T_FINAL;

	virtual bool isP2PRelayed(uint64_t userHandle) const T_OVERRIDE T_FINAL;

	virtual bool sendP2PData(uint64_t userHandle, const void* data, size_t size, bool reliable) T_OVERRIDE T_FINAL;

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
