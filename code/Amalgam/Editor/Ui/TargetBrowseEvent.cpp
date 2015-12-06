#include "Amalgam/Editor/Ui/TargetBrowseEvent.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetBrowseEvent", TargetBrowseEvent, ui::Event)

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
