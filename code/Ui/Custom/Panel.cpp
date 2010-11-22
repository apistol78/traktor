#include "Ui/Custom/Panel.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/PaintEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.Panel", Panel, Container)

bool Panel::create(Widget* parent, const std::wstring& text, Layout* layout)
{
	if (!Container::create(parent, WsNone, layout))
		return false;

	setText(text);

	addPaintEventHandler(createMethodHandler(this, &Panel::eventPaint));

	m_titleHeight = 16;

	return true;
}

Rect Panel::getInnerRect() const
{
	Rect rc = Container::getInnerRect();
	rc.left += 1;
	rc.top += m_titleHeight + 1;
	rc.right -= 1;
	rc.bottom -= 1;
	return rc;
}

void Panel::eventPaint(Event* event)
{
	PaintEvent* p = static_cast< PaintEvent* >(event);
	Canvas& canvas = p->getCanvas();

	Rect rcInner = Widget::getInnerRect();
	std::wstring text = getText();

	Size extent = canvas.getTextExtent(text);
	m_titleHeight = extent.cy + 4;

	Rect rcTitle(rcInner.left, rcInner.top, rcInner.right, rcInner.top + extent.cy + 4);

	canvas.setForeground(Color4ub(51, 94, 168));
	canvas.setBackground(Color4ub(82, 126, 192));
	canvas.fillGradientRect(rcTitle);

	canvas.setForeground(Color4ub(255, 255, 255));
	canvas.drawText(
		rcTitle.inflate(-4, 0),
		text,
		AnLeft,
		AnCenter
	);

	Point pntBorder[5] =
	{
		Point(rcInner.left, rcInner.top),
		Point(rcInner.right - 1, rcInner.top),
		Point(rcInner.right - 1, rcInner.bottom - 1),
		Point(rcInner.left, rcInner.bottom - 1),
		Point(rcInner.left, rcInner.top)
	};
	canvas.setForeground(Color4ub(128, 128, 128));
	canvas.drawLines(pntBorder, 5);

	event->consume();
}

		}
	}
}
