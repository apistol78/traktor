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
	struct Duration
	{
		uint32_t count;
		double seconds;
	};

	void begin(const TypeInfo& pipelineType);

	void end(const TypeInfo& pipelineType);

	const SmallMap< const TypeInfo*, Duration >& getDurations() const { return m_durations; }

private:
	struct Scope
	{
		const TypeInfo* pipelineType;
		Scope* parent;
		double start;
		double child;
	};

	Timer m_timer;
	ThreadLocal m_scope;
	SmallMap< const TypeInfo*, Duration > m_durations;
	CriticalSection m_lock;
};

}
