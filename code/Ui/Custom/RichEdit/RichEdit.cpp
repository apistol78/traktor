#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Ui/MethodHandler.h"
#include "Ui/ScrollBar.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Custom/RichEdit/RichEdit.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

#if defined(_DEBUG)
#	define CHECK checkConsistency()
#else
#	define CHECK
#endif

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.RichEdit", RichEdit, Widget)

RichEdit::RichEdit()
:	m_caret(0)
,	m_selectionStart(~0UL)
,	m_selectionStop(0)
{
}

bool RichEdit::create(Widget* parent, const std::wstring& text)
{
	if (!Widget::create(parent, WsClientBorder | WsDoubleBuffer))
		return false;

	addKeyDownEventHandler(createMethodHandler(this, &RichEdit::eventKeyDown));
	addKeyEventHandler(createMethodHandler(this, &RichEdit::eventKey));
	addButtonDownEventHandler(createMethodHandler(this, &RichEdit::eventButtonDown));
	addPaintEventHandler(createMethodHandler(this, &RichEdit::eventPaint));
	addSizeEventHandler(createMethodHandler(this, &RichEdit::eventSize));

	// Create scrollbars.
	m_scrollBarV = new ScrollBar();
	m_scrollBarH = new ScrollBar();
	if (!m_scrollBarV->create(this, ScrollBar::WsVertical))
		return false;
	if (!m_scrollBarH->create(this, ScrollBar::WsHorizontal))
		return false;

	m_scrollBarV->addScrollEventHandler(createMethodHandler(this, &RichEdit::eventScroll));
	m_scrollBarH->addScrollEventHandler(createMethodHandler(this, &RichEdit::eventScroll));

	Attribute attrib;
	attrib.textColor = Color4ub(0, 0, 0);
	attrib.backColor = Color4ub(255, 255, 255);
	attrib.bold = false;
	attrib.italic = false;
	attrib.underline = false;
	m_attributes.push_back(attrib);

	setText(text);

	CHECK;

	return true;
}

void RichEdit::setText(const std::wstring& text)
{
	m_text.clear();

	if (!text.empty())
	{
		StringSplit< std::wstring > ss(text, L"\n\r");
		for (StringSplit< std::wstring >::const_iterator i = ss.begin(); i != ss.end(); ++i)
		{
			const std::wstring& text = *i;

			Line line;
			line.start = m_text.size();
			line.stop = line.start + text.length();
			m_lines.push_back(line);

			m_text.insert(m_text.end(), text.begin(), text.end());
			m_text.push_back(L'\n');
		}
	}
	else
	{
		Line line;
		line.start = 0;
		line.stop = 0;
		m_lines.push_back(line);
		m_text.push_back(L'\n');
	}

	m_meta.clear();
	m_meta.resize(m_text.size(), 0);
}
	
std::wstring RichEdit::getText() const
{
	const wchar_t* text = &m_text[0];
	return std::wstring(text, text + m_text.size());
}

int RichEdit::addAttribute(const Color4ub& textColor, const Color4ub& backColor, bool bold, bool italic, bool underline)
{
	Attribute attr;
	attr.textColor = textColor;
	attr.backColor = backColor;
	attr.bold = bold;
	attr.italic = italic;
	attr.underline = underline;
	m_attributes.push_back(attr);
	return int(m_attributes.size() - 1);
}

void RichEdit::setAttribute(int start, int length, int attribute)
{
	if (attribute < 0 || attribute >= int(m_attributes.size()))
		attribute = 0;

	if (start < 0)
		start = 0;
	
	if (start + length >= int(m_text.size()))
	{
		length = int(m_text.size()) - start;
		if (length < 0)
			return;
	}

	for (int i = start; i < start + length; ++i)
		m_meta[i] = uint16_t(attribute);

	CHECK;

	update();
}

void RichEdit::clear(bool attributes, bool content)
{
	if (attributes)
	{
		m_attributes.clear();
		
		m_meta.clear();
		m_meta.resize(m_text.size(), 0);
	}

	if (content)
	{
		m_lines.clear();
		m_text.clear();
	}

	CHECK;

	updateScrollBars();
	update();
}

