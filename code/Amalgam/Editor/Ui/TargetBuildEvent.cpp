#include "Amalgam/Editor/Ui/TargetBuildEvent.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetBuildEvent", TargetBuildEvent, ui::Event)

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
