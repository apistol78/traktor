/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
#include "Ui/Custom/ScrollBar.h"
#include "Ui/Custom/RichEdit/CaretEvent.h"
#include "Ui/Custom/RichEdit/RichEdit.h"
#include "Ui/Custom/RichEdit/SearchControl.h"
#include "Ui/Custom/RichEdit/SearchEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int32_t c_lineMarginMin = 40;
const int32_t c_iconSize = 16;
const int32_t c_scrollHSteps = 10;

#if defined(__APPLE__)
const int32_t c_fontHeightMargin = 4;
#elif defined(__LINUX__)
const int32_t c_fontHeightMargin = 1;
#else
const int32_t c_fontHeightMargin = 2;
#endif

bool isWordSeparator(wchar_t ch)
{
	return !(std::iswalnum(ch) || ch == L'_');
}

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.RichEdit", RichEdit, Widget)

RichEdit::RichEdit()
:	m_imageWidth(0)
,	m_imageHeight(0)
,	m_imageCount(0)
,	m_clipboard(true)
,	m_caret(0)
,	m_caretBlink(true)
,	m_selectionStart(-1)
,	m_selectionStop(-1)
,	m_lineMargin(c_lineMarginMin)
,	m_lineOffsetH(0)
,	m_widestLineWidth(0)
,	m_nextSpecialCharacter(0xff00)
,	m_foundLineAttribute(0)
{
}

bool RichEdit::create(Widget* parent, const std::wstring& text, int32_t style)
{
	if (!Widget::create(parent, style | WsWantAllInput | WsDoubleBuffer | WsAccelerated))
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

	const ui::StyleSheet* ss = ui::Application::getInstance()->getStyleSheet();
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
			size_t j = text.find(L'\n', i);

			std::wstring ln = (j != text.npos) ? text.substr(i, j - i) : text.substr(i);

			Line line;
			line.start = int32_t(m_text.size());
			line.stop = line.start + int32_t(ln.length());
			m_lines.push_back(line);

			for (std::wstring::const_iterator k = ln.begin(); k != ln.end(); ++k)
				m_text.push_back(Character(*k));

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
		Line line;
		line.start = 0;
		line.stop = 0;
		m_lines.push_back(line);
		m_text.push_back(Character(L'\n'));
	}

	m_selectionStart =
	m_selectionStop = -1;

	int32_t lastOffset = int32_t(m_text.size());
	if (m_caret >= lastOffset)
	{
		m_caret = lastOffset;

		CaretEvent caretEvent(this);
		raiseEvent(&caretEvent);
	}

	updateCharacterWidths();
	updateScrollBars();

	santiyCheck();
}

