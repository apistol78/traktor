/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
#include "Ui/Events/LogActivateEvent.h"
#include "Ui/LogList/LogList.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.LogList", LogList, Widget)

bool LogList::create(Widget* parent, uint32_t style, const ISymbolLookup* lookup)
{
	if (!Widget::create(parent, style | WsDoubleBuffer | WsAccelerated))
		return false;

	addEventHandler< PaintEvent >(this, &LogList::eventPaint);
	addEventHandler< SizeEvent >(this, &LogList::eventSize);
	addEventHandler< MouseButtonDownEvent >(this, &LogList::eventMouseButtonDown);
	addEventHandler< MouseDoubleClickEvent >(this, &LogList::eventMouseDoubleClick);
	addEventHandler< MouseWheelEvent >(this, &LogList::eventMouseWheel);
    addEventHandler< TimerEvent >(this, &LogList::eventTimer);

	m_scrollBarH = new ScrollBar();
	if (!m_scrollBarH->create(this, ScrollBar::WsHorizontal))
		return false;

	m_scrollBarV = new ScrollBar();
	if (!m_scrollBarV->create(this, ScrollBar::WsVertical))
		return false;

	m_scrollBarH->setVisible(false);
	m_scrollBarV->setVisible(false);

	m_scrollBarH->addEventHandler< ScrollEvent >(this, &LogList::eventScroll);
	m_scrollBarV->addEventHandler< ScrollEvent >(this, &LogList::eventScroll);

	m_icons = new StyleBitmap(L"UI.Log");
	m_itemHeight = getFont().getSize() + 4_ut;
	m_lookup = lookup;
    
#if defined(__APPLE__)
	startTimer(100);
#endif
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
				e.symbolId = id;

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

#if !defined(__APPLE__)
	update();
#endif
}

void LogList::removeAll()
{
	m_logFull.clear();
	m_logFiltered.clear();
	m_logCount[0] = m_logCount[1] = m_logCount[2] = 0;
	m_threadIndices.clear();
	m_maxLineWidth = 0;
	m_nextThreadIndex = 0;
	m_selectedEntry = -1;

	updateScrollBar();
	update();
}

