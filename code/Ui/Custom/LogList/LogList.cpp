#include "Core/Guid.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/StyleBitmap.h"
#include "Ui/StyleSheet.h"
#include "Ui/Custom/ScrollBar.h"
#include "Ui/Custom/LogList/LogList.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.LogList", LogList, Widget)

LogList::LogList()
:	m_lookup(0)
,	m_itemHeight(0)
,	m_filter(LvInfo | LvWarning | LvError)
,	m_nextThreadIndex(0)
{
}

bool LogList::create(Widget* parent, int style, const ISymbolLookup* lookup)
{
	if (!Widget::create(parent, style | WsDoubleBuffer | WsAccelerated))
		return false;

	addEventHandler< PaintEvent >(this, &LogList::eventPaint);
	addEventHandler< SizeEvent >(this, &LogList::eventSize);
	addEventHandler< MouseWheelEvent >(this, &LogList::eventMouseWheel);
	addEventHandler< TimerEvent >(this, &LogList::eventTimer);

	m_scrollBar = new ScrollBar();
	if (!m_scrollBar->create(this, ScrollBar::WsVertical))
		return false;

	m_scrollBar->addEventHandler< ScrollEvent >(this, &LogList::eventScroll);

	m_icons = new StyleBitmap(L"UI.Log");

	m_itemHeight = getFont().getPixelSize() + scaleBySystemDPI(4);
	m_itemHeight = std::max< int >(m_itemHeight, m_icons->getSize().cy);

	m_lookup = lookup;

	startTimer(100, 0);

	return true;
}

void LogList::add(LogLevel level, const std::wstring& text)
{
	Entry e;
	e.threadId = ThreadManager::getInstance().getCurrentThread()->id();
	e.logLevel = level;
	e.logText = text;

	// Parse embedded guid;s in log.
	if (m_lookup)
	{
		size_t i = 0;
		for (;;)
		{
			size_t j = e.logText.find(L'{', i);
			if (j == std::wstring::npos)
				break;

			Guid id(e.logText.substr(j));
			if (id.isValid())
			{
				std::wstring symbol;
				if (m_lookup->lookupLogSymbol(id, symbol))
				{
					e.logText = e.logText.substr(0, j) + symbol + e.logText.substr(j + 38);
					continue;
				}
			}

			i = j + 1;
		}
	}

	// Add to pending list; coalesced before control is redrawn.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_pendingLock);
		m_pending.push_back(e);
		if (m_threadIndices.find(e.threadId) == m_threadIndices.end())
			m_threadIndices.insert(std::make_pair(e.threadId, m_nextThreadIndex++));
	}
}

void LogList::removeAll()
{
	m_logFull.clear();
	m_logFiltered.clear();
	m_threadIndices.clear();
	m_nextThreadIndex = 0;

	updateScrollBar();
	update();
}

void LogList::setFilter(uint32_t filter)
{
	m_filter = filter;
	m_logFiltered.clear();

	for (log_list_t::const_iterator i = m_logFull.begin(); i != m_logFull.end(); ++i)
	{
		if ((i->logLevel & m_filter) != 0)
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
		if ((i->logLevel & filter) != 0)
			ss << i->logText << Endl;
	}
	return Application::getInstance()->getClipboard()->setText(ss.str());
}

Size LogList::getPreferedSize() const
{
	return Size(200, 150);
}

void LogList::updateScrollBar()
{
	Rect inner = getInnerRect();

	int32_t logCount = int(m_logFiltered.size());
	int32_t pageCount = inner.getHeight() / m_itemHeight;

	m_scrollBar->setRange(logCount);
	m_scrollBar->setPage(pageCount);
	m_scrollBar->setVisible(logCount > pageCount);
	m_scrollBar->update();
}

