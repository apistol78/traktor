#include "Core/Misc/Align.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Ui/MethodHandler.h"
#include "Ui/ScrollBar.h"
#include "Ui/Events/KeyEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Custom/RichEdit/RichEdit.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.RichEdit", RichEdit, Widget)

RichEdit::RichEdit()
:	m_caret(0)
{
}

bool RichEdit::create(Widget* parent, const std::wstring& text)
{
	if (!Widget::create(parent, WsClientBorder | WsDoubleBuffer))
		return false;

	addKeyDownEventHandler(createMethodHandler(this, &RichEdit::eventKeyDown));
	addKeyEventHandler(createMethodHandler(this, &RichEdit::eventKey));
	addPaintEventHandler(createMethodHandler(this, &RichEdit::eventPaint));
	addSizeEventHandler(createMethodHandler(this, &RichEdit::eventSize));

	// Create scrollbars.
	m_scrollBar = new ScrollBar();
	if (!m_scrollBar->create(this, ScrollBar::WsVertical))
		return false;

	m_scrollBar->addScrollEventHandler(createMethodHandler(this, &RichEdit::eventScroll));

	// Prepare initial text.
	StringSplit< std::wstring > ss(text, L"\n\r");
	for (StringSplit< std::wstring >::const_iterator i = ss.begin(); i != ss.end(); ++i)
	{
		const std::wstring& text = *i + L"\n";

		Line line;
		line.start = m_text.size();
		line.stop = line.start + text.length();
		m_lines.push_back(line);

		m_text.insert(m_text.end(), text.begin(), text.end());
	}

	return true;
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
	int line = getLineFromOffset(start);
	int lineOffset = start - getLineOffset(line);
}

void RichEdit::clear(bool attributes, bool content)
{
	if (attributes)
	{
		m_attributes.clear();
		m_formats.clear();
	}
	if (content)
	{
		m_lines.clear();
		m_text.clear();
	}
}

void RichEdit::insert(const std::wstring& text)
{
}

int RichEdit::getCaretOffset() const
{
	return 0;
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
	return m_lines[line].start;
}

int RichEdit::getLineLength(int line) const
{
	return m_lines[line].stop - m_lines[line].start - 1;
}

std::wstring RichEdit::getLine(int line) const
{
	return std::wstring(
		&m_text[m_lines[line].start],
		&m_text[m_lines[line].stop]
	);
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
	uint32_t lineHeight = font.getSize();
	uint32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

	m_scrollBar->setRange(lineCount);
	m_scrollBar->setPage(pageLines);
	m_scrollBar->setVisible(lineCount > pageLines);
	m_scrollBar->update();
}

