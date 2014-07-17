#include "Ui/Custom/PreviewList/PreviewSelectionChangeEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.PreviewSelectionChangeEvent", PreviewSelectionChangeEvent, SelectionChangeEvent)

PreviewSelectionChangeEvent::PreviewSelectionChangeEvent(EventSubject* sender, PreviewItem* item)
:	SelectionChangeEvent(sender)
,	m_item(item)
{
}

PreviewItem* PreviewSelectionChangeEvent::getItem() const
{
	return m_item;
}

		}
	}
}
