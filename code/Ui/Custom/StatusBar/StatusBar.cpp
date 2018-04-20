/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/StatusBar/StatusBar.h"
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/Form.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int32_t c_preferedHeight = 23;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.StatusBar", StatusBar, Widget)

StatusBar::StatusBar()
:	m_alert(false)
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

void StatusBar::setAlert(bool alert)
{
	if (alert != m_alert)
	{
		m_alert = alert;
		update();
	}
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
	Size preferedSize(0, dpi96(c_preferedHeight));
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

		rc.top += dpi96(2);
		rc.bottom -= dpi96(2);
		rc.right -= dpi96(20);
		rc.left = rc.right - preferedSize.cx;

		child->setRect(rc);
	}
}

void StatusBar::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	Rect rc = getInnerRect();

	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	canvas.setBackground(ss->getColor(this, m_alert ? L"background-color-alert" : L"background-color"));
	canvas.fillRect(rc);

	std::wstring text = getText();
	canvas.setForeground(ss->getColor(this, L"color"));
	canvas.drawText(rc.inflate(dpi96(-8), 0), text, AnLeft, AnCenter);

	// Draw scale grip if parent is a form and isn't maximized.
	Ref< Form > parentForm = dynamic_type_cast< Form* >(getParent());
	if (parentForm && !parentForm->isMaximized())
	{
		rc = rc.inflate(-dpi96(2), -dpi96(2));
		for (int i = 0; i <= 2; ++i)
		{
			canvas.setForeground(getSystemColor(ScButtonShadow));
			canvas.drawLine(rc.right, rc.bottom - dpi96(i * 4 - 1), rc.right - dpi96(i * 4 - 1), rc.bottom);
			canvas.drawLine(rc.right, rc.bottom - dpi96(i * 4 - 2), rc.right - dpi96(i * 4 - 2), rc.bottom);

			canvas.setForeground(Color4ub(255, 255, 255));
			canvas.drawLine(rc.right, rc.bottom - dpi96(i * 4 - 3), rc.right - dpi96(i * 4 - 3), rc.bottom);
		}
	}

	event->consume();
}

		}
	}
}
