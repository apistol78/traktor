/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Core/CycleRefDebugger.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace
	{

template < typename Pair >
struct Pair1stEqual
{
	const typename Pair::first_type& m_lh;

	Pair1stEqual(const typename Pair::first_type& lh)
	:	m_lh(lh)
	{
	}

	bool operator () (const Pair& rh) const
	{
		return m_lh == rh.first;
	}
};

template < typename Pair >
struct Pair2ndEqual
{
	const typename Pair::second_type& m_lh;

	Pair2ndEqual(const typename Pair::second_type& lh)
	:	m_lh(lh)
	{
	}

	bool operator () (const Pair& rh) const
	{
		return m_lh == rh.second;
	}
};

	}

CycleRefDebugger::CycleRefDebugger()
:	m_tag(0)
{
}

void CycleRefDebugger::addObject(void* object, size_t size)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	ObjInfo& info = m_objects[object];
	info.size = size;
	info.nrefs = 0;
	info.ntrace = 0;
	info.tag = 0;
}

void CycleRefDebugger::removeObject(void* object)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< void*, ObjInfo >::iterator i = m_objects.find(object);
	T_ASSERT (i != m_objects.end());
	T_ASSERT (i->second.nrefs == 0);
	m_objects.erase(i);
}

void CycleRefDebugger::addObjectRef(void* ref, void* object)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< void*, ObjInfo >::iterator i = m_objects.find(object);
	if (i != m_objects.end())
		i->second.nrefs++;
	// else then we're adding a reference to an object
	// not allocated on heap.

	// Find object containing reference.
	uint8_t* refp = (uint8_t*)ref;
	for (std::map< void*, ObjInfo >::iterator j = m_objects.begin(); j != m_objects.end(); ++j)
	{
		uint8_t* objp = (uint8_t*)j->first;
		if (refp > objp + j->second.size)
			continue;
		if (refp >= objp)
		{
			j->second.refs.push_back(std::make_pair(
				object,
				i != m_objects.end() ? &i->second : 0
			));
		}
		break;
	}
}

void CycleRefDebugger::removeObjectRef(void* ref, void* object)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	std::map< void*, ObjInfo >::iterator i = m_objects.find(object);
	if (i != m_objects.end())
		i->second.nrefs--;
	// else then we're removing a reference from an object
	// not allocated on heap.

	// Find object containing reference.
	uint8_t* refp = (uint8_t*)ref;
	for (std::map< void*, ObjInfo >::iterator j = m_objects.begin(); j != m_objects.end(); ++j)
	{
		uint8_t* objp = (uint8_t*)j->first;
		if (refp > objp + j->second.size)
			continue;
		if (refp >= objp)
		{
			std::vector< std::pair< void*, ObjInfo* > >::iterator k = std::find_if(
				j->second.refs.begin(),
				j->second.refs.end(),
				Pair1stEqual< std::pair< void*, ObjInfo* > >(object)
			);
			T_FATAL_ASSERT (k != j->second.refs.end());
			j->second.refs.erase(k);
		}
		break;
	}

	// Check for orphan cycles.
	if (i != m_objects.end() && i->second.nrefs > 0)
	{
		i->second.ntrace = 0;
		i->second.tag = ++m_tag;

		for (std::vector< std::pair< void*, ObjInfo* > >::iterator j = i->second.refs.begin(); j != i->second.refs.end(); ++j)
			trace(j->second);

		if (i->second.ntrace >= i->second.nrefs)
		{
			// Orphan candidate; check if any visited object is still reachable.
			bool reachable = false;
			for (std::map< void*, ObjInfo >::iterator j = m_objects.begin(); j != m_objects.end(); ++j)
			{
				if (j->second.tag != m_tag)
					continue;
				if (j->second.nrefs > j->second.ntrace)
				{
					reachable = true;
					break;
				}
			}
			T_FATAL_ASSERT_M (reachable, L"Orphan cycle detected");
		}
	}	
}

void CycleRefDebugger::trace(ObjInfo* object)
{
	if (!object)
		return;

	if (object->tag != m_tag)
		object->ntrace = 1;
	else
		object->ntrace++;

	if (object->tag != m_tag)
	{
		object->tag = m_tag;
		for (std::vector< std::pair< void*, ObjInfo* > >::iterator i = object->refs.begin(); i != object->refs.end(); ++i)
			trace(i->second);
	}
}

}
