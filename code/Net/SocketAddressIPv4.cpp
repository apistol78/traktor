#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/TString.h"
#include "Net/SocketAddressIPv4.h"

#if defined(_WIN32) && !defined(WINCE)
#	include <ws2ipdef.h>
#	include <iphlpapi.h>
#endif

namespace traktor
{
	namespace net
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.net.SocketAddressIPv4", SocketAddressIPv4, SocketAddress)

SocketAddressIPv4::SocketAddressIPv4()
{
	std::memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = 0;
	m_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
}

SocketAddressIPv4::SocketAddressIPv4(const sockaddr_in& sockaddr)
:	m_sockaddr(sockaddr)
{
}

SocketAddressIPv4::SocketAddressIPv4(uint16_t port)
{
	std::memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(port);
	m_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
}

SocketAddressIPv4::SocketAddressIPv4(const uint32_t addr, uint16_t port)
{
	std::memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(port);
	m_sockaddr.sin_addr.s_addr = htonl(addr);
}

SocketAddressIPv4::SocketAddressIPv4(const uint8_t addr[4], uint16_t port)
{
	std::memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(port);
	m_sockaddr.sin_addr.s_addr = htonl(*reinterpret_cast< const uint32_t* >(addr));
}

SocketAddressIPv4::SocketAddressIPv4(const std::wstring& host, uint16_t port)
{
	// Try to resolve address, first try string denoted IP number as it will
	// probably fail faster than gethostbyname.
	uint32_t ia = inet_addr(wstombs(host).c_str());

#if !defined(_XBOX)
	if (ia == INADDR_NONE)
	{
		hostent* hostent = gethostbyname(wstombs(host).c_str());
		if (hostent != 0)
			ia = *reinterpret_cast< uint32_t* >(hostent->h_addr_list[0]);
	}
#endif

	std::memset(&m_sockaddr, 0, sizeof(m_sockaddr));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(port);
	m_sockaddr.sin_addr.s_addr = ia;
}

bool SocketAddressIPv4::valid() const
{
	return bool(m_sockaddr.sin_port != 0);
}

std::wstring SocketAddressIPv4::getHostName() const
{
#if !defined(_XBOX)
	return mbstows(inet_ntoa(*const_cast< in_addr* >(&m_sockaddr.sin_addr)));
#else
	return L"<unsupported>";
#endif
}

uint32_t SocketAddressIPv4::getAddr() const
{
	return ntohl(m_sockaddr.sin_addr.s_addr);
}

uint16_t SocketAddressIPv4::getPort() const
{
	return ntohs(m_sockaddr.sin_port);
}

void SocketAddressIPv4::setSockAddr(const sockaddr_in& sockaddr)
{
	std::memcpy(&m_sockaddr, &sockaddr, sizeof(sockaddr_in));
}

const sockaddr_in& SocketAddressIPv4::getSockAddr() const
{
	return m_sockaddr;
}

bool SocketAddressIPv4::getInterfaces(std::list< Interface >& outInterfaces)
{
#if defined(_WIN32) && !defined(WINCE)

	ULONG bufLen = 0;
	GetAdaptersInfo(0, &bufLen);

	AutoPtr< _IP_ADAPTER_INFO, AllocFreeAlign > info ((PIP_ADAPTER_INFO)Alloc::acquireAlign(bufLen, 16, T_FILE_LINE));
	if (!info.ptr())
		return false;

	if (GetAdaptersInfo(info.ptr(), &bufLen) != NO_ERROR)
		return false;

	for (PIP_ADAPTER_INFO ii = info.ptr(); ii; ii = ii->Next)
	{
		Interface itf;

		if (ii->Type == MIB_IF_TYPE_PPP)
			itf.type = ItVPN;
		else if (ii->Type == IF_TYPE_IEEE80211)
			itf.type = ItWiFi;
		else
			itf.type = ItDefault;

		sockaddr_in addr;
		addr.sin_port = 0;
		addr.sin_addr.s_addr = inet_addr(ii->IpAddressList.IpAddress.String);

		itf.addr = new SocketAddressIPv4(addr);

		outInterfaces.push_back(itf);
	}

#else

#	if defined(_WIN32) || TARGET_OS_MAC
	char hostName[200];
	if (gethostname(hostName, sizeof(hostName)) == 0)
	{
#		if defined(_WIN32)
		LPHOSTENT host;
#		else
		hostent* host;
#	endif
		host = gethostbyname(hostName);
		if (host && host->h_addr)
		{
			in_addr* ptr = (in_addr*)host->h_addr;

			sockaddr_in addr;
			addr.sin_port = 0;
			addr.sin_addr = *ptr;

			Interface itf;
			itf.type = ItDefault;
			itf.addr = new SocketAddressIPv4(addr);

			outInterfaces.push_back(itf);
		}
		else
		{
			log::error << L"Unable to get network interface(s); gethostbyname failed." << Endl;
			return false;
		}
	}
	else
	{
		log::error << L"Unable to get network interface(s); gethostname failed." << Endl;
		return false;
	}
#	endif

#endif

	return true;
}

bool SocketAddressIPv4::getBestInterface(Interface& outInterface)
{
	std::list< Interface > interfaces;
	if (!getInterfaces(interfaces) || interfaces.empty())
		return false;

	// Prefer wired, default interfaces.
	for (std::list< net::SocketAddressIPv4::Interface >::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
	{
		if (i->type == net::SocketAddressIPv4::ItDefault)
		{
			outInterface = *i;
			return true;
		}
	}

	// Prefer wifi over vpn.
	for (std::list< net::SocketAddressIPv4::Interface >::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
	{
		if (i->type == net::SocketAddressIPv4::ItWiFi)
		{
			outInterface = *i;
			return true;
		}
	}

	// No one left except vpn.
	outInterface = interfaces.front();
	return true;
}

	}
}
