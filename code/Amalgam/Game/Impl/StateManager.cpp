/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Thread/Acquire.h"
#include "Amalgam/Game/IState.h"
#include "Amalgam/Game/Impl/StateManager.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.StateManager", StateManager, IStateManager)

void StateManager::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (!m_next);
	
	if (m_current)
	{
		m_current->flush();
		m_current->leave();
		m_current = 0;
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
	T_ASSERT (m_next);

	if (m_current)
	{
		m_current->flush();
		m_current->leave();
		m_current = 0;
	}
}

void StateManager::enterNext()
{
	T_ASSERT (m_next);
	T_ASSERT (!m_current);

	m_current = m_next;
	m_next = 0;

	if (m_current)
		m_current->enter();

	m_lock.release();
}

	}
}
