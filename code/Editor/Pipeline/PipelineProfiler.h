/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Thread/CriticalSection.h"
#include "Core/Thread/ThreadLocal.h"
#include "Core/Timer/Timer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class T_DLLCLASS PipelineProfiler : public Object
{
	T_RTTI_CLASS;

public:
	struct Scope
	{
		const wchar_t* id;
		Scope* parent;
		double start;
		double child;
	};
	
	struct Duration
	{
		uint32_t count;
		double seconds;
	};

	void begin(const wchar_t* const id);

	void begin(const TypeInfo& pipelineType) { begin(pipelineType.getName()); }

	void end();

	const SmallMap< const wchar_t*, Duration >& getDurations() const { return m_durations; }

private:
	Timer m_timer;
	ThreadLocal m_scope;
	SmallMap< const wchar_t*, Duration > m_durations;
	CriticalSection m_lock;
};

}
