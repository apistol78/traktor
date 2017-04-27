/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	m_candidates.push_back(object);
}

void GC::removeCandidate(Collectable* object)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (object->m_traceColor != Collectable::TcGray);
	T_ASSERT (object->m_traceColor != Collectable::TcWhite);
	m_candidates.remove(object);
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
		for (IntrusiveList< Collectable >::iterator i = m_candidates.begin(); i != m_candidates.end(); ++i)
		{
			Collectable* candidate = *i;
			T_ASSERT (
				candidate->m_traceColor == Collectable::TcBlack ||
				candidate->m_traceColor == Collectable::TcPurple
			);
		}
#endif

		// Mark roots.
		for (IntrusiveList< Collectable >::iterator i = m_candidates.begin(); i != m_candidates.end(); )
		{
			Collectable* candidate = *i;
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
				i = m_candidates.erase(i);
			}
		}

		// Scan roots.
		for (IntrusiveList< Collectable >::iterator i = m_candidates.begin(); i != m_candidates.end(); ++i)
		{
			Collectable* candidate = *i;
			candidate->traceScan();
		}

		// Collect roots.
		while (!m_candidates.empty())
		{
			Collectable* candidate = m_candidates.front();

			// If we've found a purple object then it's been added during
			// collection of roots thus we need to make another trace round.
			if (candidate->m_traceColor == Collectable::TcPurple)
				break;
			
			m_candidates.pop_front();

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
