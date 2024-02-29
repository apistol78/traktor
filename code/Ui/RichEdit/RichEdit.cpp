/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cwctype>
#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Clipboard.h"
#include "Ui/StyleSheet.h"
#include "Ui/ScrollBar.h"
#include "Ui/RichEdit/CaretEvent.h"
#include "Ui/RichEdit/RichEdit.h"
#include "Ui/RichEdit/SearchControl.h"
#include "Ui/RichEdit/SearchEvent.h"

namespace traktor::ui
{
	namespace
	{

const int32_t c_lineMarginMin = 40;
const Unit c_iconSize = 16_ut;
const int32_t c_scrollHSteps = 10;

#if defined(__APPLE__)
const Unit c_fontHeightMargin = 4_ut;
#else
const Unit c_fontHeightMargin = 1_ut;
#endif

bool isWordSeparator(wchar_t ch)
{
	return !(std::iswalnum(ch) || ch == L'_');
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.RichEdit", RichEdit, Widget)

RichEdit::RichEdit()
:	m_lineMargin(c_lineMarginMin)
{
}

bool RichEdit::create(Widget* parent, const std::wstring& text, int32_t style)
{
	if (!Widget::create(parent, style | WsWantAllInput | WsDoubleBuffer))
		return false;

	m_clipboard = bool((style & WsNoClipboard) == 0);

	addEventHandler< KeyDownEvent >(this, &RichEdit::eventKeyDown);
	addEventHandler< KeyEvent >(this, &RichEdit::eventKey);
	addEventHandler< MouseButtonDownEvent >(this, &RichEdit::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &RichEdit::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &RichEdit::eventMouseMove);
	addEventHandler< MouseDoubleClickEvent >(this, &RichEdit::eventDoubleClick);
	addEventHandler< MouseWheelEvent >(this, &RichEdit::eventMouseWheel);
	addEventHandler< PaintEvent >(this, &RichEdit::eventPaint);
	addEventHandler< SizeEvent >(this, &RichEdit::eventSize);
	addEventHandler< TimerEvent >(this, &RichEdit::eventTimer);

	// Create scrollbars.
	m_scrollBarV = new ScrollBar();
	m_scrollBarH = new ScrollBar();
	if (!m_scrollBarV->create(this, ScrollBar::WsVertical))
		return false;
	if (!m_scrollBarH->create(this, ScrollBar::WsHorizontal))
		return false;

	m_scrollBarV->addEventHandler< ScrollEvent >(this, &RichEdit::eventScroll);
	m_scrollBarH->addEventHandler< ScrollEvent >(this, &RichEdit::eventScroll);

	// Create search control.
	m_searchControl = new SearchControl();
	m_searchControl->create(this);
	m_searchControl->hide();
	m_searchControl->addEventHandler< SearchEvent >(this, &RichEdit::eventSearch);

	TextAttribute txAttrib;
	txAttrib.textColor = Color4ub(0, 0, 0);
	txAttrib.bold = false;
	txAttrib.italic = false;
	txAttrib.underline = false;
	m_textAttributes.push_back(txAttrib);

	BackgroundAttribute bgAttrib;
	bgAttrib.backColor = Color4ub(255, 255, 255);
	m_backgroundAttributes.push_back(bgAttrib);

	const ui::StyleSheet* ss = getStyleSheet();
	m_foundLineAttribute = addBackgroundAttribute(ss->getColor(this, L"background-found-line"));

	setText(text);
	startTimer(500);
	return true;
}

void RichEdit::setText(const std::wstring& text)
{
	m_text.clear();
	m_lines.clear();

	if (!text.empty())
	{
		size_t i = 0;
		while (i < text.length())
		{
			const size_t j = text.find(L'\n', i);
			const std::wstring ln = (j != text.npos) ? text.substr(i, j - i) : text.substr(i);

			Line& line = m_lines.push_back();
			line.start = (int32_t)m_text.size();
			line.stop = line.start + (int32_t)ln.length();

			for (auto ch : ln)
				m_text.push_back(Character(ch));

			m_text.push_back(Character(L'\n'));

			if (j != text.npos)
			{
				i = j + 1;
				if (i < text.length() && text[i] == L'\r')
					++i;
			}
			else
				break;
		}
	}
	else
	{
		Line& line = m_lines.push_back();
		line.start = 0;
		line.stop = 0;
		m_text.push_back(Character(L'\n'));
	}

	m_selectionStart =
	m_selectionStop = -1;

	const int32_t lastOffset = int32_t(m_text.size());
	if (m_caret >= lastOffset)
	{
		m_caret = lastOffset;

		CaretEvent caretEvent(this);
		raiseEvent(&caretEvent);
	}

	contentModified();
	updateCharacterWidths();
	updateScrollBars();

	santiyCheck();
}

std::wstring RichEdit::getText() const
{
	if (!m_text.empty())
	{
		AlignedVector< wchar_t > text(m_text.size());
		for (size_t i = 0; i < m_text.size(); ++i)
			text[i] = m_text[i].ch;
		return std::wstring(text.begin(), text.end());
	}
	else
		return L"";
}

std::wstring RichEdit::getText(std::function< std::wstring (wchar_t) > cfn, std::function< std::wstring (const ISpecialCharacter*) > scfn) const
{
	StringOutputStream ss;
	for (size_t i = 0; i < m_text.size(); ++i)
	{
		auto j = m_specialCharacters.find(m_text[i].ch);
		if (j == m_specialCharacters.end())
			ss << cfn(m_text[i].ch);
		else
			ss << scfn(j->second);
	}
	return ss.str();
}

void RichEdit::setFont(const Font& font)
{
	Widget::setFont(font);

	for (auto& ch : m_text)
		ch.width = 0;

	updateCharacterWidths();
}

int32_t RichEdit::addTextAttribute(const ColorReference& textColor, bool bold, bool italic, bool underline)
{
	TextAttribute& attr = m_textAttributes.push_back();
	attr.textColor = textColor;
	attr.bold = bold;
	attr.italic = italic;
	attr.underline = underline;
	return (int32_t)(m_textAttributes.size() - 1);
}

int32_t RichEdit::addBackgroundAttribute(const ColorReference& backColor)
{
	BackgroundAttribute& attr = m_backgroundAttributes.push_back();
	attr.backColor = backColor;
	return (int32_t)(m_backgroundAttributes.size() - 1);
}

void RichEdit::setTextAttribute(int32_t start, int32_t length, int32_t attribute)
{
	if (attribute < 0 || attribute >= int32_t(m_textAttributes.size()))
		attribute = 0;

	if (start < 0)
		start = 0;

	if (start + length >= int32_t(m_text.size()))
	{
		length = int32_t(m_text.size()) - start;
		if (length < 0)
			return;
	}

	for (int32_t i = start; i < start + length; ++i)
		m_text[i].tai = uint16_t(attribute);

	update();
}

void RichEdit::setBackgroundAttribute(int32_t start, int32_t length, int32_t attribute)
{
	if (attribute < 0 || attribute >= int32_t(m_backgroundAttributes.size()))
		attribute = 0;

	if (start < 0)
		start = 0;

	if (start + length >= int32_t(m_text.size()))
	{
		length = int32_t(m_text.size()) - start;
		if (length < 0)
			return;
	}

	for (int32_t i = start; i < start + length; ++i)
		m_text[i].bgai = uint16_t(attribute);

	update();
}

void RichEdit::setBackgroundAttribute(int32_t line, int32_t attribute)
{
	if (line >= 0 && line < int32_t(m_lines.size()))
	{
		m_lines[line].attrib = attribute;
		update();
	}
	santiyCheck();
}

void RichEdit::setAttributes(int32_t start, int32_t length, int32_t textAttribute, int32_t backgroundAttribute)
{
	setTextAttribute(start, length, textAttribute);
	setBackgroundAttribute(start, length, backgroundAttribute);
}

int32_t RichEdit::addImage(IBitmap* image, uint32_t imageCount)
{
	T_ASSERT(image);
	T_ASSERT(imageCount > 0);

	if (m_image)
	{
		const uint32_t width = m_image->getSize(this).cx + image->getSize(this).cx;
		const uint32_t height = std::max(m_image->getSize(this).cy, image->getSize(this).cy);

		Ref< ui::Bitmap > newImage = new ui::Bitmap(width, height);
		newImage->copyImage(m_image->getImage(this));
		newImage->copySubImage(image->getImage(this), Rect(Point(0, 0), image->getSize(this)), Point(m_image->getSize(this).cx, 0));
		m_image = newImage;
	}
	else
	{
		m_image = image;
		m_imageWidth = std::max< uint32_t >(m_imageWidth, m_image->getSize(this).cx / imageCount);
		m_imageHeight = std::max< uint32_t >(m_imageHeight, m_image->getSize(this).cy);
	}

	const uint32_t imageBase = m_imageCount;
	m_imageCount += imageCount;

	return imageBase;
}

void RichEdit::setImage(int32_t line, int32_t image)
{
	if (line >= 0 && line < int32_t(m_lines.size()))
		m_lines[line].image = image;
	santiyCheck();
}

wchar_t RichEdit::addSpecialCharacter(const ISpecialCharacter* specialCharacter)
{
	const wchar_t ch = m_nextSpecialCharacter++;
	m_specialCharacters[ch] = specialCharacter;
	updateCharacterWidths();
	return ch;
}

void RichEdit::clear(bool attributes, bool images, bool content, bool specialCharacters)
{
	if (attributes)
	{
		for (auto& ch : m_text)
		{
			ch.tai = 0;
			ch.bgai = 0;
		}
		for (auto& line : m_lines)
			line.attrib = 0xffff;
	}

	if (content)
	{
		m_lines.clear();
		m_text.clear();
	}
	else if (images)
	{
		for (auto& line : m_lines)
			line.image = -1;
	}

	if (specialCharacters)
		m_specialCharacters.clear();

	contentModified();
	updateCharacterWidths();
	updateScrollBars();
	update();

	santiyCheck();
}

void RichEdit::deleteSelection()
{
	if (m_selectionStart >= 0)
		deleteCharacters();

	m_selectionStart =
	m_selectionStop = -1;

	contentModified();
	updateCharacterWidths();
	updateScrollBars();
	update();

	santiyCheck();
}

void RichEdit::insert(const std::wstring& text)
{
	if (text.empty())
		return;

	const std::wstring tmp = traktor::replaceAll(text, L"\r\n", L"\n");
	for (auto ch : tmp)
		insertCharacter(ch, false);

	CaretEvent caretEvent(this);
	raiseEvent(&caretEvent);

	ContentChangeEvent contentChangeEvent(this);
	raiseEvent(&contentChangeEvent);
}

int32_t RichEdit::getOffsetFromPosition(const Point& position)
{
	const uint32_t lineCount = (uint32_t)m_lines.size();
	if (lineCount == 0)
		return -1;

	const uint32_t lineOffset = m_scrollBarV->getPosition();
	const uint32_t lineHeight = getFontMetric().getHeight() + pixel(c_fontHeightMargin);

	const uint32_t line = lineOffset + position.y / lineHeight;
	if (line >= lineCount)
		return m_lines.back().stop;

	const Line& ln = m_lines[line];

	AlignedVector< int32_t > stops;

	int32_t x = 0;
	for (int32_t i = ln.start; i < ln.stop; ++i)
	{
		stops.push_back(x);
		x += m_text[i].width;
	}

	const int32_t lineWidth = x;
	const int32_t linePosition = std::max(position.x - m_lineMargin, 0) + m_lineOffsetH;
	if (linePosition < lineWidth)
	{
		for (int32_t i = int32_t(stops.size()) - 1; i >= 0; --i)
		{
			if (linePosition >= stops[i])
				return ln.start + i;
		}
	}

	return ln.stop;
}

int32_t RichEdit::getCaretOffset() const
{
	return m_caret;
}

int32_t RichEdit::getLineFromPosition(int32_t position)
{
	const uint32_t lineCount = (uint32_t)m_lines.size();
	const uint32_t lineOffset = m_scrollBarV->getPosition();
	const uint32_t lineHeight = getFontMetric().getHeight() + pixel(c_fontHeightMargin);
	const uint32_t line = lineOffset + position / lineHeight;
	return line < lineCount ? line : -1;
}

int32_t RichEdit::getLineFromOffset(int32_t offset) const
{
	for (int32_t i = 0; i < int32_t(m_lines.size()) - 1; ++i)
	{
		if (offset >= m_lines[i].start && offset <= m_lines[i].stop)
			return i;
	}
	return int32_t(m_lines.size()) - 1;
}

int32_t RichEdit::getLineCount() const
{
	return int32_t(m_lines.size());
}

int32_t RichEdit::getLineOffset(int32_t line) const
{
	return line < int32_t(m_lines.size()) ? m_lines[line].start : 0;
}

int32_t RichEdit::getLineLength(int32_t line) const
{
	return line < int32_t(m_lines.size()) ? (m_lines[line].stop - m_lines[line].start) : 0;
}

void RichEdit::setLine(int32_t line, const std::wstring& text)
{
	if (line >= int32_t(m_lines.size()))
		return;

	Line& ln = m_lines[line];

	m_text.erase(m_text.begin() + ln.start, m_text.begin() + ln.stop);
	for (uint32_t i = 0; i < text.size(); ++i)
		m_text.insert(m_text.begin() + ln.start + i, Character(text[i]));

	const int32_t adjust = int32_t(text.size()) - (ln.stop - ln.start);
	for (uint32_t i = line + 1; i < m_lines.size(); ++i)
	{
		m_lines[i].start += adjust;
		m_lines[i].stop += adjust;
	}

	ln.stop = ln.start + int32_t(text.size());

	updateCharacterWidths();
	santiyCheck();
}

std::wstring RichEdit::getLine(int32_t line) const
{
	if (line < int32_t(m_lines.size()))
	{
		AlignedVector< wchar_t > text;
		for (int32_t i = m_lines[line].start; i <= m_lines[line].stop; ++i)
			text.push_back(m_text[i].ch);
		return std::wstring(text.begin(), text.end());
	}
	else
		return L"";
}

void RichEdit::setLineData(int32_t line, Object* data)
{
	if (line < int32_t(m_lines.size()))
		m_lines[line].data = data;
	santiyCheck();
}

Object* RichEdit::getLineData(int32_t line) const
{
	return line < m_lines.size() ? m_lines[line].data : 0;
}

int32_t RichEdit::getSelectionStartOffset() const
{
	return m_selectionStart;
}

int32_t RichEdit::getSelectionStopOffset() const
{
	return m_selectionStop;
}

std::wstring RichEdit::getSelectedText() const
{
	if (m_selectionStart < 0 || m_text.empty())
		return L"";

	AlignedVector< wchar_t > text;
	text.reserve(m_selectionStop - m_selectionStart + 1);

	for (int32_t i = m_selectionStart; i < m_selectionStop; ++i)
		text.push_back(m_text[i].ch);

	return std::wstring(text.begin(), text.end());
}

std::wstring RichEdit::getSelectedText(std::function< std::wstring (wchar_t) > cfn, std::function< std::wstring (const ISpecialCharacter*) > scfn) const
{
	if (m_selectionStart < 0 || m_text.empty())
		return L"";

	StringOutputStream ss;
	for (int32_t i = m_selectionStart; i < m_selectionStop; ++i)
	{
		auto j = m_specialCharacters.find(m_text[i].ch);
		if (j == m_specialCharacters.end())
			ss << cfn(m_text[i].ch);
		else
			ss << scfn(j->second);
	}
	return ss.str();
}

bool RichEdit::scrollToLine(int32_t line)
{
	m_scrollBarV->setPosition(line);
	m_scrollBarV->update();
	update();
	return true;
}

int32_t RichEdit::getScrollLine() const
{
	return m_scrollBarV->getPosition();
}

bool RichEdit::showLine(int32_t line)
{
	const Rect rc = getEditRect();

	const int32_t lineHeight = getFontMetric().getHeight() + pixel(c_fontHeightMargin);
	const int32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;
	const int32_t top = m_scrollBarV->getPosition();

	if (line >= top && line < top + pageLines)
		return true;

	m_scrollBarV->setPosition(std::max< int32_t >(line - pageLines / 3, 0));
	m_scrollBarV->update();

	update();
	return true;
}

void RichEdit::placeCaret(int32_t offset, bool showLine)
{
	m_caret = offset;

	CaretEvent caretEvent(this);
	raiseEvent(&caretEvent);

	if (showLine)
	{
		const int32_t line = getLineFromOffset(offset);
		this->showLine(line);
	}
	else
		update();
}

void RichEdit::selectAll()
{
	if (!m_text.empty())
	{
		m_selectionStart = 0;
		m_selectionStop = int32_t(m_text.size());
	}
	update();
}

void RichEdit::unselect()
{
	m_selectionStart = -1;
	m_selectionStop = -1;
	update();
}

bool RichEdit::copy()
{
	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (!clipboard)
		return false;

	const std::wstring selectedText = getSelectedText();
	return clipboard->setText(selectedText);
}

bool RichEdit::cut()
{
	if (!copy())
		return false;

	deleteCharacters();
	return true;
}

bool RichEdit::paste()
{
	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (!clipboard)
		return false;

	if (!hasFocus())
		return false;

	const std::wstring pasteText = clipboard->getText();
	if (m_selectionStart >= 0)
		deleteCharacters();
	insert(pasteText);

	return true;
}

bool RichEdit::find()
{
	// Initialize needle with selection; only if selection is single line.
	const int32_t startLine = getLineFromOffset(getSelectionStartOffset());
	const int32_t stopLine = getLineFromOffset(getSelectionStopOffset());
	if (startLine == stopLine)
	{
		const std::wstring needle = getSelectedText();
		m_searchControl->setNeedle(needle);
	}

	// Show and focus search control.
	m_searchControl->show();
	m_searchControl->setFocus();
	return true;
}

bool RichEdit::findNext()
{
	FindResult result;

	const std::wstring needle = m_searchControl->getNeedle();
	const bool caseSensitive = m_searchControl->caseSensitive();
	const bool wholeWord = m_searchControl->wholeWord();
	const bool wildcard = m_searchControl->wildcard();

	m_searchControl->setAnyMatchingHint(false);

	const int32_t caretLine = getLineFromOffset(getCaretOffset());
	if (!findNextLine(caretLine, needle, caseSensitive, wholeWord, wildcard, result))
		return false;

	showLine(result.line);
	placeCaret(getLineOffset(result.line) + result.offset);
	for (int32_t i = 0; i < getLineCount(); ++i)
		setBackgroundAttribute(i, (i == result.line) ? m_foundLineAttribute : 0xffff);
	setFocus();

	m_searchControl->setAnyMatchingHint(true);
	return true;
}

bool RichEdit::replace()
{
	return false;
}

bool RichEdit::replaceAll()
{
	return false;
}

Rect RichEdit::getEditRect() const
{
	Rect rc = getInnerRect();
	const Size sz = rc.getSize();
	rc.right -= m_scrollBarV->getPreferredSize(sz).cx;
	if (m_scrollBarH->isVisible(false))
		rc.bottom -= m_scrollBarH->getPreferredSize(sz).cy;
	return rc;
}

int32_t RichEdit::getMarginWidth() const
{
	return m_lineMargin;
}

void RichEdit::contentModified()
{
}

void RichEdit::updateScrollBars()
{
	Rect rc = getEditRect();

	const uint32_t lineCount = uint32_t(m_lines.size());
	const uint32_t lineHeight = getFontMetric().getHeight() + pixel(c_fontHeightMargin);
	const uint32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

	m_scrollBarV->setRange(lineCount + pageLines);
	m_scrollBarV->setPage(pageLines);

	if (m_widestLineWidth >= rc.getWidth() - m_lineMargin)
	{
		m_scrollBarH->setRange(m_widestLineWidth / c_scrollHSteps);
		m_scrollBarH->setPage(c_scrollHSteps);
		m_scrollBarH->setVisible(true);
	}
	else
	{
		m_scrollBarH->setPosition(0);
		m_scrollBarH->setVisible(false);
	}

	update();
}

void RichEdit::updateCharacterWidths()
{
	int32_t blw = getFontMetric().getAdvance(L' ', 0);
	if (blw <= 0)
		blw = 1;

	m_widestLineWidth = 0;
	for (const auto& line : m_lines)
	{
		int32_t x = 0;
		for (int32_t j = line.start; j < line.stop; ++j)
		{
			Character& c = m_text[j];
			if (c.width == 0)
			{
				if (c.ch != L'\t')
				{
					auto k = m_specialCharacters.find(c.ch);
					if (k == m_specialCharacters.end())
					{
						const int32_t chw = getFontMetric().getAdvance(c.ch, 0);
						c.width = chw;
					}
					else
					{
						c.width = k->second->measureWidth(this);
					}
				}
				else
				{
					const int32_t nx = alignUp(x + 4 * blw, 4 * blw);
					c.width = nx - x;
				}
			}
			x += c.width;
		}
		m_widestLineWidth = std::max(m_widestLineWidth, x);
	}
}

void RichEdit::deleteCharacters()
{
	int32_t start = m_caret;
	int32_t stop = m_caret;

	if (m_selectionStart < m_selectionStop)
	{
		start = m_selectionStart;
		stop = m_selectionStop - 1;
	}

	T_FATAL_ASSERT (start <= stop);

	if (start >= m_text.size() - 1)
		return;

	// Naive implementation, delete each character
	// individually as we don't have to intersect selection with lines etc.

	for (int32_t i = stop; i >= start; --i)
	{
		for (uint32_t j = 0; j < m_lines.size(); )
		{
			Line& ln = m_lines[j];
			if (i >= ln.start && i <= ln.stop)
			{
				if (i == ln.stop)
				{
					if (j < m_lines.size() - 1)
					{
						Line& nx = m_lines[j + 1];
						ln.stop = nx.stop - 1;
						m_lines.erase(m_lines.begin() + j + 1);
					}
					++j;
				}
				else if (ln.start < ln.stop)
				{
					ln.stop--;
					++j;
				}
				else
					m_lines.erase(m_lines.begin() + j);
			}
			else if (i <= ln.start)
			{
				ln.start--;
				ln.stop--;
				++j;
			}
			else
				++j;
		}
	}

	m_text.erase(m_text.begin() + start, m_text.begin() + stop + 1);

	m_selectionStart = -1;
	m_selectionStop = -1;

	m_caret = start;

	CaretEvent caretEvent(this);
	raiseEvent(&caretEvent);

	updateCharacterWidths();
	santiyCheck();

	ContentChangeEvent contentChangeEvent(this);
	raiseEvent(&contentChangeEvent);
}

void RichEdit::insertCharacter(wchar_t ch, bool issueEvents)
{
	if (ch == L'\n' || ch == L'\r')
	{
		if (m_selectionStart >= 0)
			deleteCharacters();

		insertAt(m_caret++, L'\n');

		if (issueEvents)
		{
			CaretEvent caretEvent(this);
			raiseEvent(&caretEvent);

			ContentChangeEvent contentChangeEvent(this);
			raiseEvent(&contentChangeEvent);
		}
	}
	else if (ch == '\t')
	{
		if (m_selectionStart >= 0)
		{
			const int32_t indentFromLine = getLineFromOffset(m_selectionStart);
			const int32_t indentToLine = getLineFromOffset(m_selectionStop - 1);

			if (indentFromLine <= indentToLine)
			{
				for (int32_t i = indentFromLine; i <= indentToLine; ++i)
				{
					const int32_t offset = getLineOffset(i);
					insertAt(offset, L'\t');
				}

				m_selectionStart = getLineOffset(indentFromLine);
				m_selectionStop = getLineOffset(indentToLine) + getLineLength(indentToLine);

				m_caret++;

				if (issueEvents)
				{
					ContentChangeEvent contentChangeEvent(this);
					raiseEvent(&contentChangeEvent);
				}

				return;
			}
			else
				deleteCharacters();
		}

		insertAt(m_caret++, L'\t');

		if (issueEvents)
		{
			CaretEvent caretEvent(this);
			raiseEvent(&caretEvent);

			ContentChangeEvent contentChangeEvent(this);
			raiseEvent(&contentChangeEvent);
		}
	}
	else if (ch >= 32)
	{
		if (m_selectionStart >= 0)
			deleteCharacters();

		insertAt(m_caret++, ch);

		if (issueEvents)
		{
			CaretEvent caretEvent(this);
			raiseEvent(&caretEvent);

			ContentChangeEvent contentChangeEvent(this);
			raiseEvent(&contentChangeEvent);
		}
	}
}

void RichEdit::insertAt(int32_t offset, wchar_t ch)
{
	m_text.insert(m_text.begin() + offset, Character(ch));

	if (ch == L'\n' || ch == L'\r')
	{
		for (auto i = m_lines.begin(); i != m_lines.end(); ++i)
		{
			if (offset >= i->start && offset <= i->stop)
			{
				Line line;
				line.start = i->start;
				line.stop = offset;
				line.data = i->data;
				i->start = offset + 1;
				i->stop++;
				i->data = 0;
				i = m_lines.insert(i, line) + 1;
			}
			else if (i->start > offset)
			{
				i->start++;
				i->stop++;
			}
		}
	}
	else
	{
		for (auto& line : m_lines)
		{
			if (offset >= line.start && offset <= line.stop)
				line.stop++;
			else if (offset < line.start)
			{
				line.start++;
				line.stop++;
			}
		}
	}

	contentModified();
	updateCharacterWidths();
	santiyCheck();
}

void RichEdit::scrollToCaret()
{
	const int32_t caretLine = getLineFromOffset(m_caret);
	const Rect rc = getEditRect();

	if (m_scrollBarV->isVisible(false))
	{
		const int32_t lineHeight = getFontMetric().getHeight() + pixel(c_fontHeightMargin);
		const int32_t pageLines = rc.getHeight() / lineHeight;

		const int32_t top = m_scrollBarV->getPosition();

		if (caretLine < top)
		{
			m_scrollBarV->setPosition(caretLine);
			m_scrollBarV->update();
			update();
		}
		else if (caretLine >= top + pageLines)
		{
			m_scrollBarV->setPosition(caretLine - pageLines + 1);
			m_scrollBarV->update();
			update();
		}
	}

	if (m_scrollBarH->isVisible(false))
	{
		const int32_t width = rc.getWidth() - m_lineMargin;
		const int32_t lineOffset = getLineOffset(caretLine);

		int32_t x = 0;
		for (int32_t i = lineOffset; i < m_caret; ++i)
			x += m_text[i].width;
		
		const int32_t left = m_scrollBarH->getPosition() * c_scrollHSteps;
		if (x < left)
		{
			m_scrollBarH->setPosition(x / c_scrollHSteps);
			m_scrollBarH->update();
			update();
		}
		else if (x > left + width)
		{
			m_scrollBarH->setPosition((x - width + c_scrollHSteps - 1) / c_scrollHSteps);
			m_scrollBarH->update();
			update();
		}
	}
}

void RichEdit::santiyCheck()
{
	int32_t last = 0;
	for (uint32_t i = 0; i < m_lines.size(); ++i)
	{
		T_FATAL_ASSERT (m_lines[i].start == last);
		T_FATAL_ASSERT (m_lines[i].start <= m_lines[i].stop);
		last = m_lines[i].stop + 1;
	}
	T_FATAL_ASSERT (last == m_text.size());
}

void RichEdit::updateFindPreview() const
{
	FindResult result;

	const std::wstring needle = m_searchControl->getNeedle();
	const bool caseSensitive = m_searchControl->caseSensitive();
	const bool wholeWord = m_searchControl->wholeWord();
	const bool wildcard = m_searchControl->wildcard();

	m_searchControl->setAnyMatchingHint(
		findFirstLine(0, needle, caseSensitive, wholeWord, wildcard, result)
	);
}

bool RichEdit::findFirstLine(int32_t currentLine, const std::wstring& needle, bool caseSensitive, bool wholeWord, bool wildcard, FindResult& outResult) const
{
	const std::wstring ndl = caseSensitive ? needle : toLower(needle);
	int32_t line = currentLine;

	while (line < getLineCount())
	{
		const std::wstring text = caseSensitive ? getLine(line) : toLower(getLine(line));
		const size_t p = text.find(ndl);
		if (p != text.npos)
		{
			outResult.line = line;
			outResult.offset = (int32_t)p;
			return true;
		}
		++line;
	}

	if (line >= getLineCount())
	{
		line = 0;
		while (line < currentLine)
		{
			const std::wstring text = caseSensitive ? getLine(line) : toLower(getLine(line));
			const size_t p = text.find(ndl);
			if (p != text.npos)
			{
				outResult.line = line;
				outResult.offset = (int32_t)p;
				return true;
			}
			++line;
		}
	}

	return false;
}

bool RichEdit::findNextLine(int32_t currentLine, const std::wstring& needle, bool caseSensitive, bool wholeWord, bool wildcard, FindResult& outResult) const
{
	return findFirstLine(
		(currentLine + 1) % getLineCount(),
		needle,
		caseSensitive,
		wholeWord,
		wildcard,
		outResult
	);
}

void RichEdit::eventKeyDown(KeyDownEvent* event)
{
	const bool shift = ((event->getKeyState() & KsShift) != 0);
	const bool ctrl = ((event->getKeyState() & KsControl) != 0);
	const bool alt = ((event->getKeyState() & KsMenu) != 0);

	int32_t caret = m_caret;
	bool caretMovement = false;
	bool manualScrolled = false;
	bool contentChanged = false;

	switch (event->getVirtualKey())
	{
	case VkEscape:
		m_searchControl->hide();
		break;

	case VkUp:
		if (!ctrl && !alt)	// Move caret up.
		{
			for (uint32_t i = 1; i < m_lines.size(); ++i)
			{
				if (m_caret >= m_lines[i].start && m_caret <= m_lines[i].stop)
				{
					int32_t offset = m_caret - m_lines[i].start;
					offset = std::min(offset, m_lines[i - 1].stop - m_lines[i - 1].start);
					m_caret = m_lines[i - 1].start + offset;
					break;
				}
			}
			caretMovement = true;
		}
		else if (ctrl && !alt)	// Scroll up.
		{
			if (m_scrollBarV->isVisible(false))
			{
				const int32_t position = m_scrollBarV->getPosition();
				m_scrollBarV->setPosition(position - 1);
				m_scrollBarV->update();
				update();
				manualScrolled = true;
			}
		}
		else if (!ctrl && alt)	// Move line up.
		{
			for (uint32_t i = 1; i < m_lines.size(); ++i)
			{
				if (m_caret >= m_lines[i].start && m_caret <= m_lines[i].stop)
				{
					const Line ln0 = m_lines[i - 1];
					const Line ln1 = m_lines[i];

					AlignedVector< Character > tx0;
					for (int32_t i = ln0.start; i <= ln0.stop; ++i)
						tx0.push_back(m_text[i]);

					AlignedVector< Character > tx1;
					for (int32_t i = ln1.start; i <= ln1.stop; ++i)
						tx1.push_back(m_text[i]);

					const int32_t s = ln0.start + (ln1.stop - ln1.start) + 1;

					for (int32_t i = 0; i <= ln1.stop - ln1.start; ++i)
						m_text[ln0.start + i] = tx1[i];
					for (int32_t i = 0; i <= ln0.stop - ln0.start; ++i)
						m_text[s + i] = tx0[i];

					m_caret = m_lines[i - 1].start + (m_caret - m_lines[i].start);

					m_lines[i - 1].stop = s - 1;
					m_lines[i].start = s;
					break;
				}
			}
			caretMovement = true;
			contentChanged = true;
		}
		break;

	case VkDown:
		if (!ctrl && !alt)	// Move caret down.
		{
			for (uint32_t i = 0; i < m_lines.size() - 1; ++i)
			{
				if (m_caret >= m_lines[i].start && m_caret <= m_lines[i].stop)
				{
					int32_t offset = m_caret - m_lines[i].start;
					offset = std::min(offset, m_lines[i + 1].stop - m_lines[i + 1].start);
					m_caret = m_lines[i + 1].start + offset;
					break;
				}
			}
			caretMovement = true;
		}
		else if (ctrl && !alt)	// Scroll down.
		{
			if (m_scrollBarV->isVisible(false))
			{
				const int32_t position = m_scrollBarV->getPosition();
				m_scrollBarV->setPosition(position + 1);
				m_scrollBarV->update();
				update();
				manualScrolled = true;
			}
		}
		else if (!ctrl && alt)	// Move line down.
		{
			if (!m_lines.empty())
			{
				for (uint32_t i = 0; i < m_lines.size() - 1; ++i)
				{
					if (m_caret >= m_lines[i].start && m_caret <= m_lines[i].stop)
					{
						const Line ln0 = m_lines[i];
						const Line ln1 = m_lines[i + 1];

						AlignedVector< Character > tx0;
						for (int32_t i = ln0.start; i <= ln0.stop; ++i)
							tx0.push_back(m_text[i]);

						AlignedVector< Character > tx1;
						for (int32_t i = ln1.start; i <= ln1.stop; ++i)
							tx1.push_back(m_text[i]);

						const int32_t s = ln0.start + (ln1.stop - ln1.start) + 1;

						for (int32_t i = 0; i <= ln1.stop - ln1.start; ++i)
							m_text[ln0.start + i] = tx1[i];
						for (int32_t i = 0; i <= ln0.stop - ln0.start; ++i)
							m_text[s + i] = tx0[i];

						m_caret = s + (m_caret - m_lines[i].start);

						m_lines[i].stop = s - 1;
						m_lines[i + 1].start = s;
						break;
					}
				}
				caretMovement = true;
				contentChanged = true;
			}
		}
		break;

	case VkLeft:
		if (!ctrl && !alt)	// Move caret left.
		{
			if (m_caret > 0)
				--m_caret;
		}
		else if (ctrl && !alt)	// Move caret left to next word.
		{
			if (m_caret > 0)
				--m_caret;

			while (m_caret > 0)
			{
				if (isWordSeparator(m_text[m_caret - 1].ch))
					break;
				--m_caret;
			}
		}
		caretMovement = true;
		break;

	case VkRight:
		if (!ctrl && !alt)	// Move caret right.
		{
			if (m_caret < int32_t(m_text.size()) - 1)
				++m_caret;
		}
		else if (ctrl && !alt)	// Move caret right to previous word.
		{
			while (m_caret < int32_t(m_text.size()) - 1)
			{
				++m_caret;
				if (isWordSeparator(m_text[m_caret].ch))
					break;
			}
		}
		caretMovement = true;
		break;

	case VkHome:
		{
			if (!ctrl && !alt)
			{
				for (const auto& line : m_lines)
				{
					if (m_caret > line.start && m_caret <= line.stop)
					{
						m_caret = line.start;
						break;
					}
				}
			}
			else
				m_caret = 0;

			caretMovement = true;
		}
		break;

	case VkEnd:
		{
			if (!ctrl && !alt)
			{
				for (const auto& line : m_lines)
				{
					if (m_caret >= line.start && m_caret < line.stop)
					{
						m_caret = line.stop;
						break;
					}
				}
			}
			else
				m_caret = m_lines.back().stop;

			caretMovement = true;
		}
		break;

	case VkPageUp:
		// Move caret one page up.
		{
			const Rect rc = getEditRect();

			const int32_t lineHeight = getFontMetric().getHeight() + pixel(c_fontHeightMargin);
			const int32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

			for (int32_t i = 1; i < int32_t(m_lines.size()); ++i)
			{
				if (m_caret >= m_lines[i].start && m_caret <= m_lines[i].stop)
				{
					int32_t offset = m_caret - m_lines[i].start;
					const int32_t di = std::min(pageLines, i);
					offset = std::min(offset, m_lines[i - di].stop - m_lines[i - di].start);
					m_caret = m_lines[i - di].start + offset;
					break;
				}
			}
		}
		caretMovement = true;
		break;

	case VkPageDown:
		// Move caret one page down.
		{
			const Rect rc = getEditRect();

			const int32_t lineHeight = getFontMetric().getHeight() + pixel(c_fontHeightMargin);
			const int32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

			for (int32_t i = 0; i < int32_t(m_lines.size()) - 1; ++i)
			{
				if (m_caret >= m_lines[i].start && m_caret <= m_lines[i].stop)
				{
					int32_t offset = m_caret - m_lines[i].start;
					const int32_t di = std::min< int32_t >(pageLines, int32_t(m_lines.size()) - 1 - i);
					offset = std::min(offset, m_lines[i + di].stop - m_lines[i + di].start);
					m_caret = m_lines[i + di].start + offset;
					break;
				}
			}
		}
		caretMovement = true;
		break;

	case VkBackSpace:
		{
			if (m_selectionStart < 0 && m_caret > 0)
			{
				m_caret--;
				deleteCharacters();
			}
			else if (m_selectionStart >= 0)
				deleteCharacters();
		}
		break;

	case VkDelete:
		deleteCharacters();
		break;

	case VkShift:
		return;

	default:
		break;
	}

	// If caret moved while holding "shift" key then expand selection range.
	if (caretMovement && shift)
	{
		if (m_selectionStart < 0)
		{
			m_selectionStart = std::min(caret, m_caret);
			m_selectionStop = std::max(caret, m_caret);
		}
		else
		{
			const bool caretAtSelectionHead = bool(caret == m_selectionStart);
			const bool caretAtSelectionTail = bool(caret == m_selectionStop);

			if (caretAtSelectionHead && !caretAtSelectionTail)
				m_selectionStart = m_caret;
			else if (!caretAtSelectionHead && caretAtSelectionTail)
				m_selectionStop = m_caret;
			else
			{
				m_selectionStart = std::min(m_selectionStart, m_caret);
				m_selectionStop = std::max(m_selectionStop, m_caret);
			}

			if (m_selectionStart > m_selectionStop)
				std::swap(m_selectionStart, m_selectionStop);
		}

		CaretEvent caretEvent(this);
		raiseEvent(&caretEvent);
	}
	// If caret moved but no "shift" key held, discard selection.
	else if (caretMovement)
	{
		m_selectionStart =
		m_selectionStop = -1;

		CaretEvent caretEvent(this);
		raiseEvent(&caretEvent);
	}

	if (contentChanged)
	{
		ContentChangeEvent contentChangeEvent(this);
		raiseEvent(&contentChangeEvent);
	}

	updateScrollBars();

	if (!manualScrolled)
		scrollToCaret();

	update();
	santiyCheck();
}

void RichEdit::eventKey(KeyEvent* event)
{
	const int32_t ks = event->getKeyState();
	const bool ctrl = ((ks & KsControl) != 0);

	const wchar_t ch = event->getCharacter();
	const wchar_t uch = std::toupper(ch);
	
	if (ctrl && uch == L'C' && m_clipboard)
		copy();
	else if (ctrl && uch == L'V' && m_clipboard)
		paste();
	else if (ctrl && uch == L'X' && m_clipboard)
		cut();
	else if (ch != 8 && ch != 127)
		insertCharacter(ch, true);

	updateScrollBars();
	scrollToCaret();
	update();
	santiyCheck();
}

void RichEdit::eventButtonDown(MouseButtonDownEvent* event)
{
	const Point position = event->getPosition();
	if (position.x < m_lineMargin)
		return;

	setCursor(Cursor::IBeam);

	if (event->getButton() == MbtLeft)
	{
		const int32_t offset = getOffsetFromPosition(position);
		if (offset >= 0)
		{
			if ((event->getKeyState() & ui::KsShift) != 0)
			{
				m_fromCaret = m_caret;
				m_caret = offset;
				m_selectionStart = std::min(m_fromCaret, m_caret);
				m_selectionStop = std::max(m_fromCaret, m_caret);
			}
			else
			{
				m_caret = offset;
				m_fromCaret = offset;
				m_selectionStart = -1;
				m_selectionStop = -1;

				auto it = m_specialCharacters.find(m_text[m_caret].ch);
				if (it != m_specialCharacters.end())
					it->second->mouseButtonDown(event);
			}

			CaretEvent caretEvent(this);
			raiseEvent(&caretEvent);

			setCapture();
			update();
		}
	}
}

void RichEdit::eventButtonUp(MouseButtonUpEvent* event)
{
	setCursor(Cursor::IBeam);

	if (!hasCapture())
		return;

	auto i = m_specialCharacters.find(m_text[m_caret].ch);
	if (i != m_specialCharacters.end())
		i->second->mouseButtonUp(event);

	releaseCapture();
}

void RichEdit::eventMouseMove(MouseMoveEvent* event)
{
	const Point position = event->getPosition();
	if (position.x >= m_lineMargin)
		setCursor(Cursor::IBeam);
	else
		setCursor(Cursor::Arrow);

	if (!hasCapture())
		return;

	const int32_t offset = getOffsetFromPosition(position);
	if (offset >= 0)
	{
		m_caret = offset;

		if (offset != m_fromCaret)
		{
			m_selectionStart = std::min(m_fromCaret, m_caret);
			m_selectionStop = std::max(m_fromCaret, m_caret);
		}
		else
		{
			m_selectionStart = -1;
			m_selectionStop = -1;
		}

		CaretEvent caretEvent(this);
		raiseEvent(&caretEvent);

		update();
	}
}

void RichEdit::eventDoubleClick(MouseDoubleClickEvent* event)
{
	if (event->getButton() != MbtLeft)
		return;

	const Point position = event->getPosition();
	if (position.x < m_lineMargin)
		return;

	const int32_t offset = getOffsetFromPosition(position);
	if (offset >= 0)
	{
		if ((m_selectionStart = offset) > 0)
		{
			while (m_selectionStart > 0)
			{
				if (isWordSeparator(m_text[m_selectionStart - 1].ch))
					break;
				--m_selectionStart;
			}
		}
		m_selectionStop = offset;
		while (m_selectionStop < int32_t(m_text.size()) - 1)
		{
			++m_selectionStop;
			if (isWordSeparator(m_text[m_selectionStop].ch))
				break;
		}
		m_caret = m_selectionStop;

		auto i = m_specialCharacters.find(m_text[m_caret].ch);
		if (i != m_specialCharacters.end())
			i->second->mouseDoubleClick(event);

		CaretEvent caretEvent(this);
		raiseEvent(&caretEvent);

		update();
	}
}

void RichEdit::eventMouseWheel(MouseWheelEvent* event)
{
	if ((event->getKeyState() & KsControl) == 0)
	{
		if (m_scrollBarV->isVisible(false))
		{
			int32_t position = m_scrollBarV->getPosition();
			position -= event->getRotation() * 4;
			m_scrollBarV->setPosition(position);
			m_scrollBarV->update();
			update();
		}
	}
	else
	{
		Font font = getFont();
		const Unit newSize = font.getSize() + Unit(event->getRotation());
		if (newSize > 0_ut)
		{
			font.setSize(newSize);
			setFont(font);
		}
	}
}

void RichEdit::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	Rect innerRc = getInnerRect();
	Rect updateRc = event->getUpdateRect();
	const StyleSheet* ss = getStyleSheet();

	// Clear entire background.
	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(innerRc);

	const Size sz = innerRc.getSize();
	innerRc.right -= m_scrollBarV->getPreferredSize(sz).cx;
	if (m_scrollBarH->isVisible(false))
		innerRc.bottom -= m_scrollBarH->getPreferredSize(sz).cy;

	const uint32_t lineCount = uint32_t(m_lines.size());
	const uint32_t lineOffset = m_scrollBarV->getPosition();
	const uint32_t lineHeight = getFontMetric().getHeight() + pixel(c_fontHeightMargin);
	const uint32_t pageLines = (innerRc.getHeight() + lineHeight - 1) / lineHeight;

	// Calculate margin width from highest visible line number.
	m_lineMargin = pixel(c_iconSize) + canvas.getFontMetric().getExtent(toString(lineOffset + pageLines)).cx + pixel(2_ut);
	m_lineOffsetH = m_scrollBarH->isVisible(false) ? m_scrollBarH->getPosition() * c_scrollHSteps : 0;

	// Background
	{
		Rect marginRc(innerRc.left, innerRc.top, innerRc.left + m_lineMargin, innerRc.bottom);
		Rect iconsRc(innerRc.left, innerRc.top, innerRc.left + pixel(c_iconSize), innerRc.top + lineHeight);
		Rect lineRc(innerRc.left + pixel(c_iconSize), innerRc.top, innerRc.left + m_lineMargin, innerRc.top + lineHeight);

		canvas.setBackground(ss->getColor(this, L"background-color-margin"));
		canvas.fillRect(marginRc);

		canvas.setForeground(ss->getColor(this, L"color-margin"));
		for (uint32_t i = lineOffset; i < lineOffset + pageLines && i < lineCount; ++i)
		{
			canvas.drawText(lineRc, toString(i + 1), AnLeft, AnTop);

			if (m_lines[i].image >= 0)
			{
				canvas.drawBitmap(
					Point(iconsRc.left, iconsRc.top + (iconsRc.getHeight() - m_imageHeight) / 2),
					Point(m_lines[i].image * m_imageWidth, 0),
					Size(m_imageWidth, m_imageHeight),
					m_image,
					BlendMode::Alpha
				);
			}

			iconsRc = iconsRc.offset(0, lineHeight);
			lineRc = lineRc.offset(0, lineHeight);
		}
	}

	// Formatted text.
	{
		const bool showCaret = m_caretBlink && hasFocus();

		canvas.setClipRect(Rect(
			innerRc.left + m_lineMargin,
			innerRc.top,
			innerRc.right,
			innerRc.bottom
		));

		Rect lineRc(innerRc.left, innerRc.top, innerRc.right, innerRc.top + lineHeight);
		const uint32_t lineOffsetEnd = std::min(lineOffset + pageLines, lineCount);
		for (uint32_t i = lineOffset; i < lineOffsetEnd; ++i)
		{
			const Line& line = m_lines[i];

			// Draw line background attribute.
			if (line.attrib != 0xffff)
			{
				const BackgroundAttribute& bgAttrib = m_backgroundAttributes[line.attrib];
				canvas.setBackground(bgAttrib.backColor.resolve(ss));
				canvas.fillRect(lineRc);
			}

			Rect textRc = lineRc;
			int32_t x = 0;

			// Non-empty line; format print.
			for (int32_t j = line.start; j < line.stop; ++j)
			{
				const TextAttribute& txAttrib = m_textAttributes[m_text[j].tai];
				const BackgroundAttribute& bgAttrib = m_backgroundAttributes[m_text[j].bgai];

				// Draw caret.
				if (showCaret && m_caret == j)
				{
					textRc.left = m_lineMargin + 2 + x - pixel(1_ut) - m_lineOffsetH;
					textRc.right = textRc.left + pixel(1_ut);

					canvas.setBackground(ss->getColor(this, L"color-caret"));
					canvas.fillRect(textRc);
				}

				// Set attribute colors.
				bool solidBackground = false;
				if (j >= m_selectionStart && j < m_selectionStop)
				{
					canvas.setBackground(ss->getColor(this, L"background-color-selection"));
					canvas.setForeground(ss->getColor(this, L"color-selection"));
					solidBackground = true;
				}
				else
				{
					canvas.setForeground(txAttrib.textColor.resolve(ss));

					const Color4ub bg = bgAttrib.backColor.resolve(ss);
					if (bg.a != 0)
					{
						canvas.setBackground(bgAttrib.backColor.resolve(ss));
						solidBackground = true;
					}
				}

				// Draw characters.
				textRc.left = m_lineMargin + 2 + x - m_lineOffsetH;
				textRc.right = textRc.left + m_text[j].width;

				if (solidBackground)
					canvas.fillRect(textRc);

				auto k = m_specialCharacters.find(m_text[j].ch);
				if (k == m_specialCharacters.end())
				{
					if (m_text[j].ch != L'\t' && m_text[j].ch != L' ')
						canvas.drawText(textRc, std::wstring(1, m_text[j].ch), AnLeft, AnTop);
				}
				else
				{
					k->second->draw(canvas, textRc);
				}

				x += m_text[j].width;
			}

			// Special condition; caret at the very end of a line.
			if (showCaret && m_caret == line.stop)
			{
				textRc.left = m_lineMargin + 2 + x - m_lineOffsetH - pixel(1_ut);
				textRc.right = textRc.left + pixel(1_ut);

				canvas.setBackground(ss->getColor(this, L"color-caret"));
				canvas.fillRect(textRc);
			}

			lineRc = lineRc.offset(0, lineHeight);
		}
	}

	event->consume();
}

void RichEdit::eventSize(SizeEvent* event)
{
	const Rect inner = getInnerRect();
	const int32_t width = m_scrollBarV->getPreferredSize(inner.getSize()).cx;
	const int32_t height = m_scrollBarH->isVisible(false) ? m_scrollBarH->getPreferredSize(inner.getSize()).cy : 0;

	updateScrollBars();

	const Rect rcV(Point(inner.getWidth() - width, 0), Size(width, inner.getHeight() - height));
	m_scrollBarV->setRect(rcV);

	const Rect rcH(Point(0, inner.getHeight() - height), Size(inner.getWidth() - width, height));
	m_scrollBarH->setRect(rcH);

	const ui::Size searchControlSize = m_searchControl->getPreferredSize(inner.getSize());
	m_searchControl->setRect(ui::Rect(
		ui::Point(getEditRect().getWidth() - searchControlSize.cx, 0),
		searchControlSize
	));

	for (auto& ch : m_text)
		ch.width = 0;

	updateCharacterWidths();
}

void RichEdit::eventTimer(TimerEvent* event)
{
	m_caretBlink = !m_caretBlink;
	update();
}

void RichEdit::eventScroll(ScrollEvent* event)
{
	const Rect innerRc = getInnerRect();
	const Rect updateRc(innerRc.left, innerRc.top, m_lineMargin + m_widestLineWidth, innerRc.bottom);
	update(&updateRc);
}

void RichEdit::eventSearch(SearchEvent* event)
{
	if (!event->preview())
	{
		FindResult result;

		const std::wstring needle = m_searchControl->getNeedle();
		const bool caseSensitive = m_searchControl->caseSensitive();
		const bool wholeWord = m_searchControl->wholeWord();
		const bool wildcard = m_searchControl->wildcard();

		const int32_t caretLine = getLineFromOffset(getCaretOffset());
		if (findFirstLine(caretLine, needle, caseSensitive, wholeWord, wildcard, result))
		{
			showLine(result.line);
			placeCaret(getLineOffset(result.line) + result.offset);
			for (int32_t i = 0; i < getLineCount(); ++i)
				setBackgroundAttribute(i, (i == result.line) ? m_foundLineAttribute : 0xffff);
			setFocus();
		}
	}
	else
		updateFindPreview();
}

}
