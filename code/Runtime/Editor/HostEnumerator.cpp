/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Editor/HostEnumerator.h"
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Net/Discovery/NetworkService.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.HostEnumerator", HostEnumerator, Object)

HostEnumerator::HostEnumerator(const PropertyGroup* settings, net::DiscoveryManager* discoveryManager)
:	m_discoveryManager(discoveryManager)
{
	for (const auto& host : settings->getProperty< AlignedVector< std::wstring > >(L"Runtime.RemoteHosts"))
	{
		Host h;
		h.host = host;
		h.description = host;
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

const std::wstring& HostEnumerator::getDescription(int32_t index) const
{
	return m_hosts[index].description;
}

bool HostEnumerator::supportPlatform(int32_t index, const std::wstring& platform) const
{
	if (index >= 0 && index < int32_t(m_hosts.size()))
	{
		const auto& platforms = m_hosts[index].platforms;
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

		for (auto service : services)
		{
			if (service->getType() != L"RemoteTools/Server")
				continue;

			const PropertyGroup* properties = service->getProperties();
			if (!properties)
				continue;

			std::wstring host = properties->getProperty< std::wstring >(L"Host");
			
			size_t p = host.find(L':');
			if (p != host.npos)
				host = host.substr(0, p);

			Host h;
			h.description = properties->getProperty< std::wstring >(L"Description");
			h.host = properties->getProperty< std::wstring >(L"Host");
			h.platforms = properties->getProperty< AlignedVector< std::wstring > >(L"Platforms");
			h.local = bool(itf.addr != 0 && itf.addr->getHostName() == host);
			m_pending.push_back(h);
		}

		std::sort(m_pending.begin(), m_pending.end());
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
