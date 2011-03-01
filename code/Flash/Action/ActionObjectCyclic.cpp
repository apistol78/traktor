#include <algorithm>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Flash/Action/ActionObject.h"
#include "Flash/Action/ActionObjectCyclic.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

static ActionObjectCyclic* s_instance = 0;

		}

ActionObjectCyclic& ActionObjectCyclic::getInstance()
{
	if (!s_instance)
	{
		s_instance = new ActionObjectCyclic();
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

void ActionObjectCyclic::addCandidate(ActionObject* object)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (std::find(m_candidates.begin(), m_candidates.end(), object) == m_candidates.end());
	m_candidates.push_back(object);
}

void ActionObjectCyclic::removeCandidate(ActionObject* object)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (std::find(m_candidates.begin(), m_candidates.end(), object) != m_candidates.end());
	m_candidates.remove(object);
}

void ActionObjectCyclic::collectCycles(bool full)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (m_candidates.empty())
		return;
		
	do
	{
		// Mark roots.
		for (IntrusiveList< ActionObject >::iterator i = m_candidates.begin(); i != m_candidates.end(); )
		{
			ActionObject* candidate = *i;
			if (candidate->m_traceColor == ActionObject::TcPurple)
			{
				candidate->traceMarkGray();
				++i;
			}
			else
			{
				candidate->m_traceBuffered = false;
				i = m_candidates.erase(i);
			}
		}

		// Scan roots.
		for (IntrusiveList< ActionObject >::iterator i = m_candidates.begin(); i != m_candidates.end(); ++i)
		{
			ActionObject* candidate = *i;
			candidate->traceScan();
		}

		// Collect roots.
		while (!m_candidates.empty())
		{
			ActionObject* candidate = m_candidates.front();

			// If we've found a purple object then it's been added during
			// collection of roots thus we need to make another trace round.
			if (candidate->m_traceColor == ActionObject::TcPurple)
				break;
			
			m_candidates.pop_front();

			candidate->m_traceBuffered = false;
			candidate->traceCollectWhite();
		}
	}
	while (full && !m_candidates.empty());
}

void ActionObjectCyclic::destroy()
{
	T_ASSERT (s_instance == this);
	delete s_instance, s_instance = 0;
}

ActionObjectCyclic::~ActionObjectCyclic()
{
	collectCycles(true);
}

	}
}
