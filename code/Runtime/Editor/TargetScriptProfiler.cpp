/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Net/BidirectionalObjectTransport.h"
#include "Runtime/Editor/TargetScriptProfiler.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetScriptProfiler", TargetScriptProfiler, script::IScriptProfiler)

TargetScriptProfiler::TargetScriptProfiler(net::BidirectionalObjectTransport* transport)
:	m_transport(transport)
{
}

void TargetScriptProfiler::addListener(IListener* listener)
{
	m_listeners.push_back(listener);
}

void TargetScriptProfiler::removeListener(IListener* listener)
{
	m_listeners.remove(listener);
}

void TargetScriptProfiler::notifyListeners(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration)
{
	for (auto listener : m_listeners)
		listener->callMeasured(scriptId, function, callCount, inclusiveDuration, exclusiveDuration);
}

}
