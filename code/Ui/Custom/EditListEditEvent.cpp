#include "Ui/Custom/EditListEditEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.EditListEditEvent", EditListEditEvent, ContentChangeEvent)

EditListEditEvent::EditListEditEvent(EventSubject* sender, Edit* edit, int32_t index, const std::wstring& text)
:	ContentChangeEvent(sender)
,	m_edit(edit)
,	m_index(index)
,	m_text(text)
{
}

Edit* EditListEditEvent::getEdit() const
{
	return m_edit;
}

int32_t EditListEditEvent::getIndex() const
{
	return m_index;
}

const std::wstring& EditListEditEvent::getText() const
{
	return m_text;
}

		}
	}
}
