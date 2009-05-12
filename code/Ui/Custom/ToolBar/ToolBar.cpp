#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarItem.h"
#include "Ui/Custom/ToolTip.h"
#include "Ui/Custom/ToolTipEvent.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Events/CommandEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int c_marginWidth = 1;
const int c_marginHeight = 1;
const int c_itemPad = 2;
const int c_buttonAddWidth = 8;
const int c_buttonAddHeight = 8;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ToolBar", ToolBar, Widget)

ToolBar::ToolBar()
:	m_style(WsNone)
,	m_imageWidth(16)
,	m_imageHeight(16)
,	m_imageCount(0)
{
}

bool ToolBar::create(Widget* parent, int style)
{
	if (!Widget::create(parent, WsDoubleBuffer))
		return false;

	addMouseMoveEventHandler(createMethodHandler(this, &ToolBar::eventMouseMove));
	addButtonDownEventHandler(createMethodHandler(this, &ToolBar::eventButtonDown));
	addButtonUpEventHandler(createMethodHandler(this, &ToolBar::eventButtonUp));
	addPaintEventHandler(createMethodHandler(this, &ToolBar::eventPaint));

	m_toolTip = gc_new< ToolTip >();
	m_toolTip->create(this);
	m_toolTip->addShowEventHandler(createMethodHandler(this, &ToolBar::eventShowTip));

	m_style = style;
	return true;
}

void ToolBar::destroy()
{
	Widget::destroy();
}

uint32_t ToolBar::addImage(Bitmap* image, uint32_t imageCount)
{
	uint32_t width = 0, height = 0;

	if (!image)
		return 0;
	
	if (m_image)
	{
		Ref< Bitmap > source = image;

		width = m_image->getSize().cx + source->getSize().cx;
		height = std::max(m_image->getSize().cy, source->getSize().cy);

		Ref< ui::Bitmap > newImage = gc_new< ui::Bitmap >(width, height);
		newImage->copyImage(m_image->getImage());
		newImage->copySubImage(image->getImage(), Rect(Point(0, 0), source->getSize()), Point(m_image->getSize().cx, 0));
		m_image = newImage;
	}
	else
	{
		m_image = image;
		m_imageWidth = std::max< uint32_t >(m_imageWidth, m_image->getSize().cx / imageCount);
		m_imageHeight = std::max< uint32_t >(m_imageHeight, m_image->getSize().cy);
	}

	uint32_t imageBase = m_imageCount;
	m_imageCount += imageCount;

	return imageBase;
}

uint32_t ToolBar::addItem(ToolBarItem* item)
{
	m_items.push_back(item);
	return uint32_t(m_items.size() - 1);
}

ToolBarItem* ToolBar::getItem(uint32_t id)
{
	T_ASSERT (id < m_items.size());
	return m_items[id];
}

ToolBarItem* ToolBar::getItem(const Point& at)
{
	Rect rc = getInnerRect();

	int x = c_marginWidth;
	int y = c_marginHeight;

	for (RefArray< ToolBarItem >::iterator i = m_items.begin(); i != m_items.end(); ++i)
	{
		Size size = (*i)->getSize(this, m_imageWidth, m_imageHeight);

		// Calculate item rectangle.
		int offset = (rc.getHeight() - c_marginHeight * 2 - size.cy) / 2;
		Rect rc(
			Point(x, y + offset),
			size
		);

		if (rc.inside(at))
			return *i;

		x += size.cx + c_itemPad;
	}

	return 0;
}

void ToolBar::addClickEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiClick, eventHandler);
}

Size ToolBar::getPreferedSize() const
{
	int width = getParent()->getInnerRect().getWidth();
	int height = 0;

	for (RefArray< ToolBarItem >::const_iterator i = m_items.begin(); i != m_items.end(); ++i)
	{
		Size size = (*i)->getSize(this, m_imageWidth, m_imageHeight);
		height = std::max(height, size.cy);
	}

	return Size(width, height + c_marginHeight * 2 + 1);
}

void ToolBar::eventMouseMove(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);
	ToolBarItem* item = getItem(mouseEvent->getPosition());
	if (item != m_trackItem)
	{
		if (m_trackItem)
			m_trackItem->mouseLeave(this, mouseEvent);

		m_trackItem = item;

		if (m_trackItem)
		{
			m_trackItem->mouseEnter(this, mouseEvent);
			setCapture();

			// Update tooltip if it's visible.
			if (m_toolTip->isVisible(false))
			{
				std::wstring toolTip;
				if (m_trackItem->getToolTip(toolTip))
					m_toolTip->show(mouseEvent->getPosition(), toolTip);
				else
					m_toolTip->hide();
			}
		}
		else
		{
			m_toolTip->hide();
			releaseCapture();
		}

		update();
	}
}

void ToolBar::eventButtonDown(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);
	ToolBarItem* item = getItem(mouseEvent->getPosition());
	if (item)
	{
		item->buttonDown(this, mouseEvent);
		update();
	}
}

void ToolBar::eventButtonUp(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);
	ToolBarItem* item = getItem(mouseEvent->getPosition());
	if (item)
	{
		item->buttonUp(this, mouseEvent);
		update();
	}
}

void ToolBar::eventPaint(Event* event)
{
	PaintEvent* paintEvent = checked_type_cast< PaintEvent* >(event);
	Canvas& canvas = paintEvent->getCanvas();

	Rect rc = getInnerRect();
	int mid = rc.getCenter().y;

	canvas.setForeground(Color(245, 244, 242));
	canvas.setBackground(Color(234, 232, 228));
	canvas.fillGradientRect(Rect(rc.left, rc.top, rc.right, mid + 1));

	canvas.setForeground(Color(234, 232, 228));
	canvas.setBackground(Color(213, 210, 202));
	canvas.fillGradientRect(Rect(rc.left, mid - 1, rc.right, rc.bottom));

	canvas.setForeground(getSystemColor(ScButtonShadow));
	canvas.drawLine(rc.left, rc.bottom - 1, rc.right, rc.bottom - 1);

	int x = rc.left + c_marginWidth;
	int y = rc.top + c_marginHeight;

	for (RefArray< ToolBarItem >::iterator i = m_items.begin(); i != m_items.end(); ++i)
	{
		Size size = (*i)->getSize(this, m_imageWidth, m_imageHeight);

		// Calculate top-left position of item, center vertically.
		int offset = (rc.getHeight() - c_marginHeight * 2 - size.cy) / 2;
		Point at(x, y + offset);

		(*i)->paint(this, canvas, at, m_image, m_imageWidth, m_imageHeight);

		x += size.cx + c_itemPad;
	}

	paintEvent->consume();
}

void ToolBar::eventShowTip(Event* event)
{
	ToolTipEvent* tipEvent = checked_type_cast< ToolTipEvent* >(event);
	Ref< ToolBarItem > item = getItem(tipEvent->getPosition());
	if (item)
	{
		std::wstring toolTip;
		if (item->getToolTip(toolTip))
			m_toolTip->show(
				tipEvent->getPosition(),
				toolTip
			);
	}
}

		}
	}
}
