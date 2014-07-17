#include "Ui/Events/KeyUpEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.KeyUpEvent", KeyUpEvent, Event)

KeyUpEvent::KeyUpEvent(
	EventSubject* sender,
	VirtualKey virtualKey,
	uint32_t systemKey,
	wchar_t character
)
:	Event(sender)
,	m_virtualKey(virtualKey)
,	m_systemKey(systemKey)
,	m_character(character)
{
}

VirtualKey KeyUpEvent::getVirtualKey() const
{
	return m_virtualKey;
}

uint32_t KeyUpEvent::getSystemKey() const
{
	return m_systemKey;
}

wchar_t KeyUpEvent::getCharacter() const
{
	return m_character;
}

	}
}
