#include "Runtime/Events/ReconfigureEvent.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.ReconfigureEvent", ReconfigureEvent, Object)

ReconfigureEvent::ReconfigureEvent(bool finished, int32_t result)
:	m_finished(finished)
,	m_result(result)
{
}

bool ReconfigureEvent::isFinished() const
{
	return m_finished;
}

int32_t ReconfigureEvent::getResult() const
{
	return m_result;
}

	}
}
