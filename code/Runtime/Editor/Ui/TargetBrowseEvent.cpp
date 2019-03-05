#include "Runtime/Editor/Ui/TargetBrowseEvent.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetBrowseEvent", TargetBrowseEvent, ui::Event)

TargetBrowseEvent::TargetBrowseEvent(ui::EventSubject* sender, TargetInstance* instance)
:	Event(sender)
,	m_instance(instance)
{
}

TargetInstance* TargetBrowseEvent::getInstance() const
{
	return m_instance;
}

	}
}
