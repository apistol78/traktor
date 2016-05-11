#include <algorithm>
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Flash/Collectable.h"
#include "Flash/GC.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

static GC* s_instance = 0;

		}

GC& GC::getInstance()
{
	if (!s_instance)
	{
		s_instance = new GC();
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

void GC::addCandidate(Collectable* object)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (object->m_traceColor != Collectable::TcGray);
	T_ASSERT (object->m_traceColor != Collectable::TcWhite);
	
	object->m_index = m_candidates.size();
	m_candidates.push_back(object);
}

void GC::removeCandidate(Collectable* object)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (object->m_traceColor != Collectable::TcGray);
	T_ASSERT (object->m_traceColor != Collectable::TcWhite);
	
	uint32_t idx = object->m_index;
	object->m_index = ~0U;

	T_ASSERT (idx != ~0U);

	Collectable* back = m_candidates.back();
	if (back != object)
	{
		back->m_index = idx;
		m_candidates[idx] = back;
	}

	m_candidates.pop_back();
}

uint32_t GC::getCandidateCount() const
{
	return m_candidates.size();
}

void GC::collectCycles(bool full)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (m_candidates.empty())
		return;
	
	do
	{
#if defined(_DEBUG)
		// Ensure root candidates are initially colored reasonably.
		for (uint32_t i = 0; i < m_candidates.size(); ++i)
		{
			Collectable* candidate = m_candidates[i];
			T_ASSERT (candidate->m_index == i);
			T_ASSERT (
				candidate->m_traceColor == Collectable::TcBlack ||
				candidate->m_traceColor == Collectable::TcPurple
			);
		}
#endif

		// Mark roots.
		for (uint32_t i = 0; i < m_candidates.size(); )
		{
			Collectable* candidate = m_candidates[i];
			T_ASSERT (candidate->m_index == i);
			T_ASSERT (candidate->m_traceColor != Collectable::TcWhite);

			if (candidate->m_traceColor == Collectable::TcPurple)
			{
				candidate->traceMarkGray();
				++i;
			}
			else
			{
				T_ASSERT (
					candidate->m_traceColor == Collectable::TcBlack ||
					candidate->m_traceColor == Collectable::TcGray
				);
				candidate->m_traceBuffered = false;

				Collectable* back = m_candidates.back();
				if (back != candidate)
				{
					back->m_index = i;
					m_candidates[i] = back;
				}

				candidate->m_index = ~0U;
				m_candidates.pop_back();
			}
		}

		// Scan roots.
		for (uint32_t i = 0; i < m_candidates.size(); ++i)
		{
			Collectable* candidate = m_candidates[i];
			candidate->traceScan();
		}

		// Collect roots.
		while (!m_candidates.empty())
		{
			Collectable* candidate = m_candidates.back();

			// If we've found a purple object then it's been added during
			// collection of roots thus we need to make another trace round.
			if (candidate->m_traceColor == Collectable::TcPurple)
				break;
			
			m_candidates.pop_back();

			candidate->m_index = ~0U;
			candidate->m_traceBuffered = false;
			candidate->traceCollectWhite();
		}
	}
	while (full && !m_candidates.empty());
}

void GC::destroy()
{
	T_ASSERT (s_instance == this);
	delete s_instance, s_instance = 0;
}

GC::~GC()
{
	collectCycles(true);
}

	}
}