std::wstring RichEdit::getText() const
{
	if (!m_text.empty())
	{
		std::vector< wchar_t > text(m_text.size());
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
		std::map< wchar_t, Ref< const ISpecialCharacter > >::const_iterator j = m_specialCharacters.find(m_text[i].ch);
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

	for (std::vector< Character >::iterator i = m_text.begin(); i != m_text.end(); ++i)
		i->width = 0;

	updateCharacterWidths();
}

int32_t RichEdit::addTextAttribute(const Color4ub& textColor, bool bold, bool italic, bool underline)
{
	TextAttribute attr;
	attr.textColor = textColor;
	attr.bold = bold;
	attr.italic = italic;
	attr.underline = underline;
	m_textAttributes.push_back(attr);
	return int32_t(m_textAttributes.size() - 1);
}

int32_t RichEdit::addBackgroundAttribute(const Color4ub& backColor)
{
	BackgroundAttribute attr;
	attr.backColor = backColor;
	m_backgroundAttributes.push_back(attr);
	return int32_t(m_backgroundAttributes.size() - 1);
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
	T_ASSERT (image);
	T_ASSERT (imageCount > 0);

	if (m_image)
	{
		uint32_t width = m_image->getSize().cx + image->getSize().cx;
		uint32_t height = std::max(m_image->getSize().cy, image->getSize().cy);

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

	uint32_t imageBase = m_imageCount;
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
	wchar_t ch = m_nextSpecialCharacter++;
	m_specialCharacters[ch] = specialCharacter;
	updateCharacterWidths();
	return ch;
}

void RichEdit::clear(bool attributes, bool images, bool content)
{
	if (attributes)
	{
		for (std::vector< Character >::iterator i = m_text.begin(); i != m_text.end(); ++i)
		{
			i->tai = 0;
			i->bgai = 0;
		}
		for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
			i->attrib = 0xffff;
	}

	if (content)
	{
		m_lines.clear();
		m_text.clear();
	}
	else if (images)
	{
		for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
			i->image = -1;
	}

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

	updateCharacterWidths();
	updateScrollBars();
	update();

	santiyCheck();
}

void RichEdit::insert(const std::wstring& text)
{
	if (text.empty())
		return;

	std::wstring tmp = traktor::replaceAll< std::wstring >(text, L"\r\n", L"\n");
	for (std::wstring::const_iterator i = tmp.begin(); i != tmp.end(); ++i)
		insertCharacter(*i, false);

	CaretEvent caretEvent(this);
	raiseEvent(&caretEvent);

	ContentChangeEvent contentChangeEvent(this);
	raiseEvent(&contentChangeEvent);
}

int32_t RichEdit::getOffsetFromPosition(const Point& position)
{
	Font font = getFont();
	Rect rc = getEditRect();

	uint32_t lineCount = uint32_t(m_lines.size());
	uint32_t lineOffset = m_scrollBarV->getPosition();
	uint32_t lineHeight = font.getPixelSize() + ui::dpi96(c_fontHeightMargin);

	uint32_t line = lineOffset + position.y / lineHeight;
	if (line >= lineCount)
		return -1;

	const Line& ln = m_lines[line];

	std::vector< int32_t > stops;

	int32_t x = 0;
	for (int32_t i = ln.start; i < ln.stop; ++i)
	{
		stops.push_back(x);
		x += m_text[i].width;
	}

	int32_t lineWidth = x;
	int32_t linePosition = std::max(position.x - m_lineMargin, 0) + m_lineOffsetH;
	if (linePosition < lineWidth)
	{
		for (int32_t i = int32_t(stops.size()) - 1; i >= 0; --i)
		{
			if (linePosition >= stops[i])
			{
				return ln.start + i;
				break;
			}
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
	Font font = getFont();
	Rect rc = getEditRect();

	uint32_t lineCount = uint32_t(m_lines.size());
	uint32_t lineOffset = m_scrollBarV->getPosition();
	uint32_t lineHeight = font.getPixelSize() + ui::dpi96(c_fontHeightMargin);
	uint32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

	uint32_t line = lineOffset + position / lineHeight;
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

	int32_t adjust = int32_t(text.size()) - (ln.stop - ln.start);
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
		std::vector< wchar_t > text;
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

	std::vector< wchar_t > text;
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
		std::map< wchar_t, Ref< const ISpecialCharacter > >::const_iterator j = m_specialCharacters.find(m_text[i].ch);
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
	Font font = getFont();
	Rect rc = getEditRect();

	int32_t lineHeight = font.getPixelSize() + ui::dpi96(c_fontHeightMargin);
	int32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

	int32_t top = m_scrollBarV->getPosition();

	if (line >= top && line < top + pageLines)
		return true;

	m_scrollBarV->setPosition(std::max< int32_t >(line - pageLines / 3, 0));
	m_scrollBarV->update();

	update();
	return true;
}

void RichEdit::placeCaret(int32_t offset)
{
	m_caret = offset;
	
	CaretEvent caretEvent(this);
	raiseEvent(&caretEvent);

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

bool RichEdit::copy()
{
	Clipboard* clipboard = Application::getInstance()->getClipboard();
	if (!clipboard)
		return false;

	std::wstring selectedText = getSelectedText();
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

	if (m_selectionStart >= 0)
		deleteCharacters();

	std::wstring pasteText = clipboard->getText();
	insert(pasteText);

	return true;
}

bool RichEdit::find()
{
	// Initialize needle with selection; only if selection is single line.
	int32_t startLine = getLineFromOffset(getSelectionStartOffset());
	int32_t stopLine = getLineFromOffset(getSelectionStopOffset());
	if (startLine == stopLine)
	{
		std::wstring needle = getSelectedText();
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

	std::wstring needle = m_searchControl->getNeedle();
	bool caseSensitive = m_searchControl->caseSensitive();
	bool wholeWord = m_searchControl->wholeWord();
	bool wildcard = m_searchControl->wildcard();

	m_searchControl->setAnyMatchingHint(false);

	int32_t caretLine = getLineFromOffset(getCaretOffset());
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
	rc.right -= m_scrollBarV->getPreferedSize().cx;
	if (m_scrollBarH->isVisible(false))
		rc.bottom -= m_scrollBarH->getPreferedSize().cy;
	return rc;
}

int32_t RichEdit::getMarginWidth() const
{
	return m_lineMargin;
}

void RichEdit::updateScrollBars()
{
	Font font = getFont();
	Rect rc = getEditRect();

	uint32_t lineCount = uint32_t(m_lines.size());
	uint32_t lineHeight = font.getPixelSize() + ui::dpi96(c_fontHeightMargin);
	uint32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

	m_scrollBarV->setRange(lineCount + pageLines);
	m_scrollBarV->setPage(pageLines);
	m_scrollBarV->update();

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

	m_scrollBarH->update();
}

void RichEdit::updateCharacterWidths()
{
	Size szw = getTextExtent(L" ");
	if (szw.cx <= 0)
		szw.cx = 1;

	m_widestLineWidth = 0;
	for (std::vector< Line >::const_iterator i = m_lines.begin(); i != m_lines.end(); ++i)
	{
		int32_t x = 0;
		for (int32_t j = i->start; j < i->stop; ++j)
		{
			Character& c = m_text[j];
			if (c.width == 0)
			{
				if (c.ch != L'\t')
				{
					std::map< wchar_t, Ref< const ISpecialCharacter > >::const_iterator k = m_specialCharacters.find(c.ch);
					if (k == m_specialCharacters.end())
					{
						Size sz = getTextExtent(std::wstring(1, c.ch));
						c.width = sz.cx;
					}
					else
					{
						c.width = k->second->measureWidth(this);
					}
				}
				else
				{
					int32_t nx = alignUp(x + 4 * szw.cx, 4 * szw.cx);
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

	// Naiive implementation, delete each character
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
			int32_t indentFromLine = getLineFromOffset(m_selectionStart);
			int32_t indentToLine = getLineFromOffset(m_selectionStop - 1);

			if (indentFromLine <= indentToLine)
			{
				for (int32_t i = indentFromLine; i <= indentToLine; ++i)
				{
					int32_t offset = getLineOffset(i);
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
		for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
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
		for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
		{
			if (offset >= i->start && offset <= i->stop)
				i->stop++;
			else if (offset < i->start)
			{
				i->start++;
				i->stop++;
			}
		}
	}

	updateCharacterWidths();
	santiyCheck();
}

void RichEdit::scrollToCaret()
{
	if (m_scrollBarV->isVisible(false))
	{
		int32_t caretLine = getLineFromOffset(m_caret);

		Font font = getFont();
		Rect rc = getEditRect();

		int32_t lineHeight = font.getPixelSize() + ui::dpi96(c_fontHeightMargin);
		int32_t pageLines = rc.getHeight() / lineHeight;

		int32_t top = m_scrollBarV->getPosition();

		if (caretLine < top)
		{
			m_scrollBarV->setPosition(caretLine);
			m_scrollBarV->update();
		}
		else if (caretLine >= top + pageLines)
		{
			m_scrollBarV->setPosition(caretLine - pageLines + 1);
			m_scrollBarV->update();
		}

		update();
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

	std::wstring needle = m_searchControl->getNeedle();
	bool caseSensitive = m_searchControl->caseSensitive();
	bool wholeWord = m_searchControl->wholeWord();
	bool wildcard = m_searchControl->wildcard();

	m_searchControl->setAnyMatchingHint(
		findFirstLine(0, needle, caseSensitive, wholeWord, wildcard, result)
	);
}

bool RichEdit::findFirstLine(int32_t currentLine, const std::wstring& needle, bool caseSensitive, bool wholeWord, bool wildcard, FindResult& outResult) const
{
	std::wstring ndl = caseSensitive ? needle : toLower(needle);
	int32_t line = currentLine;

	while (line < getLineCount())
	{
		std::wstring text = caseSensitive ? getLine(line) : toLower(getLine(line));
		size_t p = text.find(ndl);
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
			std::wstring text = caseSensitive ? getLine(line) : toLower(getLine(line));
			size_t p = text.find(ndl);
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
	int32_t caret = m_caret;
	bool caretMovement = false;
	bool manualScrolled = false;

	switch (event->getVirtualKey())
	{
	case VkEscape:
		m_searchControl->hide();
		break;

	case VkUp:
		if ((event->getKeyState() & KsControl) == 0)
		{
			// Move caret up.
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
		else
		{
			// Scroll up.
			if (m_scrollBarV->isVisible(false))
			{
				int32_t position = m_scrollBarV->getPosition();
				m_scrollBarV->setPosition(position - 1);
				m_scrollBarV->update();
				update();
				manualScrolled = true;
			}
		}
		break;

	case VkDown:
		if ((event->getKeyState() & KsControl) == 0)
		{
			// Move caret down.
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
		else
		{
			// Scroll down.
			if (m_scrollBarV->isVisible(false))
			{
				int32_t position = m_scrollBarV->getPosition();
				m_scrollBarV->setPosition(position + 1);
				m_scrollBarV->update();
				update();
				manualScrolled = true;
			}
		}
		break;

	case VkLeft:
		// Move caret left.
		if ((event->getKeyState() & KsControl) == 0)
		{
			if (m_caret > 0)
				--m_caret;
		}
		else
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
		// Move caret right.
		if ((event->getKeyState() & KsControl) == 0)
		{
			if (m_caret < int32_t(m_text.size()) - 1)
				++m_caret;
		}
		else
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
			if ((event->getKeyState() & KsControl) == 0)
			{
				for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
				{
					if (m_caret > i->start && m_caret <= i->stop)
					{
						m_caret = i->start;
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
			if ((event->getKeyState() & KsControl) == 0)
			{
				for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
				{
					if (m_caret >= i->start && m_caret < i->stop)
					{
						m_caret = i->stop;
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
			Font font = getFont();
			Rect rc = getEditRect();

			int32_t lineHeight = font.getPixelSize() + ui::dpi96(c_fontHeightMargin);
			int32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

			for (int32_t i = 1; i < int32_t(m_lines.size()); ++i)
			{
				if (m_caret >= m_lines[i].start && m_caret <= m_lines[i].stop)
				{
					int32_t offset = m_caret - m_lines[i].start;
					int32_t di = std::min(pageLines, i);
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
			Font font = getFont();
			Rect rc = getEditRect();

			int32_t lineHeight = font.getPixelSize() + ui::dpi96(c_fontHeightMargin);
			int32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

			for (int32_t i = 0; i < int32_t(m_lines.size()) - 1; ++i)
			{
				if (m_caret >= m_lines[i].start && m_caret <= m_lines[i].stop)
				{
					int32_t offset = m_caret - m_lines[i].start;
					int32_t di = std::min< int32_t >(pageLines, int32_t(m_lines.size()) - 1 - i);
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
	if (caretMovement && (event->getKeyState() & KsShift) != 0)
	{
		if (m_selectionStart < 0)
		{
			m_selectionStart = std::min(caret, m_caret);
			m_selectionStop = std::max(caret, m_caret);
		}
		else
		{
			bool caretAtSelectionHead = bool(caret == m_selectionStart);
			bool caretAtSelectionTail = bool(caret == m_selectionStop);

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

	updateScrollBars();
	
	if (!manualScrolled)
		scrollToCaret();

	update();
	santiyCheck();
}

void RichEdit::eventKey(KeyEvent* event)
{
	wchar_t ch = event->getCharacter();

	if (ch == 3 && m_clipboard)
		copy();
	else if (ch == 22 && m_clipboard)
		paste();
	else if (ch == 24 && m_clipboard)
		cut();
	else if (ch != 8)
		insertCharacter(ch, true);

	updateScrollBars();
	scrollToCaret();
	update();
	santiyCheck();
}

void RichEdit::eventButtonDown(MouseButtonDownEvent* event)
{
	Point position = event->getPosition();
	if (position.x < m_lineMargin)
		return;

	setCursor(CrIBeam);

	if (event->getButton() == MbtLeft)
	{
		int32_t offset = getOffsetFromPosition(position);
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

				std::map< wchar_t, Ref< const ISpecialCharacter > >::const_iterator i = m_specialCharacters.find(m_text[m_caret].ch);
				if (i != m_specialCharacters.end())
					i->second->mouseButtonDown(event);
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
	setCursor(CrIBeam);

	if (!hasCapture())
		return;

	std::map< wchar_t, Ref< const ISpecialCharacter > >::const_iterator i = m_specialCharacters.find(m_text[m_caret].ch);
	if (i != m_specialCharacters.end())
		i->second->mouseButtonUp(event);

	releaseCapture();
}

void RichEdit::eventMouseMove(MouseMoveEvent* event)
{
	Point position = event->getPosition();
	if (position.x >= m_lineMargin)
		setCursor(CrIBeam);
	else
		setCursor(CrArrow);

	if (!hasCapture())
		return;

	int32_t offset = getOffsetFromPosition(position);
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

	Point position = event->getPosition();
	if (position.x < m_lineMargin)
		return;

	int32_t offset = getOffsetFromPosition(position);
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

		std::map< wchar_t, Ref< const ISpecialCharacter > >::const_iterator i = m_specialCharacters.find(m_text[m_caret].ch);
		if (i != m_specialCharacters.end())
			i->second->mouseDoubleClick(event);

		CaretEvent caretEvent(this);
		raiseEvent(&caretEvent);

		update();
	}
}

void RichEdit::eventMouseWheel(MouseWheelEvent* event)
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

void RichEdit::eventPaint(PaintEvent* event)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	Canvas& canvas = event->getCanvas();

	Font font = getFont();
	Rect innerRc = getInnerRect();
	Rect updateRc = event->getUpdateRect();

	// Clear entire background.
	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(innerRc);

	innerRc.right -= m_scrollBarV->getPreferedSize().cx;
	if (m_scrollBarH->isVisible(false))
		innerRc.bottom -= m_scrollBarH->getPreferedSize().cy;

	uint32_t lineCount = uint32_t(m_lines.size());
	uint32_t lineOffset = m_scrollBarV->getPosition();
	uint32_t lineHeight = font.getPixelSize() + ui::dpi96(c_fontHeightMargin);
	uint32_t pageLines = (innerRc.getHeight() + lineHeight - 1) / lineHeight;

	// Calculate margin width from highest visible line number.
	m_lineMargin = dpi96(c_iconSize) + canvas.getTextExtent(toString(lineOffset + pageLines)).cx + dpi96(2);
	m_lineOffsetH = m_scrollBarH->isVisible(false) ? m_scrollBarH->getPosition() * c_scrollHSteps : 0;

	// Background
	{
		Rect marginRc(innerRc.left, innerRc.top, innerRc.left + m_lineMargin, innerRc.bottom);
		Rect iconsRc(innerRc.left, innerRc.top, innerRc.left + dpi96(c_iconSize), innerRc.top + lineHeight);
		Rect lineRc(innerRc.left + dpi96(c_iconSize), innerRc.top, innerRc.left + m_lineMargin, innerRc.top + lineHeight);

		canvas.setBackground(ss->getColor(this, L"background-color-margin"));
		canvas.fillRect(marginRc);

		canvas.setForeground(ss->getColor(this, L"color-margin"));
		for (uint32_t i = lineOffset; i < lineOffset + pageLines && i < lineCount; ++i)
		{
			canvas.drawText(lineRc, toString(i + 1), AnLeft, AnCenter);

			if (m_lines[i].image >= 0)
			{
				canvas.drawBitmap(
					Point(iconsRc.left, iconsRc.top + (iconsRc.getHeight() - m_imageHeight) / 2),
					Point(m_lines[i].image * m_imageWidth, 0),
					Size(m_imageWidth, m_imageHeight),
					m_image,
					BmAlpha
				);
			}

			iconsRc = iconsRc.offset(0, lineHeight);
			lineRc = lineRc.offset(0, lineHeight);
		}
	}

	// Formatted text.
	{
		bool showCaret = m_caretBlink && hasFocus();

		canvas.setClipRect(Rect(
			innerRc.left + m_lineMargin,
			innerRc.top,
			innerRc.right,
			innerRc.bottom
		));

		Rect lineRc(innerRc.left, innerRc.top, innerRc.right, innerRc.top + lineHeight);
		uint32_t lineOffsetEnd = std::min(lineOffset + pageLines, lineCount);
		for (uint32_t i = lineOffset; i < lineOffsetEnd; ++i)
		{
			const Line& line = m_lines[i];

			// Draw line background attribute.
			if (line.attrib != 0xffff)
			{
				const BackgroundAttribute& bgAttrib = m_backgroundAttributes[line.attrib];
				canvas.setBackground(bgAttrib.backColor);
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
					textRc.left = m_lineMargin + 2 + x - dpi96(1) - m_lineOffsetH;
					textRc.right = textRc.left + dpi96(1);

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
					canvas.setForeground(txAttrib.textColor);
					solidBackground = bool(bgAttrib.backColor.a != 0);
					if (solidBackground)
						canvas.setBackground(bgAttrib.backColor);
				}

				// Draw characters.
				textRc.left = m_lineMargin + 2 + x - m_lineOffsetH;
				textRc.right = textRc.left + m_text[j].width;

				if (solidBackground)
					canvas.fillRect(textRc);

				std::map< wchar_t, Ref< const ISpecialCharacter > >::const_iterator k = m_specialCharacters.find(m_text[j].ch);
				if (k == m_specialCharacters.end())
				{
					if (std::iswgraph(m_text[j].ch))
						canvas.drawText(textRc, std::wstring(1, m_text[j].ch), AnLeft, AnCenter);
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
				textRc.left = m_lineMargin + 2 + x - m_lineOffsetH - dpi96(1);
				textRc.right = textRc.left + dpi96(1);

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
	int32_t width = m_scrollBarV->getPreferedSize().cx;
	int32_t height = m_scrollBarH->isVisible(false) ? m_scrollBarH->getPreferedSize().cy : 0;

	Rect inner = getInnerRect();

	updateScrollBars();

	Rect rcV(Point(inner.getWidth() - width, 0), Size(width, inner.getHeight() - height));
	m_scrollBarV->setRect(rcV);

	Rect rcH(Point(0, inner.getHeight() - height), Size(inner.getWidth() - width, height));
	m_scrollBarH->setRect(rcH);

	ui::Size searchControlSize = m_searchControl->getPreferedSize();
	m_searchControl->setRect(ui::Rect(
		ui::Point(getEditRect().getWidth() - searchControlSize.cx, 0),
		searchControlSize
	));
}

void RichEdit::eventTimer(TimerEvent* event)
{
	m_caretBlink = !m_caretBlink;
	update();
}

void RichEdit::eventScroll(ScrollEvent* event)
{
	Rect innerRc = getInnerRect();
	Rect updateRc(innerRc.left, innerRc.top, m_lineMargin + m_widestLineWidth, innerRc.bottom);
	update(&updateRc);
}

void RichEdit::eventSearch(SearchEvent* event)
{
	if (!event->preview())
	{
		FindResult result;

		std::wstring needle = m_searchControl->getNeedle();
		bool caseSensitive = m_searchControl->caseSensitive();
		bool wholeWord = m_searchControl->wholeWord();
		bool wildcard = m_searchControl->wildcard();

		int32_t caretLine = getLineFromOffset(getCaretOffset());
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
	}
}