void LogList::setFilter(uint8_t filter)
{
	m_filter = filter;
	m_logFiltered.clear();
	m_selectedEntry = -1;

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

void LogList::selectLine(int32_t line)
{
	m_selectedEntry = line;
	update();
}

void LogList::showLine(int32_t line)
{
	if (!m_scrollBarV->isVisible(false))
		return;

	m_scrollBarV->setPosition(line);
	update();
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

void LogList::forEachLine(const std::function< void(int32_t line, const std::wstring& text) >& fn) const
{
	for (int32_t line = 0; line < (int32_t)m_logFull.size(); ++line)
		fn(line, m_logFull[line].text);
}

void LogList::forEachFilteredLine(const std::function< void(int32_t line, const std::wstring& text) >& fn) const
{
	for (int32_t line = 0; line < (int32_t)m_logFiltered.size(); ++line)
		fn(line, m_logFiltered[line].text);
}

Size LogList::getPreferredSize(const Size& hint) const
{
	return Size(200, 150);
}

void LogList::updateScrollBar()
{
	const Rect inner = getInnerRect();

	const int32_t logCount = (int32_t)m_logFiltered.size();
	const int32_t pageCount = inner.getHeight() / pixel(m_itemHeight);

	const int32_t horizRange = m_maxLineWidth - inner.getSize().cx;
	if (horizRange > 0)
	{
		m_scrollBarH->setRange(horizRange);
		m_scrollBarH->setPage(horizRange / 10);
		m_scrollBarH->setVisible(true);
	}
	else
	{
		m_scrollBarH->setPosition(0);
		m_scrollBarH->setVisible(false);
	}
	m_scrollBarH->update();

	if (logCount > pageCount)
	{
		m_scrollBarV->setRange(logCount);
		m_scrollBarV->setPage(pageCount);
		m_scrollBarV->setVisible(true);
	}
	else
	{
		m_scrollBarV->setPosition(0);
		m_scrollBarV->setVisible(false);
	}
	m_scrollBarV->update();
}

void LogList::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const StyleSheet* ss = getStyleSheet();

	// Coalesce pending log statements.
	bool needUpdateScrollBars = false;
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_pendingLock);
		for (const auto& log : m_pending)
		{
			const Size sz = canvas.getFontMetric().getExtent(log.text);
			if (sz.cx > m_maxLineWidth)
			{
				m_maxLineWidth = sz.cx;
				needUpdateScrollBars = true;
			}

			m_logFull.push_back(log);
			if ((log.level & m_filter) != 0)
			{
				m_logFiltered.push_back(log);
				needUpdateScrollBars = true;
			}
		}

		m_pending.clear();
	}

	if (needUpdateScrollBars)
	{
		updateScrollBar();
		m_scrollBarV->setPosition((int32_t)m_logFiltered.size());
	}

	const Color4ub selectedColor = ss->getColor(this, L"color-selected");
	const Color4ub selectedBgColor = ss->getColor(this, L"background-color-selected");

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

	Rect inner = getInnerRect();
	canvas.setBackground(ss->getColor(this, isEnable(true) ? L"background-color" : L"background-color-disabled"));
	canvas.fillRect(inner);

	// Adjust inner rectangle from scrollbars.
	if (m_scrollBarH->isVisible(false))
		inner.bottom -= m_scrollBarH->getPreferredSize(inner.getSize()).cy;
	if (m_scrollBarV->isVisible(false))
		inner.right -= m_scrollBarV->getPreferredSize(inner.getSize()).cx;

	Rect rc(inner.getTopLeft(), Size(inner.getWidth(), pixel(m_itemHeight)));

	// Determine "max width" of thread identifier.
	const int32_t threadIdWidth = canvas.getFontMetric().getExtent(L"000>").cx;

	// Advance by scroll offset, keep a page worth of lines.
	int32_t offsetX = m_scrollBarH->getPosition();
	int32_t advanceCount = m_scrollBarV->getPosition();
	advanceCount = std::max(0, advanceCount);
	advanceCount = std::min(advanceCount, int32_t(m_logFiltered.size()));

	for (int32_t row = advanceCount; row < (int32_t)m_logFiltered.size() && rc.top < inner.bottom; ++row)
	{
		const auto& entry = m_logFiltered[row];
		const uint32_t threadIndex = m_threadIndices[entry.threadId];

		Size iconSize(m_icons->getSize(this).cy, m_icons->getSize(this).cy);
		Point iconPos = rc.getTopLeft() + Size(0, (rc.getHeight() - iconSize.cy) / 2);

		Color4ub fg(0, 0, 0, 0);
		Color4ub bg(0, 0, 0, 0);

		switch (entry.level)
		{
		default:
		case LvDebug:
		case LvInfo:
			fg = levelColors[0];
			bg = levelBgColors[0];
			break;

		case LvWarning:
			fg = levelColors[1];
			bg = levelBgColors[1];
			break;

		case LvError:
			fg = levelColors[2];
			bg = levelBgColors[2];
			break;
		}

		if (m_selectedEntry == row)
		{
			fg = selectedColor;
			bg = selectedBgColor;
		}

		canvas.setForeground(fg);
		if (bg.a != 0)
		{
			canvas.setBackground(bg);
			canvas.fillRect(rc);
		}

		switch (entry.level)
		{
		case LvDebug:
		case LvInfo:
			canvas.drawBitmap(iconPos, Point(0, 0), iconSize, m_icons, BlendMode::Alpha);
			break;

		case LvWarning:
			canvas.drawBitmap(iconPos, Point(iconSize.cx, 0), iconSize, m_icons, BlendMode::Alpha);
			break;

		case LvError:
			canvas.drawBitmap(iconPos, Point(2 * iconSize.cx, 0), iconSize, m_icons, BlendMode::Alpha);
			break;

		default:
			break;
		}

		Rect textRect(rc.left + iconSize.cx - offsetX, rc.top, rc.right, rc.bottom);

		canvas.drawText(textRect, str(L"%d>", threadIndex), AnLeft, AnCenter);
		textRect.left += threadIdWidth;

		size_t s = 0;
		while (s < entry.text.length())
		{
			size_t e1 = entry.text.find_first_not_of('\t', s);
			if (e1 == entry.text.npos)
				break;

			textRect.left += int32_t(e1 - s) * pixel(8_ut * 4_ut);

			size_t e2 = entry.text.find_first_of('\t', e1);
			if (e2 == entry.text.npos)
				e2 = entry.text.length();

			const std::wstring text = entry.text.substr(e1, e2 - e1);
			canvas.drawText(textRect, text, AnLeft, AnCenter);

			const Size extent = canvas.getFontMetric().getExtent(text);
			textRect.left += extent.cx;

			s = e2;
		}

		rc = rc.offset(0, pixel(m_itemHeight));
	}

	// Draw number of warnings or errors.
	if (m_logCount[1] > 0 || m_logCount[2] > 0)
	{
		std::wstring ws = str(L"%d", m_logCount[1]);
		std::wstring es = str(L"%d", m_logCount[2]);

		int32_t w = std::max< int32_t>(
			canvas.getFontMetric().getExtent(ws).cx,
			canvas.getFontMetric().getExtent(es).cx
		);

		w += pixel(16_ut);

		Rect rcCount = inner;
		rcCount.top = rcCount.bottom - pixel(m_itemHeight);

		int32_t x = inner.right;
		if (m_logCount[2] > 0)
		{
			rcCount.left = x - w;
			rcCount.right = x;

			canvas.setForeground(levelColors[2]);
			canvas.setBackground(levelBgColors[2]);
			canvas.fillRect(rcCount);
			canvas.drawText(rcCount, es, AnCenter, AnCenter);

			x -= w;	
		}
		if (m_logCount[1] > 0)
		{
			rcCount.left = x - w;
			rcCount.right = x;

			canvas.setForeground(levelColors[1]);
			canvas.setBackground(levelBgColors[1]);
			canvas.fillRect(rcCount);
			canvas.drawText(rcCount, ws, AnCenter, AnCenter);

			x -= w;	
		}
	}

	event->consume();
}