void RichEdit::insert(const std::wstring& text)
{
}

int RichEdit::getCaretOffset() const
{
	return m_caret;
}

int RichEdit::getLineFromOffset(int offset) const
{
	for (int i = 0; i < int(m_lines.size()) - 1; ++i)
	{
		if (offset >= m_lines[i].start && offset < m_lines[i].stop)
			return i;
	}
	return int(m_lines.size()) - 1;
}

int RichEdit::getLineCount() const
{
	return int(m_lines.size());
}

int RichEdit::getLineOffset(int line) const
{
	return line < m_lines.size() ? m_lines[line].start : 0;
}

int RichEdit::getLineLength(int line) const
{
	return line < m_lines.size() ? (m_lines[line].stop - m_lines[line].start) : 0;
}

std::wstring RichEdit::getLine(int line) const
{
	if (line < m_lines.size())
	{
		const wchar_t* text = &m_text[0];
		return std::wstring(
			text + m_lines[line].start,
			text + m_lines[line].stop + 1
		);
	}
	else
		return L"";
}

bool RichEdit::redo()
{
	return false;
}

bool RichEdit::undo()
{
	return false;
}

void RichEdit::addChangeEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiContentChange, eventHandler);
}

void RichEdit::updateScrollBars()
{
	Font font = getFont();
	Rect rc = getInnerRect();

	uint32_t lineCount = m_lines.size();
	uint32_t lineOffset = 0;
	uint32_t lineHeight = font.getSize() + 1;
	uint32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

	m_scrollBarV->setRange(lineCount);
	m_scrollBarV->setPage(pageLines);
	m_scrollBarV->setVisible(lineCount > pageLines);
	m_scrollBarV->update();

	m_scrollBarH->setRange(100);
	m_scrollBarH->setPage(10);
	m_scrollBarH->setVisible(true);
	m_scrollBarH->update();
}

void RichEdit::deleteCharacters(bool backspace)
{
	if (backspace && m_caret == 0)
		return;

	if (backspace)
	{
		m_text.erase(m_text.begin() + m_caret - 1);
		m_meta.erase(m_meta.begin() + m_caret - 1);
	}
	else
	{
		m_text.erase(m_text.begin() + m_caret);
		m_meta.erase(m_meta.begin() + m_caret);
	}

	for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); )
	{
		if (m_caret == i->start)
		{
			std::vector< Line >::iterator j = i - 1;
			j->stop = i->stop - 1;
			i = m_lines.erase(i);
			continue;
		}
		else if (m_caret > i->start && m_caret <= i->stop)
			i->stop--;
		else if (m_caret < i->start)
		{
			i->start--;
			i->stop--;
		}
		++i;
	}

	if (backspace)
		--m_caret;

	raiseEvent(EiContentChange, 0);
}

void RichEdit::lineBreak()
{
	m_text.insert(m_text.begin() + m_caret, L'\n');
	m_meta.insert(m_meta.begin() + m_caret, 0);

	// Break line.
	for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
	{
		if (m_caret >= i->start && m_caret <= i->stop)
		{
			Line line;
			line.start = i->start;
			line.stop = m_caret;
			i->start = m_caret + 1;
			i->stop++;
			i = m_lines.insert(i, line) + 1;
		}
		else if (i->start > m_caret)
		{
			i->start++;
			i->stop++;
		}
	}

	++m_caret;
	raiseEvent(EiContentChange, 0);
}

void RichEdit::insertCharacter(wchar_t ch)
{
	m_text.insert(m_text.begin() + m_caret, ch);
	m_meta.insert(m_meta.begin() + m_caret, 0);

	for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
	{
		if (m_caret >= i->start && m_caret <= i->stop)
			i->stop++;
		else if (m_caret < i->start)
		{
			i->start++;
			i->stop++;
		}
	}

	++m_caret;
	raiseEvent(EiContentChange, 0);
}

