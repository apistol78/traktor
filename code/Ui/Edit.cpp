/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cwctype>
#include "Core/Log/Log.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Edit.h"
#include "Ui/EditValidator.h"
#include "Ui/StyleSheet.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Edit", Edit, Widget)

Edit::Edit()
:	m_selectionStart(-1)
,	m_selectionEnd(-1)
,	m_caret(0)
,	m_caretBlink(true)
,	m_readOnly(false)
{
}

bool Edit::create(Widget* parent, const std::wstring& text, int style, const EditValidator* validator)
{
	if (!Widget::create(parent, style | WsWantAllInput | WsDoubleBuffer))
		return false;

	if ((m_validator = validator) != 0)
	{
		if (m_validator->validate(text) == EditValidator::VrInvalid)
			return false;
	}

	m_readOnly = bool((style & WsReadOnly) != 0);

	addEventHandler< FocusEvent >(this, &Edit::eventFocus);
	addEventHandler< MouseMoveEvent >(this, &Edit::eventMouseMove);
	addEventHandler< MouseButtonDownEvent >(this, &Edit::eventButtonDown);
	addEventHandler< KeyDownEvent >(this, &Edit::eventKeyDown);
	addEventHandler< KeyEvent >(this, &Edit::eventKey);
	addEventHandler< PaintEvent >(this, &Edit::eventPaint);
	addEventHandler< TimerEvent >(this, &Edit::eventTimer);

	setText(text);
	return true;
}

bool Edit::setValidator(const EditValidator* validator)
{
	if (validator)
	{
		if (validator->validate(getText()) == EditValidator::VrInvalid)
			return false;
	}
	m_validator = validator;
	return true;
}

const EditValidator* Edit::getValidator() const
{
	return m_validator;
}

void Edit::select(int from, int to)
{
	if (from >= to)
	{
		m_selectionStart = from;
		m_selectionEnd = to;
	}
	else
	{
		m_selectionStart =
		m_selectionEnd = -1;
	}
	update();
}

void Edit::selectAll()
{
	std::wstring text = getText();
	if (!text.empty())
	{
		m_selectionStart = 0;
		m_selectionEnd = text.length();
	}
	else
	{
		m_selectionStart =
		m_selectionEnd = -1;
	}
	update();
}

void Edit::deselect()
{
	m_selectionStart =
	m_selectionEnd = -1;
	update();
}

bool Edit::haveSelection() const
{
	return m_selectionStart >= 0;
}

std::wstring Edit::getSelectedText() const
{
	if (haveSelection())
	{
		std::wstring text = getText();
		return text.substr(m_selectionStart, m_selectionEnd - m_selectionStart);
	}
	else
		return L"";
}

void Edit::insert(const std::wstring& text)
{
	std::wstring current = getText();

	if (!haveSelection())
	{
		if (m_caret >= current.length())
			current += text;
		else
			current = current.substr(0, m_caret) + text + current.substr(m_caret);
	}
	else
		current = current.substr(0, m_selectionStart) + text + current.substr(m_selectionEnd);

	setText(current);
}

void Edit::copy()
{
	if (!haveSelection())
		return;

	auto clipboard = Application::getInstance()->getClipboard();
	if (clipboard != nullptr)
		clipboard->setText(getSelectedText());
}

void Edit::cut()
{
	if (!haveSelection())
		return;

	auto clipboard = Application::getInstance()->getClipboard();
	if (clipboard != nullptr)
	{
		clipboard->setText(getSelectedText());

		std::wstring current = getText();
		current = current.substr(0, m_selectionStart) + current.substr(m_selectionEnd);
		setText(current);	
	}
}

void Edit::paste()
{
	auto clipboard = Application::getInstance()->getClipboard();
	if (clipboard != nullptr)
	{
		if (clipboard->getContentType() == CtText)
			insert(clipboard->getText());
	}
}

void Edit::setBorderColor(const Color4ub& borderColor)
{
}

void Edit::setText(const std::wstring& text)
{
	Widget::setText(text);

	// Ensure caret position is clamped within text limits.
	m_caret = std::min< int32_t >(m_caret, text.length());
	m_caret = std::max< int32_t >(m_caret, 0);

	deselect();
}

Size Edit::getPreferedSize() const
{
	const int32_t height = getFontMetric().getHeight() + dpi96(4) * 2;
	return Size(dpi96(200), height);
}

void Edit::eventFocus(FocusEvent* event)
{
	if (event->gotFocus())
	{
		m_caretBlink = true;
		startTimer(500);
	}
	else
	{
		m_caretBlink = true;
		stopTimer();
		deselect();
	}
	update();
}

void Edit::eventMouseMove(MouseMoveEvent* event)
{
	if (!isEnable())
		return;

	if (!hasCapture())
	{
		setCapture();
		update();
	}
	else if (!getInnerRect().inside(event->getPosition()))
	{
		releaseCapture();
		update();
	}
}

