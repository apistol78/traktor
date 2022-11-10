/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Event.h"
#include "Core/Thread/Result.h"

namespace traktor
{
	namespace
	{

Event s_eventStatus;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Result", Result, Object)

Result::Result()
:	m_ready(false)
,	m_succeeded(false)
{
}

Result::Result(bool succeed)
:	m_ready(true)
,	m_succeeded(succeed)
{
}

void Result::succeed()
{
	if (!m_ready)
	{
		m_ready = true;
		m_succeeded = true;
		s_eventStatus.broadcast();
		deferred();
	}
}

void Result::fail()
{
	if (!m_ready)
	{
		m_ready = true;
		m_succeeded = false;
		s_eventStatus.broadcast();
		deferred();
	}
}

bool Result::ready() const
{
	return m_ready;
}

bool Result::succeeded() const
{
	wait();
	return m_succeeded;
}

void Result::defer(IDeferred* _deferred)
{
	T_ASSERT(_deferred != nullptr);

	if (m_deferred == nullptr)
		addRef(nullptr);

	m_deferred = _deferred;
	deferred();
}

void Result::wait() const
{
	while (!m_ready)
		s_eventStatus.wait(100);
}

void Result::deferred()
{
	if (m_ready && m_deferred)
	{
		m_deferred->dispatch(*this);
		m_deferred = nullptr;
		release(nullptr);
	}
}

}
