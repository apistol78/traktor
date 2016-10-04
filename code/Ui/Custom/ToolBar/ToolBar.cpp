#include "Drawing/Image.h"
#include "Drawing/Filters/BrightnessContrastFilter.h"
#include "Drawing/Filters/GrayscaleFilter.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarItem.h"
#include "Ui/Custom/ToolTip.h"
#include "Ui/Custom/ToolTipEvent.h"

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

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ToolBar", ToolBar, Widget)

ToolBar::ToolBar()
:	m_style(WsNone)
,	m_imageWidth(scaleBySystemDPI(16))
,	m_imageHeight(scaleBySystemDPI(16))
,	m_imageCount(0)
{
}

bool ToolBar::create(Widget* parent, int style)
{
	if (!Widget::create(parent, WsDoubleBuffer))
		return false;

	addEventHandler< MouseMoveEvent >(this, &ToolBar::eventMouseMove);
	addEventHandler< MouseButtonDownEvent >(this, &ToolBar::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &ToolBar::eventButtonUp);
	addEventHandler< PaintEvent >(this, &ToolBar::eventPaint);

	m_toolTip = new ToolTip();
	m_toolTip->create(this);
	m_toolTip->addEventHandler< ToolTipEvent >(this, &ToolBar::eventShowTip);

	m_style = style;
	return true;
}

void ToolBar::destroy()
{
	Widget::destroy();
}

uint32_t ToolBar::addImage(IBitmap* image, uint32_t imageCount)
{
	T_ASSERT (image);
	T_ASSERT (imageCount > 0);

	if (m_imageEnabled)
	{
		uint32_t width = m_imageEnabled->getSize().cx + image->getSize().cx;
		uint32_t height = std::max(m_imageEnabled->getSize().cy, image->getSize().cy);

		Ref< ui::Bitmap > newImage = new ui::Bitmap(width, height);
		newImage->copyImage(m_imageEnabled->getImage());
		newImage->copySubImage(image->getImage(), Rect(Point(0, 0), image->getSize()), Point(m_imageEnabled->getSize().cx, 0));
		m_imageEnabled = newImage;
	}
	else
	{
		m_imageEnabled = image;
		m_imageWidth = std::max< uint32_t >(m_imageWidth, m_imageEnabled->getSize().cx / imageCount);
		m_imageHeight = std::max< uint32_t >(m_imageHeight, m_imageEnabled->getSize().cy);
	}

	uint32_t imageBase = m_imageCount;
	m_imageCount += imageCount;

	// Create disabled image
	{
		Ref< drawing::Image > image = m_imageEnabled->getImage()->clone();

		drawing::GrayscaleFilter grayscaleFilter;
		image->apply(&grayscaleFilter);

		drawing::BrightnessContrastFilter brigtnessContrastFilter(0.4f, 0.6f);
		image->apply(&brigtnessContrastFilter);

		m_imageDisabled = new ui::Bitmap(image);
	}

	return imageBase;
}

uint32_t ToolBar::addItem(ToolBarItem* item)
{
	m_items.push_back(item);
	return uint32_t(m_items.size() - 1);
}

void ToolBar::setItem(uint32_t id, ToolBarItem* item)
{
	T_ASSERT (id < m_items.size());
	m_items[id] = item;
}

Ref< ToolBarItem > ToolBar::getItem(uint32_t id)
{
	T_ASSERT (id < m_items.size());
	return m_items[id];
}

Ref< ToolBarItem > ToolBar::getItem(const Point& at)
{
	Rect rc = getInnerRect();

	int x = scaleBySystemDPI(c_marginWidth);
	int y = scaleBySystemDPI(c_marginHeight);

	for (RefArray< ToolBarItem >::iterator i = m_items.begin(); i != m_items.end(); ++i)
	{
		Size size = (*i)->getSize(this, m_imageWidth, m_imageHeight);

		// Calculate item rectangle.
		int offset = (rc.getHeight() - scaleBySystemDPI(c_marginHeight * 2) - size.cy) / 2;
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

Size ToolBar::getPreferedSize() const
{
	int width = getParent()->getInnerRect().getWidth();
	int height = 0;

	for (RefArray< ToolBarItem >::const_iterator i = m_items.begin(); i != m_items.end(); ++i)
	{
		Size size = (*i)->getSize(this, m_imageWidth, m_imageHeight);
		height = std::max(height, size.cy);
	}

	return Size(width, height + scaleBySystemDPI(c_marginHeight * 2 + 1));
}

void ToolBar::eventMouseMove(MouseMoveEvent* event)
{
	if (!isEnable())
		return;

	ToolBarItem* item = getItem(event->getPosition());
	if (item != m_trackItem)
	{
		if (m_trackItem)
		{
			m_trackItem->mouseLeave(this, event);
			m_trackItem = 0;
		}

		m_trackItem = item;

		if (item && item->mouseEnter(this, event))
		{
			setCapture();

			// Update tooltip if it's visible.
			if (m_toolTip->isVisible(false))
			{
				std::wstring toolTip;
				if (item->getToolTip(toolTip))
					m_toolTip->show(event->getPosition(), toolTip);
				else
					m_toolTip->hide();
			}

			m_trackItem = item;
		}
		else
		{
			m_toolTip->hide();
			releaseCapture();
		}

		update();
	}
}

void ToolBar::eventButtonDown(MouseButtonDownEvent* event)
{
	if (!isEnable())
		return;

	ToolBarItem* item = getItem(event->getPosition());
	if (item && item->isEnable())
	{
		item->buttonDown(this, event);
		update();
	}
}

void ToolBar::eventButtonUp(MouseButtonUpEvent* event)
{
	if (!isEnable())
		return;

	ToolBarItem* item = getItem(event->getPosition());
	if (item && item->isEnable())
	{
		item->buttonUp(this, event);
		update();
	}
}

void ToolBar::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	Rect rc = getInnerRect();

	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(Rect(rc.left, rc.top, rc.right, rc.bottom));

	/*
	if (m_style & WsUnderline)
	{
		canvas.setForeground(getSystemColor(ScButtonShadow));
		canvas.drawLine(rc.left, rc.bottom - 1, rc.right, rc.bottom - 1);
	}
	*/

	int x = rc.left + scaleBySystemDPI(c_marginWidth);
	int y = rc.top + scaleBySystemDPI(c_marginHeight);

	for (RefArray< ToolBarItem >::iterator i = m_items.begin(); i != m_items.end(); ++i)
	{
		Size size = (*i)->getSize(this, m_imageWidth, m_imageHeight);

		// Calculate top-left position of item, center vertically.
		int offset = (rc.getHeight() - scaleBySystemDPI(c_marginHeight) * 2 - size.cy) / 2;
		Point at(x, y + offset);

		(*i)->paint(
			this,
			canvas,
			at,
			(isEnable() && (*i)->isEnable()) ? m_imageEnabled : m_imageDisabled,
			m_imageWidth,
			m_imageHeight
		);

		x += size.cx + c_itemPad;
	}

	event->consume();
}

void ToolBar::eventShowTip(ToolTipEvent* event)
{
	Ref< ToolBarItem > item = getItem(event->getPosition());
	if (item)
	{
		std::wstring toolTip;
		if (item->getToolTip(toolTip))
			m_toolTip->show(
				event->getPosition(),
				toolTip
			);
	}
}

		}
	}
}
