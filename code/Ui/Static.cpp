#include "Core/Misc/StringSplit.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Static.h"
#include "Ui/StyleSheet.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Static", Static, Widget)

bool Static::create(Widget* parent, const std::wstring& text)
{
	if (!Widget::create(parent))
		return false;

	setText(text);
	addEventHandler< PaintEvent >(this, &Static::eventPaint);
	return true;
}

Size Static::getPreferedSize() const
{
	Size extent(0, 0);

	auto fontMetric = getFontMetric();
	for (auto s : StringSplit< std::wstring >(getText(), L"\n\r"))
	{
		auto sz = fontMetric.getExtent(s);
		extent.cx = std::max(sz.cx, extent.cx);
		extent.cy += sz.cy;
	}

	return extent + Size(dpi96(1), dpi96(1));
}

Size Static::getMaximumSize() const
{
	return getPreferedSize();
}

void Static::eventPaint(PaintEvent* event)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	Canvas& canvas = event->getCanvas();

	Rect rcInner = getInnerRect();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	canvas.setForeground(ss->getColor(this, L"color"));

	Point pt(0, 0);

	auto fontMetric = getFontMetric();
	for (auto s : StringSplit< std::wstring >(getText(), L"\n\r"))
	{
		auto sz = fontMetric.getExtent(s);
		canvas.drawText(pt, s);
		pt.y += sz.cy;
	}

	event->consume();
}

	}
}
