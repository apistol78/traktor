#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/MiniButton.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.MiniButton", MiniButton, Widget)

bool MiniButton::create(Widget* parent, const std::wstring& text)
{
	if (!Widget::create(parent))
		return false;
	
	m_state = StReleased;
	
	setText(text);
	
	addEventHandler< MouseButtonDownEvent >(this, &MiniButton::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &MiniButton::eventButtonUp);
	addEventHandler< PaintEvent >(this, &MiniButton::eventPaint);
	
	return true;
}

bool MiniButton::create(Widget* parent, IBitmap* image)
{
	if (!Widget::create(parent))
		return false;
	
	m_state = StReleased;
	m_image  = image;
	
	addEventHandler< MouseButtonDownEvent >(this, &MiniButton::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &MiniButton::eventButtonUp);
	addEventHandler< PaintEvent >(this, &MiniButton::eventPaint);
	
	return true;
}

void MiniButton::setImage(IBitmap* image)
{
	m_image = image;
}

Size MiniButton::getPreferedSize() const
{
	if (m_image)
		return m_image->getSize() + Size(ui::scaleBySystemDPI(6), ui::scaleBySystemDPI(6));
	else
		return Size(ui::scaleBySystemDPI(16), ui::scaleBySystemDPI(16));
}

void MiniButton::eventButtonDown(MouseButtonDownEvent* event)
{
	m_state = StPushed;
	update();

	setCapture();
	event->consume();
}

void MiniButton::eventButtonUp(MouseButtonUpEvent* event)
{
	releaseCapture();

	if (m_state == StPushed)
	{
		m_state = StReleased;
		update();
	
		ButtonClickEvent clickEvent(this);
		raiseEvent(&clickEvent);
	}
	
	event->consume();
}

void MiniButton::eventPaint(PaintEvent* event)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	Canvas& canvas = event->getCanvas();
	
	Rect rcInner = getInnerRect();
	
	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	if (isEnable())
	{
		if (m_state == StReleased)
		{
			canvas.setForeground(ss->getColor(this, L"bevel-color"));
			canvas.drawRect(rcInner);
			
			rcInner = rcInner.inflate(-1, -1);

			canvas.setForeground(ss->getColor(this, L"bevel-highlight-color"));
			canvas.drawLine(rcInner.left, rcInner.bottom - 2, rcInner.left, rcInner.top);
			canvas.drawLine(rcInner.left, rcInner.top, rcInner.right - 1, rcInner.top);
			
			canvas.setForeground(ss->getColor(this, L"bevel-shadow-color"));
			canvas.drawLine(rcInner.left + 1, rcInner.bottom - 1, rcInner.right - 1, rcInner.bottom - 1);
			canvas.drawLine(rcInner.right - 1, rcInner.bottom - 1, rcInner.right - 1, rcInner.top);
		}
		else
		{
			canvas.setForeground(ss->getColor(this, L"bevel-color"));
			canvas.drawRect(rcInner);
		
			if (m_state == StPushed)
				rcInner = rcInner.offset(1, 1);
		}
	}
	else
	{
		canvas.setForeground(ss->getColor(this, L"bevel-disabled-color"));
		canvas.drawRect(rcInner);
	}

	if (m_image)
	{
		Size size = m_image->getSize();

		Size margin = rcInner.getSize() - size;
		Point at(rcInner.left + margin.cx / 2, rcInner.top + margin.cy / 2);

		canvas.drawBitmap(
			at,
			Point(0, 0),
			size,
			m_image,
			BmAlpha
		);
	}
	else
	{
		canvas.setForeground(ss->getColor(this, L"color"));
		canvas.drawText(rcInner, getText(), AnCenter, AnCenter);
	}
	
	event->consume();
}

		}
	}
}
