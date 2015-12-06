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
	std::vector< std::wstring > hosts = settings->getProperty< PropertyStringArray >(L"Amalgam.RemoteHosts");
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

		m_hosts.clear();
		m_hosts.insert(m_hosts.end(), m_manual.begin(), m_manual.end());

		for (RefArray< net::NetworkService >::const_iterator i = services.begin(); i != services.end(); ++i)
		{
			if ((*i)->getType() != L"RemoteTools/Server")
				continue;

			const PropertyGroup* properties = (*i)->getProperties();
			if (!properties)
				continue;

			Host h;
			h.description = properties->getProperty< PropertyString >(L"Description");
			h.host = properties->getProperty< PropertyString >(L"Host");
			h.remotePort = properties->getProperty< PropertyInteger >(L"RemotePort");
			h.httpPort = properties->getProperty< PropertyInteger >(L"HttpPort");
			h.platforms = properties->getProperty< PropertyStringArray >(L"Platforms");
			h.local = bool(itf.addr != 0 && itf.addr->getHostName() == h.host);
			m_hosts.push_back(h);
		}

		std::sort(m_hosts.begin(), m_hosts.end());
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
