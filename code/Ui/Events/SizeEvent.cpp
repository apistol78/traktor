#include "Ui/Events/SizeEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.SizeEvent", SizeEvent, Event)

SizeEvent::SizeEvent(EventSubject* sender, Object* item, const Size& size)
:	Event(sender, item)
,	m_size(size)
{
}

const Size& SizeEvent::getSize() const
{
	return m_size;
}

int SizeEvent::getWidth() const
{
	return m_size.cx;
}

int SizeEvent::getHeight() const
{
	return m_size.cy;
}

	}
}