void RichEdit::scrollToCaret()
{
}

int32_t RichEdit::getCharacterStops(const std::wstring& text, std::vector< int32_t >& outStops) const
{
	wchar_t chb[2] = { 0, 0 };
	int32_t x = 0;

	outStops.resize(0);
	for (std::wstring::const_iterator i = text.begin(); i != text.end(); ++i)
	{
		outStops.push_back(x);
		
		chb[0] = *i;
		x += getTextExtent(chb).cx;
	}

	return x;
}

void RichEdit::eventKeyDown(Event* event)
{
	KeyEvent* keyEvent = checked_type_cast< KeyEvent*, false >(event);

	// Prepare selection before caret movement.
	if (keyEvent->getKeyState() & KsShift)
	{
		if (m_selectionStart > m_selectionStop)
		{
			m_selectionStart =
			m_selectionStop = m_caret;
		}
	}

	switch (keyEvent->getVirtualKey())
	{
	case VkUp:
		// Move caret up.
		for (uint32_t i = 1; i < m_lines.size(); ++i)
		{
			if (m_caret >= m_lines[i].start && m_caret <= m_lines[i].stop)
			{
				uint32_t offset = m_caret - m_lines[i].start;
				offset = std::min(offset, m_lines[i - 1].stop - m_lines[i - 1].start);
				m_caret = m_lines[i - 1].start + offset;
				break;
			}
		}
		break;

	case VkDown:
		// Move caret down.
		for (uint32_t i = 0; i < m_lines.size() - 1; ++i)
		{
			if (m_caret >= m_lines[i].start && m_caret <= m_lines[i].stop)
			{
				uint32_t offset = m_caret - m_lines[i].start;
				offset = std::min(offset, m_lines[i + 1].stop - m_lines[i + 1].start);
				m_caret = m_lines[i + 1].start + offset;
				break;
			}
		}
		break;

	case VkLeft:
		// Move caret left.
		if (m_caret > 0)
			--m_caret;
		break;

	case VkRight:
		// Move caret right.
		if (m_caret < m_text.size() - 1)
			++m_caret;
		break;

	case VkHome:
		// Move caret home.
		for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
		{
			if (m_caret > i->start && m_caret <= i->stop)
			{
				m_caret = i->start;
				break;
			}
		}
		break;

	case VkEnd:
		// Move caret end.
		for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
		{
			if (m_caret >= i->start && m_caret < i->stop)
			{
				m_caret = i->stop;
				break;
			}
		}
		break;

	case VkPageUp:
		// Move caret one page up.
		{
			Font font = getFont();
			Rect rc = getInnerRect();

			uint32_t lineHeight = font.getSize();
			uint32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

			for (uint32_t i = 1; i < m_lines.size(); ++i)
			{
				if (m_caret >= m_lines[i].start && m_caret <= m_lines[i].stop)
				{
					uint32_t offset = m_caret - m_lines[i].start;
					uint32_t di = std::min(pageLines, i);
					offset = std::min(offset, m_lines[i - di].stop - m_lines[i - di].start);
					m_caret = m_lines[i - di].start + offset;
					break;
				}
			}
		}
		break;

	case VkPageDown:
		// Move caret one page down.
		{
			Font font = getFont();
			Rect rc = getInnerRect();

			uint32_t lineHeight = font.getSize();
			uint32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

			for (uint32_t i = 0; i < m_lines.size() - 1; ++i)
			{
				if (m_caret >= m_lines[i].start && m_caret <= m_lines[i].stop)
				{
					uint32_t offset = m_caret - m_lines[i].start;
					uint32_t di = std::min< uint32_t >(pageLines, m_lines.size() - 1 - i);
					offset = std::min(offset, m_lines[i + di].stop - m_lines[i + di].start);
					m_caret = m_lines[i + di].start + offset;
					break;
				}
			}
		}
		break;

	case VkDelete:
		deleteCharacters(false);
		break;
	}

	// Expand selection based on caret movement.
	if (keyEvent->getKeyState() & KsShift)
	{
		m_selectionStart = std::min(m_selectionStart, m_caret);
		m_selectionStop = std::max(m_selectionStop, m_caret);
	}
	else
	{
		m_selectionStart = ~0UL;
		m_selectionStop = 0;
	}

	scrollToCaret();
	update();
}

