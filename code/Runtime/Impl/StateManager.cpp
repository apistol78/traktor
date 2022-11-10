/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Thread/Acquire.h"
#include "Runtime/IState.h"
#include "Runtime/Impl/StateManager.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.StateManager", StateManager, IStateManager)

void StateManager::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT(!m_next);

	if (m_current)
	{
		m_current->leave();
		m_current = nullptr;
	}
}

void StateManager::enter(IState* state)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_next = state;
}

bool StateManager::beginTransition()
{
	if (!m_lock.wait())
		return false;

	if (m_next)
		return true;
	else
	{
		m_lock.release();
		return false;
	}
}

void StateManager::leaveCurrent()
{
	T_ASSERT(m_next);

	if (m_current)
	{
		m_current->leave();
		m_current = nullptr;
	}
}

void StateManager::enterNext()
{
	T_ASSERT(m_next);
	T_ASSERT(!m_current);

	m_current = m_next;
	m_next = nullptr;

	if (m_current)
		m_current->enter();

	m_lock.release();
}

	}
}
