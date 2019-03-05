#include "Runtime/Editor/Ui/TargetCaptureEvent.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetCaptureEvent", TargetCaptureEvent, ui::Event)

TargetCaptureEvent::TargetCaptureEvent(ui::EventSubject* sender, TargetInstance* instance, int32_t connectionIndex)
:	Event(sender)
,	m_instance(instance)
,	m_connectionIndex(connectionIndex)
{
}

TargetInstance* TargetCaptureEvent::getInstance() const
{
	return m_instance;
}

int32_t TargetCaptureEvent::getConnectionIndex() const
{
	return m_connectionIndex;
}

	}
}
