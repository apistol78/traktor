#include "Ui/Custom/Sequencer/GroupVisibleEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GroupVisibleEvent", GroupVisibleEvent, Event)

GroupVisibleEvent::GroupVisibleEvent(EventSubject* sender, SequenceGroup* group, bool visible)
:	Event(sender)
,	m_group(group)
,	m_visible(visible)
{
}

SequenceGroup* GroupVisibleEvent::getGroup() const
{
	return m_group;
}

bool GroupVisibleEvent::getVisible() const
{
	return m_visible;
}

		}
	}
}
