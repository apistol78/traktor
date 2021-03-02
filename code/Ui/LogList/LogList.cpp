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
#include "Ui/ScrollBar.h"
#include "Ui/LogList/LogList.h"

// Resources
#include "Resources/Log.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.LogList", LogList, Widget)

bool LogList::create(Widget* parent, int style, const ISymbolLookup* lookup)
{
	if (!Widget::create(parent, style | WsDoubleBuffer | WsAccelerated))
		return false;

	addEventHandler< PaintEvent >(this, &LogList::eventPaint);
	addEventHandler< SizeEvent >(this, &LogList::eventSize);
	addEventHandler< MouseWheelEvent >(this, &LogList::eventMouseWheel);

	m_scrollBar = new ScrollBar();
	if (!m_scrollBar->create(this, ScrollBar::WsVertical))
		return false;

	m_scrollBar->addEventHandler< ScrollEvent >(this, &LogList::eventScroll);

	m_icons = new StyleBitmap(L"UI.Log", c_ResourceLog, sizeof(c_ResourceLog));

	m_itemHeight = getFontMetric().getHeight() + dpi96(2);
	m_itemHeight = std::max< int >(m_itemHeight, m_icons->getSize().cy);

	m_lookup = lookup;
	return true;
}

void LogList::add(uint32_t threadId, LogLevel level, const std::wstring& text)
{
	Entry e;
	e.threadId = threadId;
	e.level = level;
	e.text = text;

	// Parse embedded guid;s in log.
	if (m_lookup)
	{
		size_t i = 0;
		for (;;)
		{
			size_t j = e.text.find(L'{', i);
			if (j == std::wstring::npos)
				break;

			Guid id(e.text.substr(j));
			if (id.isValid())
			{
				std::wstring symbol;
				if (m_lookup->lookupLogSymbol(id, symbol))
				{
					e.text = e.text.substr(0, j) + symbol + e.text.substr(j + 38);
					continue;
				}
			}

			i = j + 1;
		}
	}

	if ((level & (LvInfo | LvDebug)) != 0)
		m_logCount[0]++;
	if ((level & LvWarning) != 0)
		m_logCount[1]++;
	if ((level & LvError) != 0)
		m_logCount[2]++;

	// Add to pending list; coalesced before control is redrawn.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_pendingLock);
		m_pending.push_back(e);
		if (m_threadIndices.find(e.threadId) == m_threadIndices.end())
			m_threadIndices.insert(std::make_pair(e.threadId, m_nextThreadIndex++));
	}

	update();
}

void LogList::removeAll()
{
	m_logFull.clear();
	m_logFiltered.clear();
	m_logCount[0] = m_logCount[1] = m_logCount[2] = 0;
	m_threadIndices.clear();
	m_nextThreadIndex = 0;

	updateScrollBar();
	update();
}

void LogList::setFilter(uint8_t filter)
{
	m_filter = filter;
	m_logFiltered.clear();

	for (const auto& log : m_logFull)
	{
		if ((log.level & m_filter) != 0)
			m_logFiltered.push_back(log);
	}

	updateScrollBar();
	update();
}

uint8_t LogList::getFilter() const
{
	return m_filter;
}

bool LogList::copyLog(uint8_t filter)
{
	StringOutputStream ss;
	for (const auto& log : m_logFull)
	{
		if ((log.level & filter) != 0)
			ss << log.text << Endl;
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
	const StyleSheet* ss = getStyleSheet();

	// Coalesce pending log statements.
	int32_t added = 0;
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_pendingLock);
		for (const auto& log : m_pending)
		{
			m_logFull.push_back(log);
			if ((log.level & m_filter) != 0)
			{
				m_logFiltered.push_back(log);
				++added;
			}
		}

		m_pending.clear();
	}
	if (added > 0)
	{
		updateScrollBar();
		m_scrollBar->setPosition((int32_t)m_logFiltered.size());
	}

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

	for (; i != m_logFiltered.end() && rc.top < inner.bottom; ++i)
	{
		uint32_t threadIndex = m_threadIndices[i->threadId];

		Size iconSize(m_icons->getSize().cy, m_icons->getSize().cy);
		Point iconPos = rc.getTopLeft() + Size(0, (rc.getHeight() - iconSize.cy) / 2);
		switch (i->level)
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
		textRect.left += dpi96(20);

		size_t s = 0;
		while (s < i->text.length())
		{
			size_t e1 = i->text.find_first_not_of('\t', s);
			if (e1 == i->text.npos)
				break;

			textRect.left += int32_t(e1 - s) * dpi96(8 * 4);

			size_t e2 = i->text.find_first_of('\t', e1);
			if (e2 == i->text.npos)
				e2 = i->text.length();

			std::wstring text = i->text.substr(e1, e2 - e1);
			canvas.drawText(textRect, text, AnLeft, AnCenter);

			Size extent = canvas.getFontMetric().getExtent(text);
			textRect.left += extent.cx;

			s = e2;
		}

		rc = rc.offset(0, m_itemHeight);
	}

	if (m_logCount[1] > 0 || m_logCount[2] > 0)
	{
		std::wstring ws = str(L"%d", m_logCount[1]);
		std::wstring es = str(L"%d", m_logCount[2]);

		int32_t w = std::max< int32_t>(
			canvas.getFontMetric().getExtent(ws).cx,
			canvas.getFontMetric().getExtent(es).cx
		);

		w += dpi96(16);

		Rect rcCount = inner;
		rcCount.top = rcCount.bottom - m_itemHeight;

		rcCount.left = inner.right - w * 2;
		rcCount.right = inner.right - w * 1;

		canvas.setForeground(levelColors[1]);
		canvas.setBackground(levelBgColors[1]);
		canvas.fillRect(rcCount);
		canvas.drawText(rcCount, ws, AnCenter, AnCenter);

		rcCount.left = inner.right - w * 1;
		rcCount.right = inner.right - w * 0;

		canvas.setForeground(levelColors[2]);
		canvas.setBackground(levelBgColors[2]);
		canvas.fillRect(rcCount);
		canvas.drawText(rcCount, es, AnCenter, AnCenter);
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

	}
}
