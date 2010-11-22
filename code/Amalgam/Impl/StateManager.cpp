#include "Core/Thread/Acquire.h"
#include "Amalgam/IState.h"
#include "Amalgam/Impl/StateManager.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.StateManager", StateManager, IStateManager)

void StateManager::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	
	if (m_current)
		m_current->leave();

	m_current = 0;
	m_next = 0;
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

bool StateManager::performTransition()
{
	T_ASSERT (m_next);

	if (m_current)
	{
		m_current->leave();
		m_current = 0;
	}

	m_current = m_next;
	m_next = 0;

	if (m_current)
		m_current->enter();

	m_lock.release();
	return true;
}

void StateManager::enter(IState* state)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (m_current)
		m_next = state;
	else
	{
		m_current = state;
		m_current->enter();
	}
}

	}
}
