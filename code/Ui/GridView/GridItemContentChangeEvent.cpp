#include "Ui/GridView/GridItemContentChangeEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridItemContentChangeEvent", GridItemContentChangeEvent, ContentChangeEvent)

GridItemContentChangeEvent::GridItemContentChangeEvent(EventSubject* sender, GridItem* item, const std::wstring& originalText)
:	ContentChangeEvent(sender)
,	m_item(item)
,	m_originalText(originalText)
{
}

GridItem* GridItemContentChangeEvent::getItem() const
{
	return m_item;
}

const std::wstring& GridItemContentChangeEvent::getOriginalText() const
{
	return m_originalText;
}

	}
}
