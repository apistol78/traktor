#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/GradientStatic/GradientStatic.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GradientStatic", GradientStatic, Widget)

bool GradientStatic::create(Widget* parent, const std::wstring& text, int style)
{
	if (!Widget::create(parent, style))
		return false;

	setText(text);

	addEventHandler< PaintEvent >(this, &GradientStatic::eventPaint);
	return true;
}

Size GradientStatic::getPreferedSize() const
{
	return getTextExtent(getText());
}

void GradientStatic::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	Rect innerRect = getInnerRect();

	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	canvas.setForeground(ss->getColor(this, L"background-color-left"));
	canvas.setBackground(ss->getColor(this, L"background-color-right"));
	canvas.fillGradientRect(innerRect, true);

	canvas.setForeground(ss->getColor(this, L"color"));
	canvas.drawText(innerRect.inflate(-4, -4), getText(), AnLeft, AnTop);

	event->consume();
}

		}
	}
}