void RichEdit::eventKey(Event* event)
{
	KeyEvent* keyEvent = checked_type_cast< KeyEvent*, false >(event);

	// Insert character; need to update line offsets.
	wchar_t ch = keyEvent->getCharacter();
	if (ch == 8)
		deleteCharacters(true);
	else if (ch == L'\n' || ch == L'\r')
		lineBreak();
	else
		insertCharacter(ch);

	CHECK;

	scrollToCaret();
	update();
}

void RichEdit::eventButtonDown(Event* event)
{
	MouseEvent* mouseEvent = checked_type_cast< MouseEvent*, false >(event);
	Point mousePosition = mouseEvent->getPosition();

	Font font = getFont();
	Rect rc = getInnerRect();
	if (m_scrollBarV->isVisible(false))
		rc.right -= m_scrollBarV->getPreferedSize().cx;

	uint32_t lineCount = m_lines.size();
	uint32_t lineOffset = m_scrollBarV->getPosition();
	uint32_t lineHeight = font.getSize() + 1;
	uint32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

	uint32_t line = lineOffset + mousePosition.y / lineHeight;
	if (line >= lineCount)
		return;

	const Line& ln = m_lines[line];
	std::wstring text(&m_text[ln.start], &m_text[ln.stop]);
	
	std::vector< int32_t > stops;
	int32_t lineWidth = getCharacterStops(text, stops);

	int lineMargin = 30;

	if (mousePosition.x - lineMargin < lineWidth)
	{
		for (int32_t i = stops.size() - 1; i >= 0; --i)
		{
			if (mousePosition.x - lineMargin >= stops[i])
			{
				m_caret = ln.start + i;
				break;
			}
		}
	}
	else
		m_caret = ln.stop;

	update();
}

