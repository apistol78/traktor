/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Llm/App/ChatView.h"

#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/Events/MouseWheelEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Events/ScrollEvent.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/ScrollBar.h"
#include "Ui/StyleSheet.h"

#include <algorithm>

namespace traktor::llm
{
namespace
{

const ui::Unit c_margin = 12_ut;
const ui::Unit c_bubblePaddingX = 12_ut;
const ui::Unit c_bubblePaddingY = 8_ut;
const ui::Unit c_bubbleSpacing = 10_ut;
const ui::Unit c_bubbleRadius = 10_ut;
const ui::Unit c_lineSpacing = 4_ut;
const int32_t c_bubbleWidthPercent = 78;
const int32_t c_scrollStep = 48;

/*! Break \a text into lines that each fit \a maximumWidth.
 *
 * Widths accumulate one character at a time rather than by re-measuring a
 * growing substring, so wrapping a message costs time proportional to its
 * length; the streaming message is re-wrapped on every token that arrives.
 */
void wrapText(const std::wstring& text, int32_t maximumWidth, const ui::FontMetric& fontMetric, AlignedVector< std::wstring >& outLines)
{
	size_t paragraphStart = 0;
	for (;;)
	{
		const size_t paragraphEnd = text.find(L'\n', paragraphStart);
		const std::wstring paragraph = text.substr(paragraphStart, (paragraphEnd == std::wstring::npos) ? std::wstring::npos : paragraphEnd - paragraphStart);

		if (paragraph.empty())
			outLines.push_back(L"");

		size_t i = 0;
		while (i < paragraph.size())
		{
			size_t fit = 0;
			size_t lastBreak = 0;
			int32_t width = 0;

			while (i + fit < paragraph.size())
			{
				const wchar_t ch = paragraph[i + fit];
				const wchar_t next = (i + fit + 1 < paragraph.size()) ? paragraph[i + fit + 1] : 0;
				const int32_t advance = fontMetric.getAdvance(ch, next);

				// Always take at least one character, or a narrow view would
				// never make progress.
				if (width + advance > maximumWidth && fit > 0)
					break;

				width += advance;
				++fit;

				// Remember where a space would let us break cleanly.
				if (ch == L' ')
					lastBreak = fit;
			}

			size_t take = fit;
			if (i + fit < paragraph.size() && lastBreak > 0)
				take = lastBreak;

			outLines.push_back(paragraph.substr(i, take));
			i += take;
		}

		if (paragraphEnd == std::wstring::npos)
			break;

		paragraphStart = paragraphEnd + 1;
		if (paragraphStart >= text.size())
		{
			// Trailing newline; keep the empty line it implies.
			outLines.push_back(L"");
			break;
		}
	}

	if (outLines.empty())
		outLines.push_back(L"");
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.llm.ChatView", ChatView, ui::Widget)

bool ChatView::create(ui::Widget* parent)
{
	if (!ui::Widget::create(parent, ui::WsDoubleBuffer | ui::WsFocus))
		return false;

	addEventHandler< ui::PaintEvent >(this, &ChatView::eventPaint);
	addEventHandler< ui::SizeEvent >(this, &ChatView::eventSize);
	addEventHandler< ui::MouseWheelEvent >(this, &ChatView::eventMouseWheel);

	m_scrollBar = new ui::ScrollBar();
	if (!m_scrollBar->create(this, ui::ScrollBar::WsVertical))
		return false;

	m_scrollBar->addEventHandler< ui::ScrollEvent >(this, &ChatView::eventScroll);
	return true;
}

int32_t ChatView::addMessage(Speaker speaker, const std::wstring& text)
{
	Message& message = m_messages.push_back();
	message.speaker = speaker;
	message.text = text;

	m_followTail = true;

	layout();
	update();

	return (int32_t)m_messages.size() - 1;
}

void ChatView::setMessageText(int32_t index, const std::wstring& text)
{
	if (index < 0 || index >= (int32_t)m_messages.size())
		return;

	if (m_messages[index].text == text)
		return;

	m_messages[index].text = text;
	m_messages[index].layoutWidth = -1;

	layout();
	update();
}

void ChatView::removeAllMessages()
{
	m_messages.clear();
	m_contentHeight = 0;
	m_followTail = true;

	layout();
	update();
}

Ref< ui::StyleSheet > ChatView::createStyleSheet()
{
	Ref< ui::StyleSheet > ss = new ui::StyleSheet();

	ss->setColor(L"traktor.llm.ChatView", L"background-color", Color4ub(248, 248, 250));
	ss->setColor(L"traktor.llm.ChatView", L"color", Color4ub(24, 24, 28));
	ss->setColor(L"traktor.llm.ChatView", L"background-color-user", Color4ub(0, 122, 204));
	ss->setColor(L"traktor.llm.ChatView", L"color-user", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.llm.ChatView", L"background-color-assistant", Color4ub(255, 255, 255));
	ss->setColor(L"traktor.llm.ChatView", L"color-assistant", Color4ub(24, 24, 28));
	ss->setColor(L"traktor.llm.ChatView", L"background-color-notice", Color4ub(252, 243, 220));
	ss->setColor(L"traktor.llm.ChatView", L"color-notice", Color4ub(120, 84, 0));

	return ss;
}

void ChatView::layout()
{
	const ui::Rect inner = getInnerRect();
	const int32_t scrollWidth = m_scrollBar->getPreferredSize(inner.getSize()).cx;
	const int32_t width = inner.getWidth() - scrollWidth;


	const int32_t maximumWidth = std::max(pixel(64_ut), (width - 2 * pixel(c_margin)) * c_bubbleWidthPercent / 100 - 2 * pixel(c_bubblePaddingX));
	const ui::FontMetric fontMetric = getFontMetric();
	const int32_t lineHeight = fontMetric.getHeight() + pixel(c_lineSpacing);

	int32_t top = pixel(c_margin);
	for (auto& message : m_messages)
	{
		// Only the message that changed, and only a resize, needs re-wrapping;
		// the rest just move.
		if (message.layoutWidth != maximumWidth)
		{
			message.lines.resize(0);
			wrapText(message.text, maximumWidth, fontMetric, message.lines);

			message.width = 0;
			for (const auto& line : message.lines)
				message.width = std::max(message.width, fontMetric.getExtent(line).cx);

			message.layoutWidth = maximumWidth;
		}

		message.top = top;
		message.height = (int32_t)message.lines.size() * lineHeight + 2 * pixel(c_bubblePaddingY);

		top += message.height + pixel(c_bubbleSpacing);
	}

	m_contentHeight = top - pixel(c_bubbleSpacing) + pixel(c_margin);

	updateScrollBar();

	if (m_followTail)
		scrollToTail();
}

void ChatView::updateScrollBar()
{
	const ui::Rect inner = getInnerRect();

	// ScrollBar measures range and page in the same unit and scrolls over
	// range - page, so it wants the whole content and the visible slice, both
	// in pixels. Handing it the overflow instead leaves nothing to scroll:
	// the slider fills its track and every position clamps to zero.
	//
	// Page first, because setRange is what re-clamps the position and it uses
	// whichever page is current when it runs.
	m_scrollBar->setPage(std::max(1, inner.getHeight()));
	m_scrollBar->setRange(std::max(0, m_contentHeight));

	m_scrollBar->update();
}

void ChatView::scrollToTail()
{
	const ui::Rect inner = getInnerRect();
	m_scrollBar->setPosition(std::max(0, m_contentHeight - inner.getHeight()));
	m_scrollBar->update();
}

void ChatView::eventPaint(ui::PaintEvent* event)
{
	ui::Canvas& canvas = event->getCanvas();
	const ui::StyleSheet* ss = getStyleSheet();
	const ui::Rect inner = getInnerRect();

	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(inner);

	const int32_t scrollWidth = m_scrollBar->getPreferredSize(inner.getSize()).cx;
	const int32_t width = inner.getWidth() - scrollWidth;
	const int32_t scroll = m_scrollBar->getPosition();

	const ui::FontMetric fontMetric = getFontMetric();
	const int32_t lineHeight = fontMetric.getHeight() + pixel(c_lineSpacing);

	for (const auto& message : m_messages)
	{
		const int32_t top = message.top - scroll;
		if (top + message.height < 0)
			continue;
		if (top > inner.getHeight())
			break;

		// The bubble is only as wide as its widest line, measured at layout.
		const int32_t bubbleWidth = message.width + 2 * pixel(c_bubblePaddingX);
		const bool right = (message.speaker == Speaker::User);
		const int32_t left = right ? (width - pixel(c_margin) - bubbleWidth) : pixel(c_margin);

		const wchar_t* backgroundKey = L"background-color-assistant";
		const wchar_t* colorKey = L"color-assistant";
		if (message.speaker == Speaker::User)
		{
			backgroundKey = L"background-color-user";
			colorKey = L"color-user";
		}
		else if (message.speaker == Speaker::Notice)
		{
			backgroundKey = L"background-color-notice";
			colorKey = L"color-notice";
		}

		const ui::Rect bubble(ui::Point(left, top), ui::Size(bubbleWidth, message.height));

		canvas.setBackground(ss->getColor(this, backgroundKey));
		canvas.fillRoundRect(bubble, pixel(c_bubbleRadius));

		canvas.setForeground(ss->getColor(this, colorKey));

		int32_t y = top + pixel(c_bubblePaddingY);
		for (const auto& line : message.lines)
		{
			canvas.drawText(ui::Point(left + pixel(c_bubblePaddingX), y), line);
			y += lineHeight;
		}
	}

	event->consume();
}

void ChatView::eventSize(ui::SizeEvent* event)
{
	const ui::Rect inner = getInnerRect();
	const int32_t width = m_scrollBar->getPreferredSize(inner.getSize()).cx;

	m_scrollBar->setRect(ui::Rect(ui::Point(inner.getWidth() - width, 0), ui::Size(width, inner.getHeight())));

	layout();
}

void ChatView::eventScroll(ui::ScrollEvent* event)
{
	// Only stay pinned to the newest message while the view is at the end.
	const ui::Rect inner = getInnerRect();
	m_followTail = (m_scrollBar->getPosition() >= m_contentHeight - inner.getHeight());

	update();
}

void ChatView::eventMouseWheel(ui::MouseWheelEvent* event)
{
	const ui::Rect inner = getInnerRect();
	const int32_t range = std::max(0, m_contentHeight - inner.getHeight());

	int32_t position = m_scrollBar->getPosition() - event->getRotation() * pixel(ui::Unit(c_scrollStep));
	position = std::max(0, std::min(position, range));

	m_scrollBar->setPosition(position);
	m_scrollBar->update();

	m_followTail = (position >= range);
	update();
}

}
