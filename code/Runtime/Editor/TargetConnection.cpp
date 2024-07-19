/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Runtime/Editor/TargetConnection.h"
#include "Runtime/Editor/TargetScriptDebugger.h"
#include "Runtime/Editor/TargetScriptProfiler.h"
#include "Runtime/Target/ScriptProfilerCallMeasured.h"
#include "Runtime/Target/TargetLog.h"
#include "Runtime/Target/TargetProfilerDictionary.h"
#include "Runtime/Target/TargetProfilerEvents.h"
#include "Script/Editor/IScriptDebuggerSessions.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetConnection", TargetConnection, Object)

TargetConnection::TargetConnection(const std::wstring &name, net::BidirectionalObjectTransport *transport, ILogTarget *targetLog, script::IScriptDebuggerSessions *targetDebuggerSessions)
:	m_name(name)
,	m_transport(transport)
,	m_targetLog(targetLog)
,	m_targetDebuggerSessions(targetDebuggerSessions)
,	m_profilerEventsCallback(0)
{
	m_targetDebugger = new TargetScriptDebugger(m_transport);
	m_targetProfiler = new TargetScriptProfiler(m_transport);
	m_targetDebuggerSessions->beginSession(m_targetDebugger, m_targetProfiler);
}

TargetConnection::~TargetConnection()
{
	T_ASSERT(!m_targetProfiler);
	T_ASSERT(!m_targetDebugger);
	T_ASSERT(!m_transport);
}

void TargetConnection::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (m_targetDebuggerSessions)
	{
		m_targetDebuggerSessions->endSession(m_targetDebugger, m_targetProfiler);
		m_targetDebuggerSessions = nullptr;
		m_targetProfiler = nullptr;
		m_targetDebugger = nullptr;
	}

	m_targetLog = nullptr;

	if (m_transport)
	{
		m_transport->close();
		m_transport = nullptr;
	}
}

void TargetConnection::shutdown()
{
	destroy();
}

bool TargetConnection::update()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_transport)
		return false;

	if (!m_transport->connected())
	{
		destroy();
		return false;
	}

	{
		bool receivedPerfSet = false;
		Ref< TargetPerfSet > perfSet;
		while (m_transport->recv< TargetPerfSet >(0, perfSet) == net::BidirectionalObjectTransport::Result::Success)
		{
			m_performance[&type_of(perfSet)] = perfSet;
			receivedPerfSet = true;
		}
		if (receivedPerfSet)
		{
			if (m_profilerEventsCallback)
				m_profilerEventsCallback->receivedPerfSets();
		}
	}

	{
		Ref< TargetProfilerDictionary > profilerDictionary;
		while (m_transport->recv< TargetProfilerDictionary >(0, profilerDictionary) == net::BidirectionalObjectTransport::Result::Success)
		{
			m_dictionary = profilerDictionary->getDictionary();
			if (m_profilerEventsCallback)
				m_profilerEventsCallback->receivedProfilerDictionary(m_dictionary);
		}
	}

	{
		Ref< TargetProfilerEvents > profilerEvents;
		while (m_transport->recv< TargetProfilerEvents >(0, profilerEvents) == net::BidirectionalObjectTransport::Result::Success)
		{
			if (m_profilerEventsCallback)
				m_profilerEventsCallback->receivedProfilerEvents(profilerEvents->getCurrentTime(), profilerEvents->getEvents());
		}
	}

	{
		Ref< TargetLog > tlog;
		while (m_transport->recv< TargetLog >(0, tlog) == net::BidirectionalObjectTransport::Result::Success)
			m_targetLog->log(tlog->getThreadId(), tlog->getLevel(), tlog->getText().c_str());
	}

	{
		Ref< ScriptProfilerCallMeasured > measured;
		while (m_transport->recv< ScriptProfilerCallMeasured >(0, measured) == net::BidirectionalObjectTransport::Result::Success)
		{
			m_targetProfiler->notifyListeners(
				measured->getScriptId(),
				measured->getFunction(),
				measured->getCallCount(),
				measured->getInclusiveDuration(),
				measured->getExclusiveDuration()
			);
		}
	}

	if (m_targetDebugger)
		m_targetDebugger->update();

	return true;
}

const TargetPerfSet* TargetConnection::getPerformance(const TypeInfo& perfSetType)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	auto it = m_performance.find(&perfSetType);
	if (it != m_performance.end())
		return it->second;

	Ref< TargetPerfSet > perfSet = (TargetPerfSet*)perfSetType.createInstance();
	m_performance.insert(&perfSetType, perfSet);
	return perfSet;
}

void TargetConnection::setProfilerEventsCallback(IProfilerEventsCallback* profilerEventsCallback)
{
	if ((m_profilerEventsCallback = profilerEventsCallback) != nullptr)
		m_profilerEventsCallback->receivedProfilerDictionary(m_dictionary);
}

}
