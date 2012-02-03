#include "Amalgam/Editor/HostEnumerator.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
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
		outHost = m_hosts[index];
		return true;
	}
	else
		return false;
}

bool HostEnumerator::getDescription(int32_t index, std::wstring& outDescription) const
{
	if (index >= 0 && index < int32_t(m_descriptions.size()))
	{
		outDescription = m_descriptions[index];
		return true;
	}
	else
		return false;
}

void HostEnumerator::update()
{
	RefArray< net::NetworkService > services;
	if (!m_discoveryManager->findServices< net::NetworkService >(services, 100))
		return;

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		m_hosts.clear();
		m_descriptions.clear();

		for (RefArray< net::NetworkService >::const_iterator i = services.begin(); i != services.end(); ++i)
		{
			if ((*i)->getType() != L"RemoteTools/Server")
				continue;

			m_hosts.push_back((*i)->getHost());
			m_descriptions.push_back((*i)->getDescription());
		}
	}
}

	}
}
