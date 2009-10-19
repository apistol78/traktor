#include "Ui/Custom/LogList/LogList.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Bitmap.h"
#include "Ui/ScrollBar.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Drawing/Image.h"
#include "Core/Thread/Acquire.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/Split.h"

// Resources
#include "Resources/Log.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.LogList", LogList, Widget)

LogList::LogList()
:	m_itemHeight(0)
,	m_filter(LvInfo | LvWarning | LvError)
{
}

bool LogList::create(Widget* parent, int style)
{
	if (!Widget::create(parent, style | WsDoubleBuffer))
		return false;

	addPaintEventHandler(createMethodHandler(this, &LogList::eventPaint));
	addSizeEventHandler(createMethodHandler(this, &LogList::eventSize));
	addMouseWheelEventHandler(createMethodHandler(this, &LogList::eventMouseWheel));
	addTimerEventHandler(createMethodHandler(this, &LogList::eventTimer));

	m_scrollBar = gc_new< ScrollBar >();
	if (!m_scrollBar->create(this, ScrollBar::WsVertical))
		return false;

	m_scrollBar->addScrollEventHandler(createMethodHandler(this, &LogList::eventScroll));

	m_icons = Bitmap::load(c_ResourceLog, sizeof(c_ResourceLog), L"png");

	m_itemHeight = abs(getFont().getSize()) + 3;
	m_itemHeight = std::max< int >(m_itemHeight, m_icons->getSize().cy);

	startTimer(100, 0);

	return true;
}

void LogList::add(LogLevel level, const std::wstring& text)
{
	Acquire< Semaphore > lock(m_pendingLock);
	m_pending.push_back(std::make_pair(level, text));
}

void LogList::removeAll()
{
	m_logFull.clear();
	m_logFiltered.clear();
	updateScrollBar();
	update();
}

void LogList::setFilter(uint32_t filter)
{
	m_filter = filter;
	m_logFiltered.clear();
	for (log_list_t::const_iterator i = m_logFull.begin(); i != m_logFull.end(); ++i)
	{
		if ((i->first & m_filter) != 0)
			m_logFiltered.push_back(*i);
	}
	updateScrollBar();
	update();
}

uint32_t LogList::getFilter() const
{
	return m_filter;
}

bool LogList::copyLog(uint32_t filter)
{
	StringOutputStream ss;
	for (log_list_t::iterator i = m_logFull.begin(); i != m_logFull.end(); ++i)
	{
		if ((i->first & filter) != 0)
			ss << i->second << Endl;
	}
	return Application::getInstance().getClipboard()->setText(ss.str());
}

Size LogList::getPreferedSize() const
{
	return Size(200, 150);
}

void LogList::updateScrollBar()
{
	Rect inner = getInnerRect();

	int logCount = int(m_logFiltered.size());
	int pageCount = inner.getHeight() / m_itemHeight;

	m_scrollBar->setRange(logCount);
	m_scrollBar->setPage(pageCount);
	m_scrollBar->setVisible(logCount > pageCount);
	m_scrollBar->update();
}

void LogList::eventPaint(Event* event)
{
	PaintEvent* paintEvent = checked_type_cast< PaintEvent* >(event);
	Canvas& canvas = paintEvent->getCanvas();

	const Color c_color[] = { Color(255, 255, 255), Color(255, 255, 200), Color(255, 200, 200) };

	// Get inner rectangle, adjust for scrollbar.
	Rect inner = getInnerRect();
	if (m_scrollBar->isVisible(false))
		inner.right -= m_scrollBar->getPreferedSize().cx;

	canvas.setBackground(getSystemColor(ScWindowBackground));
	canvas.fillRect(inner);

	int scrollOffset = m_scrollBar->getPosition();
	Rect rc(inner.getTopLeft(), Size(inner.getWidth(), m_itemHeight));

	log_list_t::const_iterator i = m_logFiltered.begin();

	// Advance iterator by scroll offset, keep a page worth of lines.
	int advanceCount = scrollOffset;
	advanceCount = std::max(0, advanceCount);
	advanceCount = std::min(advanceCount, int(m_logFiltered.size()));
	std::advance(i, advanceCount);

	for (; i != m_logFiltered.end(); ++i)
	{
		Size iconSize(m_icons->getSize().cy, m_icons->getSize().cy);
		switch (i->first)
		{
		case LvInfo:
			canvas.setBackground(c_color[0]);
			canvas.fillRect(rc);
			canvas.drawBitmap(rc.getTopLeft(), Point(0, 0), iconSize, m_icons, BmNone);
			break;
		case LvWarning:
			canvas.setBackground(c_color[1]);
			canvas.fillRect(rc);
			canvas.drawBitmap(rc.getTopLeft(), Point(iconSize.cx, 0), iconSize, m_icons, BmNone);
			break;
		case LvError:
			canvas.setBackground(c_color[2]);
			canvas.fillRect(rc);
			canvas.drawBitmap(rc.getTopLeft(), Point(2 * iconSize.cx, 0), iconSize, m_icons, BmNone);
			break;
		default:
			canvas.setBackground(c_color[0]);
			canvas.fillRect(rc);
			break;
		}

		Rect textRect(rc.left + iconSize.cx, rc.top, rc.right, rc.bottom);
		canvas.setForeground(Color(0, 0, 0));

		size_t s = 0;
		while (s < i->second.length())
		{
			size_t e1 = i->second.find_first_not_of('\t', s);

			textRect.left += int(e1 - s) * 8 * 4;

			size_t e2 = i->second.find_first_of('\t', e1);
			if (e2 == i->second.npos)
				e2 = i->second.length();

			std::wstring text = i->second.substr(e1, e2 - e1);

			canvas.drawText(textRect, text, AnLeft, AnCenter);

			Size extent = canvas.getTextExtent(text);
			textRect.left += extent.cx;

			s += e2;
		}

		rc = rc.offset(0, m_itemHeight);
	}

	paintEvent->consume();
}

void LogList::eventSize(Event* event)
{
	int width = m_scrollBar->getPreferedSize().cx;

	Rect inner = getInnerRect();
	Rect rc(Point(inner.getWidth() - width, 0), Size(width, inner.getHeight()));

	m_scrollBar->setRect(rc);

	updateScrollBar();
}

void LogList::eventMouseWheel(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent* >(event);
	
	int position = m_scrollBar->getPosition();
	position -= mouseEvent->getWheelRotation() * 4;
	m_scrollBar->setPosition(position);

	update();
}

void LogList::eventScroll(Event* event)
{
	update();
}

void LogList::eventTimer(Event* event)
{
	{
		Acquire< Semaphore > lock(m_pendingLock);

		if (m_pending.empty())
			return;

		for (log_list_t::const_iterator i = m_pending.begin(); i != m_pending.end(); ++i)
		{
			m_logFull.push_back(*i);
			if ((i->first & m_filter) != 0)
				m_logFiltered.push_back(*i);
		}

		m_pending.clear();
	}

	updateScrollBar();
	m_scrollBar->setPosition(int(m_logFiltered.size()));

	update();
}

		}
	}
}
