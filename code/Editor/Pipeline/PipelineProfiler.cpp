/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#define T_BOXES_USE_MT_LOCK
#include "Core/Class/BoxedAllocator.h"
#include "Core/Thread/Acquire.h"
#include "Editor/Pipeline/PipelineProfiler.h"

namespace traktor::editor
{
	namespace
	{

BoxedAllocator< PipelineProfiler::Scope, 256 > s_allocScope;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineProfiler", PipelineProfiler, Object)

void PipelineProfiler::begin(const wchar_t* const id)
{
	Scope* current = s_allocScope.alloc();
	current->id = id;
	current->parent = (Scope*)m_scope.get();
	current->start = m_timer.getElapsedTime();
	current->child = 0.0;
	m_scope.set(current);
}

void PipelineProfiler::end()
{
	Scope* current = (Scope*)m_scope.get();
	m_scope.set(current->parent);

	const double end = m_timer.getElapsedTime();
	const double inclusive = end - current->start;
	const double exclusive = inclusive - current->child;
	
	if (current->parent)
		current->parent->child += inclusive;

	{
		T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_lock);
		auto it = m_durations.find(current->id);
		if (it != m_durations.end())
		{
			it->second.count++;
			it->second.seconds += exclusive;
		}
		else
			m_durations.insert(current->id, { 1, exclusive });
	}

	s_allocScope.free(current);
}

}
