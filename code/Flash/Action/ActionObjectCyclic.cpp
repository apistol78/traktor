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
	m_candidates.push_back(object);
}

void ActionObjectCyclic::removeCandidate(ActionObject* object)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_candidates.remove(object);
}

void ActionObjectCyclic::collectCycles()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	// Mark roots.
	for (std::list< ActionObject* >::iterator i = m_candidates.begin(); i != m_candidates.end(); )
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
	for (std::list< ActionObject* >::iterator i = m_candidates.begin(); i != m_candidates.end(); ++i)
	{
		ActionObject* candidate = *i;
		candidate->traceScan();
	}

	// Collect roots.
	while (!m_candidates.empty())
	{
		ActionObject* candidate = m_candidates.front(); m_candidates.pop_front();
		candidate->m_traceBuffered = false;
		candidate->traceCollectWhite();
	}
}

void ActionObjectCyclic::destroy()
{
	T_ASSERT (s_instance == this);
	delete s_instance, s_instance = 0;
}

ActionObjectCyclic::~ActionObjectCyclic()
{
	collectCycles();
}

	}
}