void LogList::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();

	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	const Color4ub levelColors[] =
	{
		ss->getColor(this, L"color-info"),
		ss->getColor(this, L"color-warning"),
		ss->getColor(this, L"color-error")
	};
	const Color4ub levelBgColors[] =
	{
		ss->getColor(this, L"background-color-info"),
		ss->getColor(this, L"background-color-warning"),
		ss->getColor(this, L"background-color-error")
	};

	// Get inner rectangle, adjust for scrollbar.
	Rect inner = getInnerRect();
	if (m_scrollBar->isVisible(false))
		inner.right -= m_scrollBar->getPreferedSize().cx;

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(inner);

	int32_t scrollOffset = m_scrollBar->getPosition();
	Rect rc(inner.getTopLeft(), Size(inner.getWidth(), m_itemHeight));

	log_list_t::const_iterator i = m_logFiltered.begin();

	// Advance iterator by scroll offset, keep a page worth of lines.
	int32_t advanceCount = scrollOffset;
	advanceCount = std::max(0, advanceCount);
	advanceCount = std::min(advanceCount, int32_t(m_logFiltered.size()));
	std::advance(i, advanceCount);

	for (; i != m_logFiltered.end(); ++i)
	{
		uint32_t threadIndex = m_threadIndices[i->threadId];

		Size iconSize(m_icons->getSize().cy, m_icons->getSize().cy);
		Point iconPos = rc.getTopLeft() + Size(0, (rc.getHeight() - iconSize.cy) / 2);
		switch (i->logLevel)
		{
		case LvDebug:
		case LvInfo:
			if (levelBgColors[0].a != 0)
			{
				canvas.setBackground(levelBgColors[0]);
				canvas.fillRect(rc);
			}
			canvas.drawBitmap(iconPos, Point(0, 0), iconSize, m_icons, BmAlpha);
			canvas.setForeground(levelColors[0]);
			break;

		case LvWarning:
			if (levelBgColors[1].a != 0)
			{
				canvas.setBackground(levelBgColors[1]);
				canvas.fillRect(rc);
			}
			canvas.drawBitmap(iconPos, Point(iconSize.cx, 0), iconSize, m_icons, BmAlpha);
			canvas.setForeground(levelColors[1]);
			break;

		case LvError:
			if (levelBgColors[2].a != 0)
			{
				canvas.setBackground(levelBgColors[2]);
				canvas.fillRect(rc);
			}
			canvas.drawBitmap(iconPos, Point(2 * iconSize.cx, 0), iconSize, m_icons, BmAlpha);
			canvas.setForeground(levelColors[2]);
			break;

		default:
			if (levelBgColors[0].a != 0)
				canvas.setBackground(levelBgColors[0]);
			canvas.fillRect(rc);
			canvas.setForeground(levelColors[0]);
			break;
		}

		Rect textRect(rc.left + iconSize.cx, rc.top, rc.right, rc.bottom);
		canvas.drawText(textRect, toString(threadIndex) + L">", AnLeft, AnCenter);
		textRect.left += scaleBySystemDPI(20);

		size_t s = 0;
		while (s < i->logText.length())
		{
			size_t e1 = i->logText.find_first_not_of('\t', s);
			if (e1 == i->logText.npos)
				break;

			textRect.left += int32_t(e1 - s) * scaleBySystemDPI(8 * 4);

			size_t e2 = i->logText.find_first_of('\t', e1);
			if (e2 == i->logText.npos)
				e2 = i->logText.length();

			if (e2 <= e1)
				break;

			std::wstring text = i->logText.substr(e1, e2 - e1);
			canvas.drawText(textRect, text, AnLeft, AnCenter);

			Size extent = canvas.getTextExtent(text);
			textRect.left += extent.cx;

			s += e2;
		}

		rc = rc.offset(0, m_itemHeight);
	}

	event->consume();
}

void LogList::eventSize(SizeEvent* event)
{
	int32_t width = m_scrollBar->getPreferedSize().cx;

	Rect inner = getInnerRect();
	Rect rc(Point(inner.getWidth() - width, 0), Size(width, inner.getHeight()));

	m_scrollBar->setRect(rc);

	updateScrollBar();
}

void LogList::eventMouseWheel(MouseWheelEvent* event)
{
	int32_t position = m_scrollBar->getPosition();
	position -= event->getRotation() * 4;
	m_scrollBar->setPosition(position);
	update();
}

void LogList::eventScroll(ScrollEvent* event)
{
	update();
}

void LogList::eventTimer(TimerEvent* event)
{
	int32_t added = 0;

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_pendingLock);

		if (m_pending.empty())
			return;

		for (log_list_t::const_iterator i = m_pending.begin(); i != m_pending.end(); ++i)
		{
			m_logFull.push_back(*i);
			if ((i->logLevel & m_filter) != 0)
			{
				m_logFiltered.push_back(*i);
				++added;
			}
		}

		m_pending.clear();
	}

	if (added > 0)
	{
		updateScrollBar();
		m_scrollBar->setPosition(int32_t(m_logFiltered.size()));
	}

	update();
}

		}
	}
}
