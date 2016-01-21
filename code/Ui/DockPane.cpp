#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/DockPane.h"
#include "Ui/StyleSheet.h"
#include "Ui/StyleBitmap.h"

#include "Resources/DockClose.h"
#include "Resources/DockGripper.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const int c_splitterDim = 6;
const int c_gripperDim = 21;
const int c_minimumSplit = c_gripperDim + 64;

template < typename EventType >
void addEventHandlers(Widget* widget, ui::EventSubject::IEventHandler* eventHandler)
{
	widget->addEventHandler< EventType >(eventHandler);
	for (Ref< Widget > child = widget->getFirstChild(); child; child = child->getNextSibling())
		addEventHandlers< EventType >(child, eventHandler);
}

template < typename EventType >
void removeEventHandlers(Widget* widget, ui::EventSubject::IEventHandler* eventHandler)
{
	widget->removeEventHandler< EventType >(eventHandler);
	for (Ref< Widget > child = widget->getFirstChild(); child; child = child->getNextSibling())
		removeEventHandlers< EventType >(child, eventHandler);
}

int calculateRealSplit(const Rect& rc, int split, bool vertical)
{
	int dim = vertical ? rc.getHeight() : rc.getWidth();
	int realSplit = split >= 0 ? split : dim + split;
	if (realSplit <= c_minimumSplit)
		realSplit = c_minimumSplit;
	if (realSplit >= dim - c_minimumSplit)
		realSplit = dim - c_minimumSplit;
	realSplit += vertical ? rc.top : rc.left;
	return realSplit;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.DockPane", DockPane, Object)

DockPane::DockPane(Widget* owner, DockPane* parent)
:	m_owner(owner)
,	m_parent(parent)
,	m_detachable(false)
,	m_vertical(false)
,	m_split(0)
,	m_focus(false)
{
	m_bitmapClose = new ui::StyleBitmap(L"UI.DockClose", c_ResourceDockClose, sizeof(c_ResourceDockClose));
	T_FATAL_ASSERT (m_bitmapClose);

	m_bitmapGripper = Bitmap::load(c_ResourceDockGripper, sizeof(c_ResourceDockGripper), L"png");
	T_FATAL_ASSERT (m_bitmapGripper);

	m_focusEventHandler = new EventSubject::MethodEventHandler< DockPane, FocusEvent >(this, &DockPane::eventFocus);
}

void DockPane::split(bool vertical, int split, Ref< DockPane >& outLeftPane, Ref< DockPane >& outRightPane)
{
	outLeftPane = new DockPane(m_owner, this);
	outRightPane = new DockPane(m_owner, this);

	if (m_widget)
		removeEventHandlers< FocusEvent >(m_widget, m_focusEventHandler);

	m_widget = 0;
	m_split = split;
	m_vertical = vertical;

	m_child[0] = outLeftPane;
	m_child[1] = outRightPane;
}

void DockPane::dock(Widget* widget, bool detachable)
{
	T_ASSERT (widget);

	if (m_widget)
		removeEventHandlers< FocusEvent >(m_widget, m_focusEventHandler);

	m_widget = widget;
	m_detachable = detachable;

	m_child[0] =
	m_child[1] = 0;

	addEventHandlers< FocusEvent >(m_widget, m_focusEventHandler);
}

void DockPane::dock(Widget* widget, bool detachable, Direction direction, int split)
{
	if (m_widget)
	{
		// Already contains a widget, split our node.

		Ref< Widget > currentWidget = m_widget;
		bool currentDetachable = m_detachable;

		Ref< DockPane > leftPane, rightPane;
		DockPane::split(
			direction == DrNorth || direction == DrSouth,
			(direction == DrNorth || direction == DrWest) ? split : -split,
			leftPane,
			rightPane
		);

		if (direction == DrNorth || direction == DrWest)
		{
			leftPane->dock(widget, detachable);
			rightPane->dock(currentWidget, currentDetachable);
		}
		else
		{
			rightPane->dock(widget, detachable);
			leftPane->dock(currentWidget, currentDetachable);
		}
	}
	else	// No widget.
	{
		if (m_child[0] && m_child[1])
		{
			Ref< DockPane > paneLeft = new DockPane(m_owner, this);
			Ref< DockPane > paneRight = new DockPane(m_owner, this);

			if (direction == DrNorth || direction == DrWest)
			{
				paneLeft->dock(widget, detachable);

				paneRight->m_widget = m_widget;
				paneRight->m_detachable = m_detachable;
				paneRight->m_child[0] = m_child[0];
				paneRight->m_child[1] = m_child[1];
				paneRight->m_vertical = m_vertical;
				paneRight->m_split = m_split;

				if (paneRight->m_child[0])
					paneRight->m_child[0]->m_parent = paneRight;
				if (paneRight->m_child[1])
					paneRight->m_child[1]->m_parent = paneRight;

				m_child[0] = paneLeft;
				m_child[1] = paneRight;
				m_vertical = direction == DrNorth;
				m_split = split;
			}
			else
			{
				paneLeft->m_widget = m_widget;
				paneLeft->m_detachable = m_detachable;
				paneLeft->m_child[0] = m_child[0];
				paneLeft->m_child[1] = m_child[1];
				paneLeft->m_vertical = m_vertical;
				paneLeft->m_split = m_split;

				if (paneLeft->m_child[0])
					paneLeft->m_child[0]->m_parent = paneLeft;
				if (paneLeft->m_child[1])
					paneLeft->m_child[1]->m_parent = paneLeft;

				paneRight->dock(widget, detachable);

				m_child[0] = paneLeft;
				m_child[1] = paneRight;
				m_vertical = direction == DrSouth;
				m_split = -split;
			}
		}
		else
		{
			T_ASSERT (!m_child[0] && !m_child[1]);
			dock(widget, detachable);

			if (m_parent)
			{
				if (m_parent->m_split < 0)
					m_parent->m_split = -traktor::abs(split);
				else
					m_parent->m_split = traktor::abs(split);
			}
		}
	}
}

void DockPane::undock(Widget* widget)
{
	if (m_widget == widget)
	{
		if (m_widget)
			removeEventHandlers< FocusEvent >(m_widget, m_focusEventHandler);

		m_widget = 0;
		m_child[0] = 0;
		m_child[1] = 0;
		m_detachable = false;
	}
	else if (m_child[0])
	{
		T_ASSERT (m_child[1]);
		T_ASSERT (!m_widget);

		m_child[0]->undock(widget);
		m_child[1]->undock(widget);

		if (
			m_child[0]->m_widget == 0 &&
			m_child[1]->m_widget == 0 &&
			m_child[0]->m_child[0] == 0 &&
			m_child[1]->m_child[0] == 0
		)
		{
			T_ASSERT (m_child[0]->m_child[1] == 0);
			T_ASSERT (m_child[1]->m_child[1] == 0);

			m_detachable = false;

			m_child[0] = 0;
			m_child[1] = 0;
		}
		else if (
			m_child[1]->m_widget &&
			m_child[0]->m_widget == 0 &&
			m_child[0]->m_child[0] == 0
		)
		{
			T_ASSERT (m_child[0]->m_child[1] == 0);

			m_widget = m_child[1]->m_widget;
			m_detachable = m_child[1]->m_detachable;

			m_child[0] = 0;
			m_child[1] = 0;
		}
		else if (
			m_child[0]->m_widget &&
			m_child[1]->m_widget == 0 &&
			m_child[1]->m_child[0] == 0
		)
		{
			T_ASSERT (m_child[1]->m_child[1] == 0);

			m_widget = m_child[0]->m_widget;
			m_detachable = m_child[0]->m_detachable;

			m_child[0] = 0;
			m_child[1] = 0;
		}
	}
}

void DockPane::detach()
{
	T_ASSERT (m_detachable);

	if (m_widget)
		removeEventHandlers< FocusEvent >(m_widget, m_focusEventHandler);

	if (m_parent)
	{
		Ref< DockPane > childPane;

		if (m_parent->m_child[0] == this)
			childPane = m_parent->m_child[1];
		else if (m_parent->m_child[1] == this)
			childPane = m_parent->m_child[0];

		T_ASSERT (childPane);

		m_parent->m_widget = childPane->m_widget;
		m_parent->m_child[0] = childPane->m_child[0];
		m_parent->m_child[1] = childPane->m_child[1];
		m_parent->m_detachable = childPane->m_detachable;
		m_parent->m_split = childPane->m_split;
		m_parent->m_vertical = childPane->m_vertical;

		if (m_parent->m_child[0])
			m_parent->m_child[0]->m_parent = m_parent;
		if (m_parent->m_child[1])
			m_parent->m_child[1]->m_parent = m_parent;
	}

	m_parent = 0;
	m_widget = 0;
	m_child[0] = 0;
	m_child[1] = 0;
	m_detachable = false;
}

void DockPane::update(const Rect& rect, std::vector< WidgetRect >& outWidgetRects)
{
	m_rect = rect;
	if (m_widget)
	{
		Rect widgetRect = rect;
		if (m_detachable)
			widgetRect.top += scaleBySystemDPI(c_gripperDim);
		if (m_widget)
			outWidgetRects.push_back(WidgetRect(m_widget, widgetRect));
	}
	else
	{
		bool childVisible1 = m_child[0] ? m_child[0]->isVisible() : false;
		bool childVisible2 = m_child[1] ? m_child[1]->isVisible() : false;

		if (childVisible1 && childVisible2)
		{
			Rect childRects[2] = { rect, rect };
			if (m_vertical)
			{
				int split = calculateRealSplit(rect, m_split, true);
				childRects[0].bottom = split - c_splitterDim / 2;
				childRects[1].top = split + c_splitterDim / 2;
			}
			else
			{
				int split = calculateRealSplit(rect, m_split, false);
				childRects[0].right = split - c_splitterDim / 2;
				childRects[1].left = split + c_splitterDim / 2;
			}
			m_child[0]->update(childRects[0], outWidgetRects);
			m_child[1]->update(childRects[1], outWidgetRects);
		}
		else
		{
			if (childVisible1)
				m_child[0]->update(rect, outWidgetRects);
			if (childVisible2)
				m_child[1]->update(rect, outWidgetRects);
		}
	}
}

void DockPane::draw(Canvas& canvas)
{
	if (!isVisible())
		return;

	const StyleSheet* ss = Application::getInstance()->getStyleSheet();

	if (isSplitter() && m_child[0]->isVisible() && m_child[1]->isVisible())
	{
		Rect splitterRect = m_rect;
		if (m_vertical)
		{
			int split = calculateRealSplit(m_rect, m_split, true);
			splitterRect.left += 2;
			splitterRect.right -= 2;
			splitterRect.top = split - c_splitterDim / 2;
			splitterRect.bottom = split + c_splitterDim / 2;
			canvas.setBackground(ss->getColor(m_owner, L"splitter-color"));
			canvas.fillRect(splitterRect);
		}
		else
		{
			int split = calculateRealSplit(m_rect, m_split, false);
			splitterRect.left = split - 1;
			splitterRect.right = split + 1;
			splitterRect.top += 2;
			splitterRect.bottom -= 2;

			canvas.setForeground(ss->getColor(m_owner, L"background-color"));
			canvas.setBackground(ss->getColor(m_owner, L"splitter-color"));
			canvas.fillGradientRect(Rect(splitterRect.left, splitterRect.top, splitterRect.right, splitterRect.getCenter().y));

			canvas.setForeground(ss->getColor(m_owner, L"splitter-color"));
			canvas.setBackground(ss->getColor(m_owner, L"background-color"));
			canvas.fillGradientRect(Rect(splitterRect.left,splitterRect.getCenter().y, splitterRect.right, splitterRect.bottom));
		}
	}

	if (m_widget && m_detachable)
	{
		Rect captionRect = m_rect;
		captionRect.bottom = captionRect.top + scaleBySystemDPI(c_gripperDim);

		if (m_focus)
		{
			canvas.setBackground(ss->getColor(m_owner, L"caption-background-color"));
			canvas.fillRect(captionRect);
		}

		canvas.setForeground(ss->getColor(m_owner, m_focus ? L"caption-color-focus" : L"caption-color-no-focus"));

		Rect titleRect = captionRect.offset(0, -1);
		titleRect.left += 4;
		titleRect.right -= 16;

		std::wstring title = m_widget->getText();

		Size titleExtent = canvas.getTextExtent(title);
		if (titleExtent.cx > titleRect.getWidth())
		{
			while (title.length() > 0 && titleExtent.cx > titleRect.getWidth())
			{
				title = title.substr(0, title.length() - 1);
				titleExtent = canvas.getTextExtent(title + L"...");
			}
			title += L"...";
		}

		canvas.drawText(titleRect, title, AnLeft, AnCenter);

		int32_t gx = titleRect.left + titleExtent.cx + 4;
		int32_t gx1 = captionRect.right - 16 - 4;
		while (gx < gx1 - 256)
		{
			int32_t w = min(256, gx1 - gx);
			canvas.drawBitmap(
				Point(gx, captionRect.getCenter().y - 3),
				Point(0, m_focus ? 5 : 0),
				Size(w, 5),
				m_bitmapGripper,
				BmAlpha
			);
			gx += 256;
		}

		while (gx < gx1)
		{
			canvas.drawBitmap(
				Point(gx, captionRect.getCenter().y - 3),
				Point(0, m_focus ? 5 : 0),
				Size(4, 5),
				m_bitmapGripper,
				BmAlpha
			);
			gx += 4;
		}

		// \fixme White when focus
		canvas.drawBitmap(
			Point(captionRect.right -  m_bitmapClose->getSize().cx - 4, captionRect.getCenter().y - m_bitmapClose->getSize().cy / 2),
			Point(0, 0),
			m_bitmapClose->getSize(),
			m_bitmapClose,
			BmAlpha
		);
	}

	if (m_child[0])
		m_child[0]->draw(canvas);
	if (m_child[1])
		m_child[1]->draw(canvas);
}

Ref< DockPane > DockPane::findWidgetPane(Widget* widget)
{
	if (m_widget == widget)
		return this;

	for (uint32_t i = 0; i < 2; ++i)
	{
		if (!m_child[i])
			continue;

		Ref< DockPane > pane = m_child[i]->findWidgetPane(widget);
		if (pane)
			return pane;
	}

	return 0;
}

Ref< DockPane > DockPane::getPaneFromPosition(const Point& position)
{
	if (!m_rect.inside(position))
		return 0;

	for (int i = 0; i < 2; ++i)
	{
		if (m_child[i])
		{
			Ref< DockPane > childPane = m_child[i]->getPaneFromPosition(position);
			if (childPane)
				return childPane;
		}
	}

	if (m_widget && m_widget->isVisible(false))
		return this;
	else
		return 0;
}

Ref< DockPane > DockPane::getSplitterFromPosition(const Point& position)
{
	if (!m_rect.inside(position))
		return 0;

	for (int i = 0; i < 2; ++i)
	{
		if (m_child[i])
		{
			Ref< DockPane > childPane = m_child[i]->getSplitterFromPosition(position);
			if (childPane)
				return childPane;
		}
	}

	if (m_widget && m_widget->isVisible(false))
		return 0;
	else
		return this;
}

bool DockPane::hitGripper(const Point& position) const
{
	T_ASSERT (m_rect.inside(position));

	if (isSplitter() || (m_widget && !m_widget->isVisible(false)))
		return false;

	return position.y >= m_rect.top && position.y <= m_rect.top + scaleBySystemDPI(c_gripperDim);
}

bool DockPane::hitGripperClose(const Point& position) const
{
	T_ASSERT (m_rect.inside(position));

	if (isSplitter() || !hitGripper(position))
		return false;

	return position.x >= m_rect.right - 16;
}

bool DockPane::hitSplitter(const Point& position) const
{
	T_ASSERT (m_rect.inside(position));

	if (!isSplitter())
		return false;

	int pos, split;
	if (m_vertical)
	{
		pos = position.y;
		split = calculateRealSplit(m_rect, m_split, true);
	}
	else
	{
		pos = position.x;
		split = calculateRealSplit(m_rect, m_split, false);
	}

	return pos >= split - c_splitterDim / 2 && pos <= split + c_splitterDim / 2;
}

void DockPane::setSplitterPosition(const Point& position)
{
	int pos = m_vertical ? (position.y - m_rect.top) : (position.x - m_rect.left);
	int extent = m_vertical ? m_rect.getHeight() : m_rect.getWidth();

	pos = std::max(0, pos);
	pos = std::min(extent, pos);

	if (m_split < 0)
		m_split = -(extent - pos);
	else
		m_split = pos;
}

bool DockPane::isVisible() const
{
	if (isSplitter())
		return m_child[0]->isVisible() || m_child[1]->isVisible();

	return m_widget ? m_widget->isVisible(false) : false;
}

void DockPane::dump()
{
	log::info << L"Pane \"" << (m_widget ? m_widget->getText().c_str() : L"null") << L"\"" << Endl;
	log::info << L" \"" << (m_widget ? type_name(m_widget) : L"NA") << L"\"" << Endl;
	log::info << L" m_detachable " << m_detachable << Endl;
	log::info << L" m_vertical " << m_vertical << Endl;
	log::info << L" m_split " << m_split << Endl;
	log::info << L" m_focus " << m_focus << Endl;
	log::info << L" m_child[0]" << Endl;
	log::info << IncreaseIndent;
	if (m_child[0])
		m_child[0]->dump();
	else
		log::info << L"<null>" << Endl;
	log::info << DecreaseIndent;

	log::info << L" m_child[1]" << Endl;
	log::info << IncreaseIndent;
	if (m_child[1])
		m_child[1]->dump();
	else
		log::info << L"<null>" << Endl;
	log::info << DecreaseIndent;
}

void DockPane::eventFocus(FocusEvent* event)
{
	bool focus = event->gotFocus();
	if (focus != m_focus)
	{
		m_focus = focus;
		if (m_owner)
			m_owner->update();
	}
}

	}
}
