#include <algorithm>
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Tab.h"
#include "Ui/TabPage.h"
#include "Ui/Font.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const int32_t c_tabHeight = 21;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Tab", Tab, Widget)

Tab::Tab()
:	m_imageWidth(0)
,	m_imageHeight(0)
,	m_closeButton(false)
,	m_drawBorder(false)
,	m_drawLine(false)
,	m_bottom(false)
,	m_closeHighlight(false)
{
}

bool Tab::create(Widget* parent, int32_t style)
{
	if (!Widget::create(parent, style & ~WsBorder))
		return false;

	addEventHandler< MouseMoveEvent >(this, &Tab::eventMouseMove);
	addEventHandler< MouseButtonDownEvent >(this, &Tab::eventButtonDown);
	addEventHandler< SizeEvent >(this, &Tab::eventSize);
	addEventHandler< PaintEvent >(this, &Tab::eventPaint);
	
	m_fontBold = getFont();
	m_fontBold.setBold(true);

	m_closeButton = bool((style & WsCloseButton) == WsCloseButton);
	m_drawBorder = bool((style & WsBorder) == WsBorder);
	m_drawLine = bool((style & WsLine) == WsLine);
	m_bottom = bool((style & WsBottom) == WsBottom);
	
	m_closeHighlight = false;
	return true;
}

Rect Tab::getInnerRect() const
{
	return m_innerRect;
}

int32_t Tab::addImage(Bitmap* image, int32_t imageCount)
{
	uint32_t width = 0, height = 0;
	
	// Resize existing image.
	if (m_image)
	{
		width = m_image->getSize().cx + image->getSize().cx;
		height = std::max(m_image->getSize().cy, image->getSize().cy);

		Ref< ui::Bitmap > newImage = new ui::Bitmap(width, height);
		newImage->copyImage(m_image->getImage());
		newImage->copySubImage(image->getImage(), Rect(Point(0, 0), image->getSize()), Point(m_image->getSize().cx, 0));
		m_image = newImage;
	}
	else
	{
		m_image = image;
		m_imageWidth = std::max< uint32_t >(m_imageWidth, m_image->getSize().cx / imageCount);
		m_imageHeight = std::max< uint32_t >(m_imageHeight, m_image->getSize().cy);
	}

	return 0;
}

int32_t Tab::addPage(TabPage* page)
{
	if (page == 0)
		return -1;

	for (page_state_vector_t::iterator i = m_pages.begin(); i != m_pages.end(); ++i)
		i->depth++;

	m_pages.push_back(PageState(page));

	if (m_selectedPage)
		m_selectedPage->setVisible(false);

	page->setRect(m_innerRect);
	page->setVisible(true);

	m_selectedPage = page;

#if defined(_DEBUG)
	checkPageStates();
#endif
	return int32_t(m_pages.size() - 1);
}

int32_t Tab::getPageCount() const
{
	return int32_t(m_pages.size());
}

Ref< TabPage > Tab::getPage(int32_t index) const
{
	if (index < 0 || index >= int32_t(m_pages.size()))
		return 0;
	return m_pages[index].page;
}

Ref< TabPage > Tab::getPageAt(const Point& position) const
{
	if (position.y >= c_tabHeight)
		return 0;

	for (page_state_vector_t::const_iterator i = m_pages.begin(); i != m_pages.end(); ++i)
	{
		if (position.x <= i->right)
			return i->page;
	}

	return 0;
}

void Tab::removePage(TabPage* page)
{
	page_state_vector_t::iterator i = std::find(m_pages.begin(), m_pages.end(), PageState(page));
	if (i != m_pages.end())
	{
		m_pages.erase(i);
		if (page == m_selectedPage)
		{
			page->setVisible(false);

			// Decrement all depths, to one found with zero depth is the new active page.
			for (page_state_vector_t::iterator j = m_pages.begin(); j != m_pages.end(); ++j)
				j->depth--;

#if defined(_DEBUG)
			checkPageStates();
#endif

			PageState* newPageState = findPageState(0);
			if (newPageState)
			{
				m_selectedPage = newPageState->page;
				m_selectedPage->setVisible(true);
			}
			else
				m_selectedPage = 0;

#if defined(_DEBUG)
			checkPageStates();
#endif
		}
	}
}

void Tab::removeAllPages()
{
	m_pages.clear();
}

