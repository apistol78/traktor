#include <algorithm>
#include "Ui/Custom/Tab/Tab.h"
#include "Ui/Custom/Tab/TabPage.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/CloseEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int c_borderSize = 8;
const int c_tabHeight = 24;
const int c_tabHeaderMargin = 8;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.Tab", Tab, Widget)

bool Tab::create(Widget* parent)
{
	if (!Widget::create(parent, ui::WsDoubleBuffer))
		return false;

	addButtonDownEventHandler(createMethodHandler(this, &Tab::eventButtonDown));
	addSizeEventHandler(createMethodHandler(this, &Tab::eventSize));
	addPaintEventHandler(createMethodHandler(this, &Tab::eventPaint));
	
	m_fontBold = getFont();
	m_fontBold.setBold(true);
	
	return true;
}

Rect Tab::getInnerRect() const
{
	return m_innerRect;
}

Size Tab::getPreferedSize() const
{
	return Widget::getPreferedSize() + Size(16, 32);
}

int Tab::addPage(TabPage* page)
{
	if (page == 0)
		return -1;

	page->setRect(m_innerRect);
	m_pages.push_back(TabPagePair(page));
	setActivePage(page);

	return int(m_pages.size() - 1);
}

int Tab::getPageCount() const
{
	return int(m_pages.size());
}

Ref< TabPage > Tab::getPage(int index)
{
	if (index < 0 || index >= int(m_pages.size()))
		return 0;
	return m_pages[index].page;
}

void Tab::removePage(TabPage* page)
{
	TabPageVector::iterator i = std::find(m_pages.begin(), m_pages.end(), TabPagePair(page));
	if (i != m_pages.end())
	{
		m_pages.erase(i);
		if (page == m_selectedPage)
		{
			page->setVisible(false);
			if (m_pages.size() > 0)
			{
				m_selectedPage = m_pages[0].page;
				m_selectedPage->setVisible(true);
			}
			else
				m_selectedPage = 0;
		}
		update();
	}
}

void Tab::removeAllPages()
{
	m_pages.clear();
	update();
}

void Tab::setActivePage(TabPage* page)
{
	if (m_selectedPage != 0)
		m_selectedPage->setVisible(false);
	if ((m_selectedPage = page) != 0)
		m_selectedPage->setVisible(true);
	update();
}

Ref< TabPage > Tab::getActivePage()
{
	return m_selectedPage;
}

void Tab::addSelChangeEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiSelectionChange, eventHandler);
}

void Tab::addCloseEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiClose, eventHandler);
}

void Tab::eventButtonDown(Event* event)
{
	MouseEvent* mouseEvent = static_cast< MouseEvent* >(event);
	Point pnt = mouseEvent->getPosition();
	
	Rect rcTab = Widget::getInnerRect().inflate(-c_borderSize, -c_borderSize);

	if (pnt.x < rcTab.left || pnt.x > rcTab.right)
		return;

	if (pnt.y < rcTab.top || pnt.y > rcTab.top + c_tabHeight)
		return;

	Ref< TabPage > selectPage;
	for (TabPageVector::iterator i = m_pages.begin(); i != m_pages.end(); ++i)
	{
		if (pnt.x <= i->right)
		{
			selectPage = i->page;
			break;
		}
	}

	if (selectPage)
	{
		if (m_selectedPage)
			m_selectedPage->setVisible(false);
		m_selectedPage = selectPage;
		m_selectedPage->setVisible(true);
		
		CommandEvent cmdEvent(this, m_selectedPage);
		raiseEvent(EiSelectionChange, &cmdEvent);
	}

	event->consume();
	update();
}

void Tab::eventSize(Event* event)
{
	m_innerRect = Widget::getInnerRect().inflate(-c_borderSize, -c_borderSize);
	
	m_innerRect.left += 2;
	m_innerRect.top += c_tabHeight + 2;
	m_innerRect.right -= 2;
	m_innerRect.bottom -= 2;

	for (Widget* child = getFirstChild(); child != 0; child = child->getNextSibling())
		child->setRect(m_innerRect);

	event->consume();
}