void RichEdit::eventPaint(Event* event)
{
	PaintEvent* paintEvent = checked_type_cast< PaintEvent*, false >(event);
	Canvas& canvas = paintEvent->getCanvas();

	Font font = getFont();
	Rect rc = getInnerRect();
	if (m_scrollBarV->isVisible(false))
		rc.right -= m_scrollBarV->getPreferedSize().cx;
	if (m_scrollBarH->isVisible(false))
		rc.bottom -= m_scrollBarH->getPreferedSize().cy;

	canvas.setBackground(Color4ub(255, 255, 255));
	canvas.fillRect(rc);

	uint32_t lineCount = m_lines.size();
	uint32_t lineOffset = m_scrollBarV->getPosition();
	uint32_t lineHeight = font.getSize() + 1;
	uint32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

	int lineMargin = 30;

	// Background
	{
		Rect lineRc(rc.left, rc.top, rc.right, rc.top + lineHeight);
		for (uint32_t i = lineOffset; i < lineOffset + pageLines && i < lineCount; ++i)
		{
			canvas.setBackground((i & 1) ? Color4ub(255, 255, 255) : Color4ub(245, 245, 245));
			canvas.fillRect(lineRc);

			Rect lnrRc = lineRc;
			lnrRc.right = lnrRc.left + lineMargin;

			canvas.setForeground(Color4ub(180, 180, 180));
			canvas.setBackground(Color4ub(200, 200, 200));
			canvas.fillGradientRect(lnrRc, false);

			canvas.setForeground(Color4ub(0, 0, 0));
			canvas.drawText(lnrRc, toString(i + 1), AnLeft, AnCenter);

			lineRc = lineRc.offset(0, lineHeight);
		}
	}

	// Formatted text.
	{
		Rect lineRc(rc.left, rc.top, rc.right, rc.top + lineHeight);
		uint32_t lineOffsetEnd = std::min(lineOffset + pageLines, lineCount);
		for (uint32_t i = lineOffset; i < lineOffsetEnd; ++i)
		{
			const Line& line = m_lines[i];

			Rect textRc = lineRc;
			uint32_t x = lineMargin + 2;

			// Non-empty line; format print.
			for (uint32_t j = line.start; j < line.stop; ++j)
			{
				const Attribute& attrib = m_attributes[m_meta[j]];

				if (m_caret == j)
				{
					Rect caretRc = lineRc;
					caretRc.left = lineRc.left + x - 1;
					caretRc.right = caretRc.left + 1;
					canvas.setBackground(Color4ub(0, 0, 0));
					canvas.fillRect(caretRc);
				}

				bool solidBackground = false;
				if (j >= m_selectionStart && j <= m_selectionStop)
				{
					canvas.setBackground(Color4ub(160, 160, 255));
					solidBackground = true;
				}
				else if (attrib.backColor != Color4ub(255, 255, 255))
				{
					canvas.setBackground(attrib.backColor);
					solidBackground = true;
				}
					
				canvas.setForeground(attrib.textColor);

				if (m_text[j] != '\t')
				{
					std::wstring ch(&m_text[j], &m_text[j + 1]);
					int32_t chw = canvas.getTextExtent(ch).cx;

					textRc.left = lineRc.left + x;
					textRc.right = textRc.left + chw;

					if (solidBackground)
						canvas.fillRect(textRc);
					canvas.drawText(textRc, ch, AnLeft, AnCenter);

					x += chw;
				}
				else
				{
					// Adjust offset to nearest tab-stop.
					int32_t nx = alignUp(x + 4 * 8, 4 * 8);

					textRc.left = lineRc.left + x;
					textRc.right = lineRc.left + nx;

					if (solidBackground)
						canvas.fillRect(textRc);

					x = nx;
				}
			}

			// Special condition; caret at the very end of a line.
			if (m_caret == line.stop)
			{
				Rect caretRc = lineRc;
				caretRc.left = lineRc.left + x - 1;
				caretRc.right = caretRc.left + 1;
				canvas.setBackground(Color4ub(0, 0, 0));
				canvas.fillRect(caretRc);
			}

			lineRc = lineRc.offset(0, lineHeight);
		}
	}

	event->consume();
}

void RichEdit::eventSize(Event* event)
{
	int width = m_scrollBarV->getPreferedSize().cx;
	int height = m_scrollBarH->getPreferedSize().cy;

	Rect inner = getInnerRect();

	Rect rcV(Point(inner.getWidth() - width, 0), Size(width, inner.getHeight() - height));
	m_scrollBarV->setRect(rcV);

	Rect rcH(Point(0, inner.getHeight() - height), Size(inner.getWidth() - width, height));
	m_scrollBarH->setRect(rcH);

	updateScrollBars();
}

void RichEdit::eventScroll(Event* event)
{
	update();
}

#if defined(_DEBUG)
void RichEdit::checkConsistency()
{
	if (m_lines.empty())
	{
		T_ASSERT (m_text.empty());
		T_ASSERT (m_meta.empty());
		return;
	}

	T_ASSERT (m_text.size() == m_meta.size());

	T_ASSERT (m_lines[0].start == 0);
	for (uint32_t i = 0; i < m_lines.size() - 1; ++i)
	{
		T_ASSERT (m_lines[i].stop + 1 == m_lines[i + 1].start);

		std::wstring text(&m_text[m_lines[i].start], &m_text[m_lines[i].stop]);
		T_ASSERT (text.find('\n') == text.npos);
		T_ASSERT (text.find('\r') == text.npos);
	}

	T_ASSERT (m_lines.back().stop == m_text.size() - 1);

	for (std::vector< uint16_t >::const_iterator i = m_meta.begin(); i != m_meta.end(); ++i)
		T_ASSERT (*i < m_attributes.size());

	// Save "good" lines so we can spot differences if
	// next fail.
	m_linesLastGood = m_lines;
}
#endif

		}
	}
}
