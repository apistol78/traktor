#include "Ui/Events/KeyDownEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.KeyDownEvent", KeyDownEvent, Event)

KeyDownEvent::KeyDownEvent(
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

VirtualKey KeyDownEvent::getVirtualKey() const
{
	return m_virtualKey;
}

uint32_t KeyDownEvent::getSystemKey() const
{
	return m_systemKey;
}

wchar_t KeyDownEvent::getCharacter() const
{
	return m_character;
}

	}
}