void LogList::eventSize(SizeEvent* event)
{
	const Rect inner = getInnerRect();
	const int32_t width = m_scrollBarV->getPreferredSize(inner.getSize()).cx;
	const int32_t height = m_scrollBarH->getPreferredSize(inner.getSize()).cy;

	{
		Rect rc(Point(0, inner.getHeight() - height), Size(inner.getWidth(), height));
		if (m_scrollBarV->isVisible(false))
			rc.right -= width;
		m_scrollBarH->setRect(rc);
	}

	{
		Rect rc(Point(inner.getWidth() - width, 0), Size(width, inner.getHeight()));
		m_scrollBarV->setRect(rc);
	}

	updateScrollBar();
}

void LogList::eventMouseButtonDown(MouseButtonDownEvent* event)
{
	int32_t advanceCount = m_scrollBarV->getPosition();
	advanceCount = std::max(0, advanceCount);
	advanceCount = std::min(advanceCount, int32_t(m_logFiltered.size()));

	const int32_t row = advanceCount + event->getPosition().y / pixel(m_itemHeight);
	selectLine(row);
}

void LogList::eventMouseDoubleClick(MouseDoubleClickEvent* event)
{
	if (m_selectedEntry < 0)
		return;

	const auto& entry = m_logFiltered[m_selectedEntry];
	if (entry.symbolId.isNotNull())
	{
		LogActivateEvent event(this, entry.symbolId);
		raiseEvent(&event);
	}
}

void LogList::eventMouseWheel(MouseWheelEvent* event)
{
	int32_t position = m_scrollBarV->getPosition();
	position -= event->getRotation() * 4;
	m_scrollBarV->setPosition(position);
	update();
}
    
void LogList::eventTimer(TimerEvent* event)
{
#if defined(__APPLE__)
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_pendingLock);
	if (!m_pending.empty())
		update();
#endif
}

void LogList::eventScroll(ScrollEvent* event)
{
	update();
}

}
