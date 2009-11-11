#include "Ui/Event.h"
#include "Ui/Application.h"
#include "Ui/Itf/IEventLoop.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Event", Event, Object)

Event::Event(EventSubject* sender, Object* item)
:	m_sender(sender)
,	m_item(item)
,	m_keyState(WsNone)
,	m_consumed(false)
{
	m_keyState = Application::getInstance().getEventLoop()->getAsyncKeyState();
}

Ref< EventSubject > Event::getSender() const
{
	return m_sender;
}

Ref< Object > Event::getItem() const
{
	return m_item;
}

int Event::getKeyState() const
{
	return m_keyState;
}

void Event::consume()
{
	m_consumed = true;
}

bool Event::consumed() const
{
	return m_consumed;
}
	
	}
}
