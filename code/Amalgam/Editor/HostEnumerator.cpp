#include "Amalgam/Editor/HostEnumerator.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/NetworkService.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.HostEnumerator", HostEnumerator, Object)

HostEnumerator::HostEnumerator(const PropertyGroup* settings, net::DiscoveryManager* discoveryManager)
:	m_discoveryManager(discoveryManager)
{
	std::vector< std::wstring > hosts = settings->getProperty< std::vector< std::wstring > >(L"Amalgam.RemoteHosts");
	for (std::vector< std::wstring >::const_iterator i = hosts.begin(); i != hosts.end(); ++i)
	{
		Host h;
		h.host = *i;
		h.description = *i;
		h.local = false;
		m_manual.push_back(h);
	}
}

int32_t HostEnumerator::count() const
{
	// Get new hosts if updated from thread.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		if (!m_pending.empty())
		{
			m_hosts = m_pending;
			m_pending.clear();
		}
	}
	return int32_t(m_hosts.size());
}

const std::wstring& HostEnumerator::getHost(int32_t index) const
{
	return m_hosts[index].host;
}

int32_t HostEnumerator::getRemotePort(int32_t index) const
{
	return m_hosts[index].remotePort;
}

int32_t HostEnumerator::getHttpPort(int32_t index) const
{
	return m_hosts[index].httpPort;
}

const std::wstring& HostEnumerator::getDescription(int32_t index) const
{
	return m_hosts[index].description;
}

bool HostEnumerator::supportPlatform(int32_t index, const std::wstring& platform) const
{
	if (index >= 0 && index < int32_t(m_hosts.size()))
	{
		const std::vector< std::wstring >& platforms = m_hosts[index].platforms;
		if (!platforms.empty())
			return std::find(platforms.begin(), platforms.end(), platform) != platforms.end();
		else
			return true;
	}
	else
		return false;
}

bool HostEnumerator::isLocal(int32_t index) const
{
	if (index >= 0 && index < int32_t(m_hosts.size()))
		return m_hosts[index].local;
	else
		return false;
}

void HostEnumerator::update()
{
	RefArray< net::NetworkService > services;
	if (!m_discoveryManager->findServices< net::NetworkService >(services))
		return;

	net::SocketAddressIPv4::Interface itf;
	net::SocketAddressIPv4::getBestInterface(itf);

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		m_pending.clear();
		m_pending.insert(m_pending.end(), m_manual.begin(), m_manual.end());

		for (RefArray< net::NetworkService >::const_iterator i = services.begin(); i != services.end(); ++i)
		{
			if ((*i)->getType() != L"RemoteTools/Server")
				continue;

			const PropertyGroup* properties = (*i)->getProperties();
			if (!properties)
				continue;

			Host h;
			h.description = properties->getProperty< std::wstring >(L"Description");
			h.host = properties->getProperty< std::wstring >(L"Host");
			h.remotePort = properties->getProperty< int32_t >(L"RemotePort");
			h.httpPort = properties->getProperty< int32_t >(L"HttpPort");
			h.platforms = properties->getProperty< std::vector< std::wstring > >(L"Platforms");
			h.local = bool(itf.addr != 0 && itf.addr->getHostName() == h.host);
			m_pending.push_back(h);
		}

		std::sort(m_pending.begin(), m_pending.end());
	}
}

HostEnumerator::Host::Host()
:	remotePort(0)
,	httpPort(0)
,	local(false)
{
}

bool HostEnumerator::Host::operator < (const Host& h) const
{
	return description < h.description;
}

	}
}
