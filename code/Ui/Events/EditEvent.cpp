#include "Ui/Events/EditEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.EditEvent", EditEvent, Event)

EditEvent::EditEvent(EventSubject* sender, Object* item, int param, const std::wstring& text)
:	Event(sender, item)
,	m_param(param)
,	m_text(text)
{
}

int EditEvent::getParam() const
{
	return m_param;
}

const std::wstring& EditEvent::getText() const
{
	return m_text;
}

	}
}
