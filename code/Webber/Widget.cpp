#include "Webber/Widget.h"

namespace traktor
{
	namespace wbr
	{
		namespace
		{
		
int32_t allocateWidgetId()
{
	static int32_t s_widgetNextId = 1;
	return s_widgetNextId++;
}

		}
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Widget", Widget, Object)

Widget::Widget()
:	m_id(allocateWidgetId())
{
}

	}
}
