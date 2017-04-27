/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Thread/Event.h"
#include "Online/Result.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

Event s_eventStatus;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.Result", Result, Object)

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
		m_deferred = 0;
	}
}

	}
}