void Tab::eventPaint(Event* event)
{
	PaintEvent* paintEvent = checked_type_cast< PaintEvent* >(event);
	
	Canvas& canvas = paintEvent->getCanvas();
	Rect rcInner = Widget::getInnerRect();

	canvas.setBackground(Color(196, 194, 195));
	canvas.fillRect(rcInner);

	Rect rcTab = rcInner.inflate(-c_borderSize, -c_borderSize);

	int gradientCenter = rcTab.getCenter().x;
	
	canvas.setBackground(Color(224, 224, 224));
	canvas.setForeground(Color(196, 194, 195));
	{
		Rect rc(rcTab.left, rcTab.top, gradientCenter, rcTab.top + c_tabHeight);
		canvas.fillGradientRect(rc, false);
	}

	canvas.setBackground(Color(196, 194, 195));
	canvas.setForeground(Color(224, 224, 224));
	{
		Rect rc(gradientCenter, rcTab.top, rcTab.right, rcTab.top + c_tabHeight);
		canvas.fillGradientRect(rc, false);
	}

	canvas.setForeground(Color(227, 227, 227));
	canvas.drawLine(rcTab.left, rcTab.top, rcTab.right, rcTab.top);

	if (!m_pages.empty())
	{
		int left = rcTab.left + 1;
		for (TabPageVector::iterator i = m_pages.begin(); i != m_pages.end(); ++i)
		{
			Ref< TabPage >& page = i->page;

			std::wstring text = page->getText();
			Size extent = canvas.getTextExtent(text);

			int headerWidth = extent.cx + c_tabHeaderMargin * 2;

			if (page == m_selectedPage)
			{
				canvas.setBackground(Color(248, 196, 94));
				canvas.setForeground(Color(253, 229, 179));
				canvas.fillGradientRect(
					Rect(
						left + 1,
						rcTab.top + 4 + 1,
						left + headerWidth / 2,
						rcTab.top + c_tabHeight + 1
					),
					false
				);

				canvas.setBackground(Color(253, 229, 179));
				canvas.setForeground(Color(248, 196, 94));
				canvas.fillGradientRect(
					Rect(
						left + headerWidth / 2,
						rcTab.top + 4 + 1,
						left + headerWidth,
						rcTab.top + c_tabHeight + 1
					),
					false
				);

				canvas.setForeground(Color(129, 129, 129));
				canvas.drawLine(
					left,
					rcTab.top + 4 + 1,
					left,
					rcTab.top + c_tabHeight + 1
				);

				canvas.setForeground(Color(159, 159, 159));
				canvas.drawLine(
					left + 1,
					rcTab.top + 4,
					left + headerWidth,
					rcTab.top + 4
				);

				canvas.setForeground(Color(234, 234, 234));
				canvas.drawLine(
					left + headerWidth,
					rcTab.top + 4 + 1,
					left + headerWidth,
					rcTab.top + c_tabHeight + 1
				);

				canvas.setForeground(Color(165, 95, 0));
			}
			else
				canvas.setForeground(Color(51, 51, 101));

			canvas.drawText(
				Point(
					left + c_tabHeaderMargin,
					rcTab.top + (c_tabHeight - extent.cy) / 2 + 3
				),
				text
			);

			i->right = left + headerWidth;

			left += headerWidth;
		}
	}

	canvas.setForeground(Color(136, 136, 136));
	canvas.drawRect(
		Rect(
			m_innerRect.left - 1,
			m_innerRect.top - 1,
			m_innerRect.right + 1,
			m_innerRect.bottom + 1
		)
	);
	
	paintEvent->consume();
}

void Tab::drawClose(Canvas& canvas, int x, int y)
{
	canvas.setForeground(Color(128, 128, 128));
	canvas.drawLine(Point(x, y), Point(x + 7, y + 7));
	canvas.drawLine(Point(x + 1, y), Point(x + 8, y + 7));
	canvas.drawLine(Point(x + 6, y), Point(x - 1, y + 7));
	canvas.drawLine(Point(x + 7, y), Point(x, y + 7));
}

Tab::TabPagePair::TabPagePair(TabPage* page_, int right_) :
	page(page_),
	right(right_)
{
}

bool Tab::TabPagePair::operator == (const Tab::TabPagePair& rh) const
{
	return page == rh.page;
}

		}
	}
}

