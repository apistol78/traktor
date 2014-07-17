#include "Amalgam/Editor/Ui/TargetPlayEvent.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetPlayEvent", TargetPlayEvent, ui::Event)

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
