#include "Ui/Custom/AspectLayout.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.AspectLayout", AspectLayout, Layout)

AspectLayout::AspectLayout(float ratio) :
	m_ratio(ratio)
{
}

void AspectLayout::setRatio(float ratio)
{
	m_ratio = ratio;
}

bool AspectLayout::fit(Widget* widget, const Size& bounds, Size& result)
{
	Widget* child = widget->getFirstChild();
	if (!child)
		return false;

	int width = bounds.cx;
	int height = int(width / m_ratio);
	if (height > bounds.cy)
	{
		height = bounds.cy;
		width = int(height * m_ratio);
	}

	result.cx = width;
	result.cy = height;
	return true;
}

void AspectLayout::update(Widget* widget)
{
	Widget* child = widget->getFirstChild();
	if (child != 0)
	{
		Rect rc = widget->getInnerRect();

		int width = rc.getWidth();
		int height = int(width / m_ratio);
		if (height > rc.getHeight())
		{
			height = rc.getHeight();
			width = int(height * m_ratio);
		}

		int x = rc.left + (rc.getWidth() - width) / 2;
		int y = rc.top + (rc.getHeight() - height) / 2;

		child->setRect(Rect(x, y, x + width, y + height));
	}
}

		}
	}
}
