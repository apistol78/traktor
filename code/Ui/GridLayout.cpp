#include "Ui/GridLayout.h"
#include "Ui/Container.h"
#include "Ui/Rect.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GridLayout", GridLayout, Layout)

GridLayout::GridLayout(int32_t columns, int32_t rows)
:	m_columns(columns)
,	m_rows(rows)
{
}

bool GridLayout::fit(Widget* widget, const Size& bounds, Size& result)
{
	result = bounds;
	return true;
}

void GridLayout::update(Widget* widget)
{
	AlignedVector< WidgetRect > widgetRects;
	Rect inner = widget->getInnerRect();

	int32_t i = 0;
	for (Ref< Widget > child = widget->getFirstChild(); child != nullptr; child = child->getNextSibling())
	{
		if (!child->acceptLayout())
			continue;

		int32_t c = i % m_columns;
		int32_t r = i / m_columns;
		if (r >= m_rows)
			break;

		Point tl = inner.getTopLeft();
		Rect rc(
			tl.x + (c * inner.getWidth()) / m_columns,
			tl.y + (r * inner.getHeight()) / m_rows,
			tl.x + ((c + 1) * inner.getWidth()) / m_columns,
			tl.y + ((r + 1) * inner.getHeight()) / m_rows
		);
		widgetRects.push_back(WidgetRect(child, rc));

		i++;
	}

	widget->setChildRects(&widgetRects[0], (uint32_t)widgetRects.size());
}

	}
}
