#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Thread/CriticalSection.h"
#include "Core/Thread/ThreadLocal.h"
#include "Core/Timer/Timer.h"

namespace traktor
{
	namespace editor
	{

class PipelineProfiler : public Object
{
	T_RTTI_CLASS;

public:
	PipelineProfiler();

	void begin(const TypeInfo& pipelineType);

	void end(const TypeInfo& pipelineType);

	const SmallMap< const TypeInfo*, double >& getDurations() const { return m_durations; }

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
	SmallMap< const TypeInfo*, double > m_durations;
	CriticalSection m_lock;
};

	}
}
