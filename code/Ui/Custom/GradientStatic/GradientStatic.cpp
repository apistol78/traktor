#include "Ui/Custom/GradientStatic/GradientStatic.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.GradientStatic", GradientStatic, Widget)

bool GradientStatic::create(Widget* parent, const Color4ub& colorLeft, const Color4ub& colorRight, const Color4ub& colorText, const std::wstring& text, int style)
{
	if (!Widget::create(parent, style))
		return false;

	m_colorLeft = colorLeft;
	m_colorRight = colorRight;
	m_colorText = colorText;

	setText(text);

	addEventHandler< PaintEvent >(this, &GradientStatic::eventPaint);

	return true;
}

Size GradientStatic::getPreferedSize() const
{
	return getTextExtent(getText());
}

void GradientStatic::setColorLeft(const Color4ub& colorLeft)
{
	m_colorLeft = colorLeft;
}

void GradientStatic::setColorRight(const Color4ub& colorRight)
{
	m_colorRight = colorRight;
}

void GradientStatic::setColorText(const Color4ub& colorText)
{
	m_colorText = colorText;
}

void GradientStatic::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();

	Rect innerRect = getInnerRect();

	canvas.setForeground(m_colorLeft);
	canvas.setBackground(m_colorRight);
	canvas.fillGradientRect(innerRect, true);

	canvas.setForeground(m_colorText);
	canvas.drawText(innerRect.inflate(-4, -4), getText(), AnLeft, AnTop);

	event->consume();
}

		}
	}
}
