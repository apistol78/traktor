#include "Amalgam/Events/ReconfigureEvent.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ReconfigureEvent", ReconfigureEvent, IEvent)

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
