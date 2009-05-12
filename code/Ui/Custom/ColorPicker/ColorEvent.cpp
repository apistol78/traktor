#include "Ui/Custom/ColorPicker/ColorEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ColorEvent", ColorEvent, Event)

ColorEvent::ColorEvent(EventSubject* sender, Object* item, const Color& color)
:	Event(sender, item)
,	m_color(color)
{
}

const Color& ColorEvent::getColor() const
{
	return m_color;
}

		}
	}
}
