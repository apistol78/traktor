#include "Ui/Events/TreeViewDragEvent.h"
#include "Ui/TreeViewItem.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeViewDragEvent", TreeViewDragEvent, Event)

TreeViewDragEvent::TreeViewDragEvent(EventSubject* sender, TreeViewItem* dragItem, DragMoment moment, const Point& position)
:	Event(sender, dragItem)
,	m_moment(moment)
,	m_position(position)
,	m_cancelled(false)
{
}

TreeViewDragEvent::DragMoment TreeViewDragEvent::getMoment() const
{
	return m_moment;
}

const Point& TreeViewDragEvent::getPosition() const
{
	return m_position;
}

void TreeViewDragEvent::cancel()
{
	m_cancelled = true;
}

bool TreeViewDragEvent::cancelled() const
{
	return m_cancelled;
}

	}
}
