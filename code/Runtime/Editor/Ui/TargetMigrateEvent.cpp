#include "Runtime/Editor/Ui/TargetMigrateEvent.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetMigrateEvent", TargetMigrateEvent, ui::Event)

TargetMigrateEvent::TargetMigrateEvent(ui::EventSubject* sender, TargetInstance* instance)
:	Event(sender)
,	m_instance(instance)
{
}

TargetInstance* TargetMigrateEvent::getInstance() const
{
	return m_instance;
}

	}
}
