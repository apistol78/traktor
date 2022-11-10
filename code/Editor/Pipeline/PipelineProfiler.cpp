#include "Core/Thread/Acquire.h"
#include "Editor/Pipeline/PipelineProfiler.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineProfiler", PipelineProfiler, Object)

void PipelineProfiler::begin(const TypeInfo& pipelineType)
{
	Scope* current = new Scope();
	current->pipelineType = &pipelineType;
	current->parent = (Scope*)m_scope.get();
	current->start = m_timer.getElapsedTime();
	current->child = 0.0;
	m_scope.set(current);
}

void PipelineProfiler::end(const TypeInfo& pipelineType)
{
	Scope* current = (Scope*)m_scope.get();
	m_scope.set(current->parent);

	T_FATAL_ASSERT(current->pipelineType == &pipelineType);

	double end = m_timer.getElapsedTime();

	double inclusive = end - current->start;
	double exclusive = inclusive - current->child;
	
	if (current->parent)
		current->parent->child += inclusive;

	{
		T_ANONYMOUS_VAR(Acquire< CriticalSection >)(m_lock);
		auto it = m_durations.find(&pipelineType);
		if (it != m_durations.end())
		{
			it->second.count++;
			it->second.seconds += exclusive;
		}
		else
			m_durations.insert(&pipelineType, { 1, exclusive });
	}

	delete current;
}

}
