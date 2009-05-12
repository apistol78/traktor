#include "Ui/Events/KeyEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.KeyEvent", KeyEvent, Event)

KeyEvent::KeyEvent(EventSubject* sender, Object* item, int keyCode)
:	Event(sender, item)
,	m_keyCode(keyCode)
{
}

int KeyEvent::getKeyCode() const
{
	return m_keyCode;
}

	}
}