void Tab::setActivePage(TabPage* page)
{
	if (page == m_selectedPage)
		return;

	if (m_selectedPage)
	{
		PageState* state = findPageState(m_selectedPage);
		T_ASSERT (state);

		m_selectedPage->setVisible(false);
	}

#if defined(_DEBUG)
	checkPageStates();
#endif

	if ((m_selectedPage = page) != 0)
	{
		PageState* state = findPageState(page);
		T_ASSERT (state);

#if defined(_DEBUG)
		checkPageStates();
#endif

		int32_t depth = state->depth;
		for (page_state_vector_t::iterator i = m_pages.begin(); i != m_pages.end(); ++i)
		{
			if (i->depth <= depth)
				i->depth++;
		}

		state->depth = 0;

#if defined(_DEBUG)
		checkPageStates();
#endif

		m_selectedPage->setVisible(true);
	}
}

Ref< TabPage > Tab::getActivePage()
{
	return m_selectedPage;
}

Ref< TabPage > Tab::cycleActivePage(bool forward)
{
	if (m_pages.size() < 2)
		return m_selectedPage;

#if defined(_DEBUG)
	checkPageStates();
#endif

	int32_t maxDepth = int32_t(m_pages.size() - 1);

	if (forward)
	{
		for (page_state_vector_t::iterator i = m_pages.begin(); i != m_pages.end(); ++i)
		{
			if (--i->depth < 0)
				i->depth = maxDepth;
		}
	}
	else
	{
		for (page_state_vector_t::iterator i = m_pages.begin(); i != m_pages.end(); ++i)
		{
			if (++i->depth > maxDepth)
				i->depth = 0;
		}
	}

#if defined(_DEBUG)
	checkPageStates();
#endif

	if (m_selectedPage)
		m_selectedPage->setVisible(false);

	PageState* pageState = findPageState(0);
	T_ASSERT (pageState);

#if defined(_DEBUG)
	checkPageStates();
#endif

	m_selectedPage = pageState->page;
	m_selectedPage->setVisible(true);

	update();

	return m_selectedPage;
}

Size Tab::getMinimumSize() const
{
	return Size(256, 256);
}

Size Tab::getPreferedSize() const
{
	return Size(256, 256);
}

void Tab::eventMouseMove(MouseMoveEvent* event)
{
	Point pnt = event->getPosition();
	Rect inner = Widget::getInnerRect();

	int32_t y0, y1;
	if (!m_bottom)
	{
		y0 = inner.top;
		y1 = inner.top + c_tabHeight;
	}
	else
	{
		y0 = inner.bottom - c_tabHeight;
		y1 = inner.bottom;
	}

	if (inner.inside(pnt) && pnt.y >= y0 && pnt.y <= y1)
	{
		if (m_closeButton && m_selectedPage && pnt.x > inner.right - 20)
		{
			if (!m_closeHighlight)
			{
				m_closeHighlight = true;
				update();
			}
			setCapture();
			return;
		}
	}

	if (m_closeHighlight)
	{
		m_closeHighlight = false;
		Rect rc(inner.right, inner.top, inner.right, inner.bottom);
		update();
		releaseCapture();
	}
}

void Tab::eventButtonDown(MouseButtonDownEvent* event)
{
	Point pnt = event->getPosition();
	Rect inner = Widget::getInnerRect();
	
	int32_t y0, y1;
	if (!m_bottom)
	{
		y0 = inner.top;
		y1 = inner.top + c_tabHeight;
	}
	else
	{
		y0 = inner.bottom - c_tabHeight;
		y1 = inner.bottom;
	}

	if (pnt.y >= y0 && pnt.y <= y1)
	{
		if (m_closeButton && m_selectedPage && pnt.x > inner.right - 16)
		{
			TabCloseEvent closeEvent(this, m_selectedPage);
			raiseEvent(&closeEvent);
		}
		else
		{
			Ref< TabPage > selectPage;
			for (page_state_vector_t::iterator i = m_pages.begin(); i != m_pages.end(); ++i)
			{
				if (pnt.x <= i->right)
				{
					selectPage = i->page;
					break;
				}
			}
			if (selectPage && selectPage != m_selectedPage)
			{
#if defined(_DEBUG)
				checkPageStates();
#endif

				if (m_selectedPage)
				{
					PageState* state = findPageState(m_selectedPage);
					T_ASSERT (state);
					T_ASSERT (state->depth == 0);

					m_selectedPage->setVisible(false);
				}

				TabSelectionChangeEvent selectionChangeEvent(this, selectPage);
				raiseEvent(&selectionChangeEvent);

				if ((m_selectedPage = selectPage) != 0)
				{
					PageState* state = findPageState(selectPage);
					T_ASSERT (state);

					for (page_state_vector_t::iterator i = m_pages.begin(); i != m_pages.end(); ++i)
					{
						if (i->depth < state->depth)
							i->depth++;
					}

					state->depth = 0;

					m_selectedPage->setVisible(true);
				}

#if defined(_DEBUG)
				checkPageStates();
#endif
			}
		}

		setFocus();
		update();
	}
}

