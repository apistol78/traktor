#include "Ui/Application.h"
#include "Ui/Bitmap.h"
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

bool MiniButton::create(Widget* parent, Bitmap* image)
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

void MiniButton::setImage(Bitmap* image)
{
	m_image = image;
}

Size MiniButton::getPreferedSize() const
{
	if (m_image)
		return m_image->getSize() + Size(6, 6);
	else
		return Size(16, 16);
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
	Canvas& canvas = event->getCanvas();
	
	Rect rcInner = getInnerRect();
	
	canvas.setBackground(getSystemColor(ScButtonFace));
	canvas.fillRect(rcInner);

	if (isEnable())
	{
		if (m_state == StReleased)
		{
			canvas.setForeground(Color4ub(140, 140, 140));
			canvas.drawRect(rcInner);
			
			rcInner = rcInner.inflate(-1, -1);

			canvas.setForeground(Color4ub(255, 255, 255));
			canvas.drawLine(rcInner.left, rcInner.bottom - 2, rcInner.left, rcInner.top);
			canvas.drawLine(rcInner.left, rcInner.top, rcInner.right - 1, rcInner.top);
			
			canvas.setForeground(Color4ub(64, 64, 64));
			canvas.drawLine(rcInner.left + 1, rcInner.bottom - 1, rcInner.right - 1, rcInner.bottom - 1);
			canvas.drawLine(rcInner.right - 1, rcInner.bottom - 1, rcInner.right - 1, rcInner.top);
		}
		else
		{
			canvas.setForeground(getSystemColor(ScButtonShadow));
			canvas.drawRect(rcInner);
		
			if (m_state == StPushed)
				rcInner = rcInner.offset(1, 1);
		}
	}
	else
	{
		canvas.setForeground(Color4ub(140, 140, 140));
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
		canvas.setForeground(Color4ub(0, 0, 0));
		canvas.drawText(rcInner, getText(), AnCenter, AnCenter);
	}
	
	event->consume();
}

		}
	}
}
