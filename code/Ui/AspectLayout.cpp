#include "Ui/AspectLayout.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.AspectLayout", AspectLayout, Layout)

AspectLayout::AspectLayout(float ratio)
:	m_ratio(ratio)
{
}

bool AspectLayout::fit(Widget* widget, const Size& bounds, Size& result)
{
	Widget* child = widget->getFirstChild();
	if (!child)
		return false;

	Size sz = child->getPreferredSize(bounds);
	float ratio = m_ratio > 0.0f ? m_ratio : float(sz.cx) / sz.cy;

	int32_t width = bounds.cx;
	int32_t height = (int32_t)(width / ratio);
	if (height > bounds.cy)
	{
		height = bounds.cy;
		width = (int32_t)(height * ratio);
	}

	result.cx = width;
	result.cy = height;
	return true;
}

void AspectLayout::update(Widget* widget)
{
	Widget* child = widget->getFirstChild();
	if (child != nullptr)
	{
		Rect rc = widget->getInnerRect();

		Size sz = child->getPreferredSize(rc.getSize());
		float ratio = m_ratio > 0.0f ? m_ratio : float(sz.cx) / sz.cy;

		int32_t width = rc.getWidth();
		int32_t height = (int32_t)(width / ratio);
		if (height > rc.getHeight())
		{
			height = rc.getHeight();
			width = (int32_t)(height * ratio);
		}

		int32_t x = rc.left + (rc.getWidth() - width) / 2;
		int32_t y = rc.top + (rc.getHeight() - height) / 2;

		child->setRect(Rect(x, y, x + width, y + height));
	}
}

	}
}