void Tab::eventSize(SizeEvent* event)
{
	m_innerRect = Widget::getInnerRect();
	if (!m_bottom)
		m_innerRect.top += c_tabHeight;
	else
		m_innerRect.bottom -= c_tabHeight;

	if (m_drawBorder)
		m_innerRect = m_innerRect.inflate(-1, -1);

	for (Widget* child = getFirstChild(); child != 0; child = child->getNextSibling())
		child->setRect(m_innerRect);

	update();

	event->consume();
}

void Tab::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	Rect rcPaint = event->getUpdateRect();
	Rect rcInner = Widget::getInnerRect();
	
	int32_t y0, y1;
	if (!m_bottom)
	{
		y0 = rcInner.top;
		y1 = rcInner.top + c_tabHeight;
	}
	else
	{
		y0 = rcInner.bottom - c_tabHeight;
		y1 = rcInner.bottom;
	}

	// Fill tab background.
	Rect rcTabs(rcInner.left, y0, rcInner.right, y1);
	if (!m_bottom)
	{
		canvas.setForeground(Color4ub(255, 255, 255));
		canvas.setBackground(getSystemColor(ScButtonFace));
	}
	else
	{
		canvas.setBackground(Color4ub(255, 255, 255));
		canvas.setForeground(getSystemColor(ScButtonFace));
	}
	canvas.fillGradientRect(rcTabs);
	
	// Draw close button.
	if (m_closeButton && m_pages.size())
		drawClose(canvas, rcInner.right - 16, (y0 + y1 - 6) / 2);
	
	// White separator.
	canvas.setForeground(Color4ub(255, 255, 255));
	canvas.setBackground(getSystemColor(ScButtonFace));
	canvas.drawLine(Point(rcTabs.left, rcTabs.bottom), Point(rcTabs.right, rcTabs.bottom));
	
	// Draw tab pages.
	if (!m_pages.empty())
	{
		int32_t left = rcTabs.left + 4;
		for (page_state_vector_t::iterator i = m_pages.begin(); i != m_pages.end(); ++i)
		{
			Ref< TabPage >& page = i->page;
			std::wstring text = page->getText();

			// Calculate extent.
			canvas.setFont(page == m_selectedPage ? m_fontBold : getFont());
			Size sizText = canvas.getTextExtent(text);
						
			// Save right separator position in vector.
			i->right = left + m_imageWidth + sizText.cx + 16;

			// Draw only those tabs that are visible.
			if (i->right < rcTabs.right)
			{
				// Calculate tab item rectangle.
				Rect rcTab(
					left,
					m_bottom ? rcTabs.top : rcTabs.top + 3,
					left + m_imageWidth + sizText.cx + 16,
					m_bottom ? rcTabs.bottom - 2 : rcTabs.bottom
				);
				
				// Draw separator.
				canvas.setForeground(Color4ub(128, 128, 128));
				canvas.drawLine(Point(rcTab.right, rcTab.top + 1), Point(rcTab.right, rcTab.bottom - 1));
				
				// Highlight selected tab.
				if (page == m_selectedPage)
				{
					canvas.fillRect(rcTab);
					
					canvas.setForeground(Color4ub(128, 128, 128));
					canvas.drawLine(Point(rcTab.left, rcTab.bottom - 1), Point(rcTab.left, rcTab.top));
					if (!m_bottom)
						canvas.drawLine(Point(rcTab.left, rcTab.top), Point(rcTab.right, rcTab.top));
					
					canvas.setForeground(Color4ub(0, 0, 0));
					canvas.drawLine(Point(rcTab.right, rcTab.top), Point(rcTab.right, rcTab.bottom - 1));
					if (m_bottom)
						canvas.drawLine(Point(rcTab.left + 1, rcTab.bottom - 1), Point(rcTab.right, rcTab.bottom - 1));
				}

				// Draw image.
				if (m_image)
				{
					canvas.drawBitmap(
						rcTab.getTopLeft() + Size(1, 1),
						Point(page->getImageIndex() * m_imageWidth, 0),
						Size(m_imageWidth, m_imageHeight),
						m_image,
						BmAlpha
					);
					rcTab.left += m_imageWidth;
				}
				
				// Draw text.
				if (isEnable())
				{
					canvas.setForeground((page == m_selectedPage) ? Color4ub(0, 0, 0) : Color4ub(128, 128, 128));
					canvas.drawText(rcTab, text, AnCenter, AnCenter);
				}
				else
				{
					canvas.setForeground(Color4ub(250, 250, 250));
					canvas.drawText(rcTab, text, AnCenter, AnCenter);
					
					rcTab.left -= 1;
					rcTab.top -= 1;
					canvas.setForeground(Color4ub(120, 120, 120));
					canvas.drawText(rcTab, text, AnCenter, AnCenter);
				}
			}

			left += m_imageWidth + sizText.cx + 16;
		}
	}
	else
	{
		// No tab pages, fill solid background.
		Rect rcTabItem(
			rcInner.left,
			m_bottom ? rcInner.top : rcInner.top + c_tabHeight + 1,
			rcInner.right,
			m_bottom ? rcInner.bottom - c_tabHeight - 1 : rcInner.bottom
		);
		canvas.setBackground(getSystemColor(ScButtonFace));
		canvas.fillRect(rcTabItem);
	}

	// Draw line.
	if (m_drawLine)
	{
		canvas.setForeground(Color4ub(128, 128, 128));
		if (!m_bottom)
			canvas.drawLine(rcInner.left, rcInner.top + c_tabHeight - 1, rcInner.right, rcInner.top + c_tabHeight - 1);
		else
			canvas.drawLine(rcInner.left, rcInner.bottom - c_tabHeight, rcInner.right, rcInner.bottom - c_tabHeight);
	}
	
	// Draw surrounding gray border.
	if (m_drawBorder)
	{
		canvas.setForeground(Color4ub(128, 128, 128));
		canvas.drawRect(rcInner);
	}

	event->consume();
}