void Edit::eventButtonDown(MouseButtonDownEvent* event)
{
	int32_t mx = event->getPosition().x;
	int32_t x = dpi96(4);

	int32_t caret = m_caret;
	if (mx >= x)
	{
		caret = -1;

		std::wstring text = getText();
		FontMetric fm = getFontMetric();

		for (int32_t i = 0; i < text.length(); ++i)
		{
			int32_t a = fm.getAdvance(text[i], 0);
			if (mx >= x && mx <= x + a)
			{
				if (mx <= x + a / 2)
					caret = i;
				else
					caret = i + 1;
				break;
			}
			x += a;
		}
		if (caret < 0)
			caret = int32_t(text.length());
	}
	else
		caret = 0;

	if (caret != m_caret)
	{
		if ((event->getKeyState() & KsShift) != 0)
		{
			if (m_selectionStart == -1)
			{
				m_selectionStart = std::min< int32_t >(caret, m_caret);
				m_selectionEnd = std::max< int32_t >(caret, m_caret);
			}
			else
			{
				m_selectionStart = std::min< int32_t >(m_selectionStart, caret);
				m_selectionEnd = std::max< int32_t >(m_selectionEnd, caret);
			}
		}
		else
		{
			deselect();
		}

		m_caret = caret;
		update();		
	}
}

void Edit::eventKeyDown(KeyDownEvent* event)
{
	int32_t caret = m_caret;

	switch (event->getVirtualKey())
	{
	case VkLeft:
		{
			caret = std::max< int32_t >(caret - 1 , 0);
		}
		break;

	case VkRight:
		{
			std::wstring text = getText();
			caret = std::min< int32_t >(caret + 1, text.length());
		}
		break;

	case VkHome:
		{
			caret = 0;
		}
		break;
		
	case VkEnd:
		{
			std::wstring text = getText();
			caret = text.length();
		}
		break;

	default:
		break;
	}

	if (caret != m_caret)
	{
		if ((event->getKeyState() & KsShift) != 0)
		{
			if (m_selectionStart == -1)
			{
				m_selectionStart = std::min< int32_t >(caret, m_caret);
				m_selectionEnd = std::max< int32_t >(caret, m_caret);
			}
			else
			{
				m_selectionStart = std::min< int32_t >(m_selectionStart, caret);
				m_selectionEnd = std::max< int32_t >(m_selectionEnd, caret);
			}
		}
		else
		{
			deselect();
		}

		m_caret = caret;
		update();
	}
}

void Edit::eventKey(KeyEvent* event)
{
	wchar_t ch = event->getCharacter();
	if ((event->getKeyState() & KsControl) == 0)
	{
		std::wstring text = getText();
		int32_t caret = m_caret;

		if (ch == 8)
		{
			if (!haveSelection())
			{
				if (caret > 0)
				{
					text = text.substr(0, caret - 1) + text.substr(caret);
					--caret;
				}
			}
			else
			{
				text = text.substr(0, m_selectionStart) + text.substr(m_selectionEnd);
			}
		}
		else if (ch == 10 || ch == 13 || ch == 127)
			return;
		else
		{
			if (!haveSelection())
			{
				if (caret >= text.length())
					text += ch;
				else
					text = text.substr(0, caret) + ch + text.substr(caret);
			}
			else
				text = text.substr(0, m_selectionStart) + ch + text.substr(m_selectionEnd);

			++caret;
		}

		deselect();

		// Ensure caret position is clamped within text limits.
		caret = std::min< int32_t >(caret, text.length());
		caret = std::max< int32_t >(caret, 0);

		if (m_validator == nullptr || m_validator->validate(text) != EditValidator::VrInvalid)
		{
			setText(text);
			m_caret = caret;

			ContentChangeEvent contentChangeEvent(this);
			raiseEvent(&contentChangeEvent);
		}

		update();
	}
	else
	{
		if (ch == L'a' || ch == L'A')
			selectAll();
		else if (ch == L'c' || ch == L'C')
			copy();
		else if (ch == L'x' || ch == L'X')
			cut();
		else if (ch == L'v' || ch == L'V')
			paste();
	}
}

void Edit::eventPaint(PaintEvent* event)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	Canvas& canvas = event->getCanvas();
	FontMetric fm = getFontMetric();
	Rect rcInner = getInnerRect();

	bool hover = isEnable() && hasCapture();
	
	canvas.setBackground(ss->getColor(this, hover ? L"background-color-hover" : L"background-color"));
	canvas.fillRect(rcInner);

	canvas.setForeground(ss->getColor(this, L"border-color"));
	canvas.drawRect(rcInner);

	std::wstring text = getText();

	canvas.setForeground(ss->getColor(this, isEnable() ? L"color" : L"color-disabled"));

	int32_t h = fm.getHeight();
	int32_t x = dpi96(4);
	int32_t y = dpi96(4);
	int32_t caretX = 0;

	for (int32_t i = 0; i < text.length(); ++i)
	{
		int32_t w = fm.getAdvance(text[i], 0);

		if (i >= m_selectionStart && i < m_selectionEnd)
		{
			canvas.setBackground(ss->getColor(this, L"background-color-selection"));
			canvas.fillRect(Rect(
				x, rcInner.top + 1,
				x + w, rcInner.bottom - 1
			));			
		}

		wchar_t chs[2] = { text[i], 0 };
		canvas.drawText(Point(x, y), chs);

		if (i == m_caret)
			caretX = x;

		x += w;
	}

	if (m_caret >= text.length())
		caretX = x;

	bool caretVisible = hasFocus() && !m_readOnly && m_caretBlink;

	if (caretVisible)
	{
		canvas.setForeground(ss->getColor(this, L"color"));
		canvas.drawLine(Point(caretX, y), Point(caretX, y + h));
	}

	event->consume();
}

void Edit::eventTimer(TimerEvent* event)
{
	if (!hasFocus() && !m_caretBlink)
		return;

	m_caretBlink = !m_caretBlink;
	update();
}

	}
}
