#include "Ui/Events/KeyEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.KeyEvent", KeyEvent, Event)

KeyEvent::KeyEvent(
	EventSubject* sender,
	Object* item,
	VirtualKey virtualKey,
	uint32_t systemKey,
	wchar_t character
)
:	Event(sender, item)
,	m_virtualKey(virtualKey)
,	m_systemKey(systemKey)
,	m_character(character)
{
}

VirtualKey KeyEvent::getVirtualKey() const
{
	return m_virtualKey;
}

uint32_t KeyEvent::getSystemKey() const
{
	return m_systemKey;
}

wchar_t KeyEvent::getCharacter() const
{
	return m_character;
}

	}
}
