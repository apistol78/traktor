/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Timer/Profiler.h"

namespace traktor
{
	namespace
	{

thread_local uint8_t s_threadIndex;
uint8_t s_threadIndexNext = 0;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Profiler", Profiler, Object)

Profiler& Profiler::getInstance()
{
	static Profiler* s_instance = nullptr;
	if (!s_instance)
	{
		s_instance = new Profiler();
		s_instance->addRef(nullptr);
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

void Profiler::setListener(IReportListener* listener)
{
	m_listener = listener;
}

void Profiler::beginEvent(const std::wstring& name)
{
	if (!m_listener)
		return;

	// Get ID of immutable name.
	uint16_t id = 0;
	{
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_nameIdsLock);
		auto it = m_nameIds.find(name);
		if (it != m_nameIds.end())
			id = it->second;
		else
		{
			id = (uint16_t)m_nameIds.size();
			m_nameIds[name] = id;
			m_dictionary[id] = name;
			m_dictionaryDirty = true;
		}
	}

	// Get event queue for calling thread.
	ThreadEvents* te = static_cast< ThreadEvents* >(m_localThreadEvents.get());
	if (!te)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		te = new ThreadEvents();
		m_threadEvents.push_back(te);
		m_localThreadEvents.set(te);
		s_threadIndex = s_threadIndexNext++;
	}

	// Begin event.
	Event& e = te->events.push_back();
	e.name = id;
	e.threadId = s_threadIndex;
	e.depth = uint8_t(te->events.size() - 1);
	e.start = m_timer.getElapsedTime();
	e.end = 0.0;
}

void Profiler::endEvent()
{
	if (!m_listener)
		return;

	// Get event queue for calling thread.
	ThreadEvents* te = static_cast< ThreadEvents* >(m_localThreadEvents.get());
	T_FATAL_ASSERT (te);

	// End event.
	T_FATAL_ASSERT(!te->events.empty());
	Event& e = te->events.back();
	e.end = m_timer.getElapsedTime();

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		// Merge finished event.
		m_events.push_back(e);
		te->events.pop_back();

		// Report events if we've queued enough.
		if (m_events.size() >= MaxQueuedEvents)
		{
			if (m_dictionaryDirty)
			{
				m_listener->reportProfilerDictionary(m_dictionary);
				m_dictionaryDirty = false;
			}

			m_listener->reportProfilerEvents(m_timer.getElapsedTime(), m_events);
			m_events.resize(0);
		}
	}
}

void Profiler::addEvent(const std::wstring& name, double start, double duration)
{
	if (!m_listener)
		return;

	// Get ID of immutable name.
	uint16_t id = 0;
	{
		T_ANONYMOUS_VAR(Acquire< SpinLock >)(m_nameIdsLock);
		auto it = m_nameIds.find(name);
		if (it != m_nameIds.end())
			id = it->second;
		else
		{
			id = (uint16_t)m_nameIds.size();
			m_nameIds[name] = id;
			m_dictionary[id] = name;
			m_dictionaryDirty = true;
		}
	}

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		
		auto& e = m_events.push_back();
		e.name = id;
		e.threadId = -1;
		e.depth = 0;
		e.start = start;
		e.end = start + duration;

		// Report events if we've queued enough.
		if (m_events.full())
		{
			m_listener->reportProfilerEvents(m_timer.getElapsedTime(), m_events);
			m_events.resize(0);
		}
	}
}

double Profiler::getTime() const
{
	return m_timer.getElapsedTime();
}

Profiler::Profiler()
:	m_dictionaryDirty(false)
{
	m_timer.reset();
}

void Profiler::destroy()
{
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		if (!m_events.empty())
		{
			if (m_listener)
				m_listener->reportProfilerEvents(m_timer.getElapsedTime(), m_events);
			m_events.clear();
		}

		m_listener = nullptr;

		for (auto threadEvent : m_threadEvents)
			delete threadEvent;

		m_threadEvents.clear();
	}
	T_SAFE_RELEASE(this);
}

}
