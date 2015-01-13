#include "Ui/Custom/StatusBar/StatusBar.h"
#include "Ui/Application.h"
#include "Ui/Form.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int32_t c_preferedHeight = 18;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.StatusBar", StatusBar, Widget)

StatusBar::StatusBar()
{
}

bool StatusBar::create(Widget* parent, int style)
{
	if (!Widget::create(parent, style))
		return false;

	addEventHandler< SizeEvent >(this, &StatusBar::eventSize);
	addEventHandler< PaintEvent >(this, &StatusBar::eventPaint);

	return true;
}

void StatusBar::setText(const std::wstring& text)
{
	if (text != getText())
	{
		Widget::setText(text);
		update();
	}
}

Size StatusBar::getPreferedSize() const
{
	Size preferedSize(0, c_preferedHeight);
	if (getParent())
		preferedSize.cx = getParent()->getInnerRect().getWidth();
	return preferedSize;
}

void StatusBar::eventSize(SizeEvent* event)
{
	Ref< Widget > child = getFirstChild();
	if (child)
	{
		Rect rc = getInnerRect();
		Size preferedSize = child->getPreferedSize();

		rc.top += 2;
		rc.bottom -= 2;
		rc.right -= 20;
		rc.left = rc.right - preferedSize.cx;

		child->setRect(rc);
	}
}

void StatusBar::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();

	Rect rc = getInnerRect();

	canvas.setBackground(getSystemColor(ScButtonFace));
	canvas.fillRect(rc);

	std::wstring text = getText();
	canvas.setForeground(getSystemColor(ScMenuText));
	canvas.drawText(rc.inflate(-8, 0), text, AnLeft, AnCenter);

	// Draw scale grip if parent is a form and isn't maximized.
	Ref< Form > parentForm = dynamic_type_cast< Form* >(getParent());
	if (parentForm && !parentForm->isMaximized())
	{
		rc = rc.inflate(-2, -2);
		for (int i = 0; i <= 2; ++i)
		{
			canvas.setForeground(getSystemColor(ScButtonShadow));
			canvas.drawLine(rc.right, rc.bottom - i * 4 - 1, rc.right - i * 4 - 1, rc.bottom);
			canvas.drawLine(rc.right, rc.bottom - i * 4 - 2, rc.right - i * 4 - 2, rc.bottom);

			canvas.setForeground(Color4ub(255, 255, 255));
			canvas.drawLine(rc.right, rc.bottom - i * 4 - 3, rc.right - i * 4 - 3, rc.bottom);
		}
	}

	event->consume();
}

		}
	}
}
