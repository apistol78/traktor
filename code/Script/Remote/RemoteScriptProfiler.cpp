/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Net/BidirectionalObjectTransport.h"
#include "Script/Remote/RemoteScriptProfiler.h"

namespace traktor::script
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.RemoteScriptProfiler", RemoteScriptProfiler, script::IScriptProfiler)

RemoteScriptProfiler::RemoteScriptProfiler(net::BidirectionalObjectTransport* transport)
:	m_transport(transport)
{
}

void RemoteScriptProfiler::addListener(IListener* listener)
{
	m_listeners.push_back(listener);
}

void RemoteScriptProfiler::removeListener(IListener* listener)
{
	auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
	if (it != m_listeners.end())
		m_listeners.erase(it);
}

void RemoteScriptProfiler::notifyListeners(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration)
{
	for (auto listener : m_listeners)
		listener->callMeasured(scriptId, function, callCount, inclusiveDuration, exclusiveDuration);
}

}