Tab::PageState* Tab::findPageState(const TabPage* page)
{
	for (page_state_vector_t::iterator i = m_pages.begin(); i != m_pages.end(); ++i)
	{
		if (i->page == page)
			return &(*i);
	}
	return 0;
}

Tab::PageState* Tab::findPageState(int32_t depth)
{
	for (page_state_vector_t::iterator i = m_pages.begin(); i != m_pages.end(); ++i)
	{
		if (i->depth == depth)
			return &(*i);
	}
	return 0;
}

void Tab::drawClose(Canvas& canvas, int32_t x, int32_t y)
{
	canvas.setForeground(Color4ub(128, 128, 128));

	if (m_closeHighlight)
	{
		ui::Rect rc(x - 3, y - 3, x + 11, y + 10);
		canvas.setForeground(Color4ub(128, 128, 144));
		canvas.setBackground(Color4ub(224, 224, 255));
		canvas.fillRect(rc);
		canvas.drawRect(rc);
	}

	canvas.drawLine(Point(x, y), Point(x + 6, y + 6));
	canvas.drawLine(Point(x + 1, y), Point(x + 7, y + 6));
	canvas.drawLine(Point(x + 6, y), Point(x, y + 6));
	canvas.drawLine(Point(x + 7, y), Point(x + 1, y + 6));
}

#if defined(_DEBUG)
void Tab::checkPageStates()
{
	std::vector< int32_t > counts(m_pages.size(), 0);
	for (page_state_vector_t::const_iterator i = m_pages.begin(); i != m_pages.end(); ++i)
	{
		int32_t depth = i->depth;
		T_ASSERT (depth < counts.size());

		counts[depth]++;
	}
	for (std::vector< int32_t >::const_iterator i = counts.begin(); i != counts.end(); ++i)
		T_ASSERT (*i == 1);
}
#endif

Tab::PageState::PageState(TabPage* page_, int32_t right_)
:	page(page_)
,	right(right_)
,	depth(0)
{
}

bool Tab::PageState::operator == (const Tab::PageState& rh) const
{
	return page == rh.page;
}

	}
}

