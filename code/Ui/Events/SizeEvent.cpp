#include "Ui/Events/SizeEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.SizeEvent", SizeEvent, Event)

SizeEvent::SizeEvent(EventSubject* sender, const Size& size)
:	Event(sender)
,	m_size(size)
{
}

const Size& SizeEvent::getSize() const
{
	return m_size;
}

int32_t SizeEvent::getWidth() const
{
	return m_size.cx;
}

int32_t SizeEvent::getHeight() const
{
	return m_size.cy;
}

	}
}
