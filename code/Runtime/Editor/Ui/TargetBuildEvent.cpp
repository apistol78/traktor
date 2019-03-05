#include "Runtime/Editor/Ui/TargetBuildEvent.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetBuildEvent", TargetBuildEvent, ui::Event)

TargetBuildEvent::TargetBuildEvent(ui::EventSubject* sender, TargetInstance* instance)
:	Event(sender)
,	m_instance(instance)
{
}

TargetInstance* TargetBuildEvent::getInstance() const
{
	return m_instance;
}

	}
}
