#include "Amalgam/Editor/HostEnumerator.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Core/Settings/PropertyString.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/NetworkService.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.HostEnumerator", HostEnumerator, Object)

HostEnumerator::HostEnumerator(net::DiscoveryManager* discoveryManager)
:	m_discoveryManager(discoveryManager)
{
}

int32_t HostEnumerator::count() const
{
	return int32_t(m_hosts.size());
}

bool HostEnumerator::getHost(int32_t index, std::wstring& outHost) const
{
	if (index >= 0 && index < int32_t(m_hosts.size()))
	{
		outHost = m_hosts[index].host;
		return true;
	}
	else
		return false;
}

bool HostEnumerator::getDescription(int32_t index, std::wstring& outDescription) const
{
	if (index >= 0 && index < int32_t(m_hosts.size()))
	{
		outDescription = m_hosts[index].description;
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
	if (!m_discoveryManager->findServices< net::NetworkService >(services, 100))
		return;

	net::SocketAddressIPv4::Interface itf;
	net::SocketAddressIPv4::getBestInterface(itf);

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		m_hosts.clear();

		for (RefArray< net::NetworkService >::const_iterator i = services.begin(); i != services.end(); ++i)
		{
			if ((*i)->getType() != L"RemoteTools/Server")
				continue;

			const PropertyGroup* properties = (*i)->getProperties();
			if (!properties)
				continue;

			Host h;
			h.host = properties->getProperty< PropertyString >(L"Host");
			h.description = properties->getProperty< PropertyString >(L"Description");
			h.local = bool(itf.addr->getHostName() == h.host);

			m_hosts.push_back(h);
		}

		std::sort(m_hosts.begin(), m_hosts.end());
	}
}

HostEnumerator::Host::Host()
:	local(false)
{
}

bool HostEnumerator::Host::operator < (const Host& h) const
{
	return description < h.description;
}

	}
}
