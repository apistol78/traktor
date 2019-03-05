#include "Runtime/Editor/Ui/TargetPlayEvent.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetPlayEvent", TargetPlayEvent, ui::Event)

TargetPlayEvent::TargetPlayEvent(ui::EventSubject* sender, TargetInstance* instance)
:	Event(sender)
,	m_instance(instance)
{
}

TargetInstance* TargetPlayEvent::getInstance() const
{
	return m_instance;
}

	}
}