void RichEdit::eventKeyDown(Event* event)
{
	KeyEvent* keyEvent = checked_type_cast< KeyEvent*, false >(event);
	switch (keyEvent->getVirtualKey())
	{
	case VkUp:
		// Move caret up.
		for (uint32_t i = 1; i < m_lines.size(); ++i)
		{
			if (m_caret >= m_lines[i].start && m_caret < m_lines[i].stop)
			{
				uint32_t offset = m_caret - m_lines[i].start;
				offset = std::min(offset, m_lines[i - 1].stop - m_lines[i - 1].start - 1);
				m_caret = m_lines[i - 1].start + offset;
				update();
				break;
			}
		}
		break;

	case VkDown:
		// Move caret down.
		for (uint32_t i = 0; i < m_lines.size() - 1; ++i)
		{
			if (m_caret >= m_lines[i].start && m_caret < m_lines[i].stop)
			{
				uint32_t offset = m_caret - m_lines[i].start;
				offset = std::min(offset, m_lines[i + 1].stop - m_lines[i + 1].start - 1);
				m_caret = m_lines[i + 1].start + offset;
				update();
				break;
			}
		}
		break;

	case VkLeft:
		// Move caret left.
		if (m_caret > 0)
		{
			--m_caret;
			update();
		}
		break;

	case VkRight:
		// Move caret right.
		if (m_caret < m_text.size() - 1)
		{
			++m_caret;
			update();
		}
		break;

	case VkHome:
		// Move caret home.
		for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
		{
			if (m_caret > i->start && m_caret < i->stop)
			{
				m_caret = i->start;
				update();
				break;
			}
		}
		break;

	case VkEnd:
		// Move caret end.
		for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
		{
			if (m_caret >= i->start && m_caret < i->stop - 1)
			{
				m_caret = i->stop - 1;
				update();
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
				if (m_caret >= m_lines[i].start && m_caret < m_lines[i].stop)
				{
					uint32_t offset = m_caret - m_lines[i].start;
					uint32_t di = std::min(pageLines, i);
					offset = std::min(offset, m_lines[i - di].stop - m_lines[i - di].start - 1);
					m_caret = m_lines[i - di].start + offset;
					update();
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
				if (m_caret >= m_lines[i].start && m_caret < m_lines[i].stop)
				{
					uint32_t offset = m_caret - m_lines[i].start;
					uint32_t di = std::min(pageLines, m_lines.size() - 1 - i);
					offset = std::min(offset, m_lines[i + di].stop - m_lines[i + di].start - 1);
					m_caret = m_lines[i + di].start + offset;
					update();
					break;
				}
			}
		}
		break;

	case VkBackSpace:
		if (m_caret > 0)
		{
			m_text.erase(m_text.begin() + m_caret - 1);
			for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
			{
				if (m_caret >= i->start && m_caret < i->stop)
					i->stop--;
				else if (m_caret < i->start)
				{
					i->start--;
					i->stop--;
				}
			}
			--m_caret;
			update();
		}
		break;
	}
}

void RichEdit::eventKey(Event* event)
{
	KeyEvent* keyEvent = checked_type_cast< KeyEvent*, false >(event);

	// Insert character; need to update line offsets.
	wchar_t ch = keyEvent->getCharacter();
	if (ch == L'\n' || ch == L'\r')
	{
		for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
		{
			if (m_caret >= i->start && m_caret < i->stop)
			{
				Line line;
				line.start = i->start;
				line.stop = m_caret;
				i->start = m_caret;
				m_lines.insert(i, line);
				break;
			}
		}
	}
	else
	{
		m_text.insert(m_text.begin() + m_caret, ch);
		for (std::vector< Line >::iterator i = m_lines.begin(); i != m_lines.end(); ++i)
		{
			if (m_caret >= i->start && m_caret < i->stop)
				i->stop++;
			else if (m_caret < i->start)
			{
				i->start++;
				i->stop++;
			}
		}
		++m_caret;
	}

	update();
}

void RichEdit::eventPaint(Event* event)
{
	PaintEvent* paintEvent = checked_type_cast< PaintEvent*, false >(event);
	Canvas& canvas = paintEvent->getCanvas();

	Font font = getFont();
	Rect rc = getInnerRect();
	if (m_scrollBar->isVisible(false))
		rc.right -= m_scrollBar->getPreferedSize().cx;

	canvas.setBackground(Color4ub(255, 255, 255));
	canvas.fillRect(rc);

	uint32_t lineCount = m_lines.size();
	uint32_t lineOffset = m_scrollBar->getPosition();
	uint32_t lineHeight = font.getSize();
	uint32_t pageLines = (rc.getHeight() + lineHeight - 1) / lineHeight;

	// Background
	{
		Rect lineRc(rc.left, rc.top, rc.right, rc.top + lineHeight);
		for (uint32_t i = lineOffset; i < lineOffset + pageLines && i < lineCount; ++i)
		{
			canvas.setBackground((i & 1) ? Color4ub(255, 255, 255) : Color4ub(240, 240, 240));
			canvas.fillRect(lineRc);

			Rect lnrRc = lineRc;
			lnrRc.right = lnrRc.left + 30;

			canvas.setBackground(Color4ub(160, 160, 160));
			canvas.fillRect(lnrRc);

			canvas.setForeground(Color4ub(0, 0, 0));
			canvas.drawText(lnrRc, toString(i + 1), AnLeft, AnCenter);

			lineRc = lineRc.offset(0, lineHeight);
		}
	}

	// Formatted text.
	{
		Rect lineRc(rc.left, rc.top, rc.right, rc.top + lineHeight);
		uint32_t lineOffsetEnd = std::min(lineOffset + pageLines, lineCount - 1);
		for (uint32_t i = lineOffset; i < lineOffsetEnd; ++i)
		{
			const Line& line = m_lines[i];

			canvas.setBackground(Color4ub(0, 0, 0));
			canvas.setForeground(Color4ub(0, 0, 0));

			Rect textRc = lineRc;
			uint32_t x = 0;

			for (uint32_t j = line.start; j < line.stop; )
			{
				uint32_t k = j;
				while (
					k < line.stop &&
					m_text[k] != '\t'
				)
					++k;

				if (m_caret >= j && m_caret < k)
				{
					std::wstring text(&m_text[j], &m_text[m_caret]);
					Rect caretRc = lineRc;
					caretRc.left = lineRc.left + 30 + x + canvas.getTextExtent(text).cx - 1;
					caretRc.right = caretRc.left + 1;
					canvas.fillRect(caretRc);
				}

				if (k > j)
				{
					std::wstring text(&m_text[j], &m_text[k]);
					textRc.left = lineRc.left + 30 + x;
					canvas.drawText(textRc, text, AnLeft, AnCenter);
					x += canvas.getTextExtent(text).cx;
				}

				if (m_text[k] == '\t')
					x = alignUp(x + 4 * 8, 4 * 8);

				j = k + 1;
			}

			lineRc = lineRc.offset(0, lineHeight);
		}
	}

	event->consume();
}

void RichEdit::eventSize(Event* event)
{
	int width = m_scrollBar->getPreferedSize().cx;

	Rect inner = getInnerRect();
	Rect rc(Point(inner.getWidth() - width, 0), Size(width, inner.getHeight()));

	m_scrollBar->setRect(rc);

	updateScrollBars();
}

void RichEdit::eventScroll(Event* event)
{
	update();
}

		}
	}
}
