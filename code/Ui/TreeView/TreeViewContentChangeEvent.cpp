#include "Ui/TreeView/TreeViewContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TreeViewContentChangeEvent", TreeViewContentChangeEvent, ContentChangeEvent)

TreeViewContentChangeEvent::TreeViewContentChangeEvent(EventSubject* sender, TreeViewItem* item, const std::wstring& originalText)
:	ContentChangeEvent(sender)
,	m_item(item)
,	m_originalText(originalText)
{
}

TreeViewItem* TreeViewContentChangeEvent::getItem() const
{
	return m_item;
}

const std::wstring& TreeViewContentChangeEvent::getOriginalText() const
{
	return m_originalText;
}

	}
}
