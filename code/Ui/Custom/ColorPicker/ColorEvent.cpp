#include "Ui/Custom/ColorPicker/ColorEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ColorEvent", ColorEvent, Event)

ColorEvent::ColorEvent(EventSubject* sender, const Color4ub& color)
:	Event(sender)
,	m_color(color)
{
}

const Color4ub& ColorEvent::getColor() const
{
	return m_color;
}

		}
	}
}
