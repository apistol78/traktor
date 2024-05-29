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
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Edit.h"
#include "Ui/EditValidator.h"
#include "Ui/StyleSheet.h"

namespace traktor::ui
{
	namespace
	{

bool isWord(wchar_t ch)
{
	return 
		(ch >= L'0' && ch <= L'9') ||
		(ch >= L'a' && ch <= L'z') ||
		(ch >= L'A' && ch <= L'Z');
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Edit", Edit, Widget)

Edit::Edit()
:	m_borderColor(0, 0, 0, 0)
,	m_selectionStart(-1)
,	m_selectionEnd(-1)
,	m_caret(0)
,	m_caretBlink(true)
,	m_acceptTab(false)
,	m_readOnly(false)
,	m_hover(false)
{
}

bool Edit::create(Widget* parent, const std::wstring& text, uint32_t style, const EditValidator* validator)
{
	if (!Widget::create(parent, style | WsFocus | WsDoubleBuffer))
		return false;

	if ((m_validator = validator) != nullptr)
	{
		if (m_validator->validate(text) == EditValidator::VrInvalid)
			return false;
	}

	m_acceptTab = (bool)((style & WsWantAllInput) != 0);
	m_readOnly = (bool)((style & WsReadOnly) != 0);

	addEventHandler< FocusEvent >(this, &Edit::eventFocus);
	addEventHandler< MouseTrackEvent >(this, &Edit::eventMouseTrack);
	addEventHandler< MouseButtonDownEvent >(this, &Edit::eventButtonDown);
	addEventHandler< MouseDoubleClickEvent >(this, &Edit::eventDoubleClick);
	addEventHandler< PaintEvent >(this, &Edit::eventPaint);
	addEventHandler< TimerEvent >(this, &Edit::eventTimer);

	if (!m_readOnly)
	{
		addEventHandler< KeyDownEvent >(this, &Edit::eventKeyDown);
		addEventHandler< KeyEvent >(this, &Edit::eventKey);
	}

	setText(text);
	return true;
}

void Edit::destroy()
{
	// If control is destroyed while having focus then no event is issued
	// for loosing focus, so we must also enable global event handlers here.
	if (getIWidget() != nullptr && hasFocus())
		Application::getInstance()->enableEventHandlers< KeyDownEvent >();

	Widget::destroy();
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
	const std::wstring text = getText();
	if (!text.empty())
	{
		m_selectionStart = 0;
		m_selectionEnd = (int32_t)text.length();
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
	// Remove all occurances of invalid characters.
	std::wstring nwst = text;
	auto it = std::remove_if(nwst.begin(), nwst.end(), [](wchar_t ch) {
		return ch == L'\n' || ch == L'\r';
	});
	nwst.erase(it, nwst.end());

	std::wstring current = getText();
	if (!haveSelection())
	{
		if (m_caret >= current.length())
		{
			current += nwst;
			m_caret = (int32_t)current.length();
		}
		else
		{
			current = current.substr(0, m_caret) + nwst + current.substr(m_caret);
			m_caret = m_caret + (int32_t)nwst.length();
		}
	}
	else
	{
		current = current.substr(0, m_selectionStart) + nwst + current.substr(m_selectionEnd);
		m_caret = m_selectionStart + (int32_t)nwst.length();
		deselect();
	}

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
		if (clipboard->getContentType() == ClipboardContent::Text)
			insert(clipboard->getText());
	}
}

void Edit::setBorderColor(const Color4ub& borderColor)
{
	m_borderColor = borderColor;
	update();
}

void Edit::setText(const std::wstring& text)
{
	Widget::setText(text);

	// Ensure caret position is clamped within text limits.
	m_caret = std::min< int32_t >(m_caret, (int32_t)text.length());
	m_caret = std::max< int32_t >(m_caret, 0);

	deselect();
}

Size Edit::getPreferredSize(const Size& hint) const
{
	const int32_t height = getFontMetric().getHeight() + pixel(4_ut) * 2;
	return Size(pixel(100_ut), height);
}

Size Edit::getMaximumSize() const
{
	const int32_t height = getFontMetric().getHeight() + pixel(4_ut) * 2;
	return Size(65535, height);
}

void Edit::eventFocus(FocusEvent* event)
{
	m_caretBlink = true;
	if (event->gotFocus())
	{
		// Disable global KeyDown events while Edit got focus to prevent
		// global hooks, such as ShortcutTable, to receive events.
		Application::getInstance()->disableEventHandlers< KeyDownEvent >();
		startTimer(500);
	}
	else
	{
		Application::getInstance()->enableEventHandlers< KeyDownEvent >();
		stopTimer();
		deselect();
	}
	update();
}

void Edit::eventMouseTrack(MouseTrackEvent* event)
{
	m_hover = event->entered();
	update();
}

void Edit::eventButtonDown(MouseButtonDownEvent* event)
{
	const int32_t mx = event->getPosition().x;
	int32_t x = pixel(4_ut);

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

void Edit::eventDoubleClick(MouseDoubleClickEvent* event)
{
	selectAll();
}

void Edit::eventKeyDown(KeyDownEvent* event)
{
	int32_t caret = m_caret;
	bool modified = false;

	switch (event->getVirtualKey())
	{
	case VkLeft:
		{
			if ((event->getKeyState() & KsControl) == 0)
				--caret;
			else
			{
				const std::wstring text = getText();
				for (--caret; caret > 0 && isWord(text[caret - 1]); --caret)
					;
			}
			caret = std::max< int32_t >(caret, 0);
			modified = true;
		}
		break;

	case VkRight:
		{
			const std::wstring text = getText();
			if ((event->getKeyState() & KsControl) == 0)
				++caret;
			else
			{
				for (++caret; caret < (int32_t)text.length() && isWord(text[caret]); ++caret)
					;
			}
			caret = std::min< int32_t >(caret, (int32_t)text.length());
			modified = true;
		}
		break;

	case VkHome:
		{
			caret = 0;
			modified = true;
		}
		break;

	case VkEnd:
		{
			const std::wstring text = getText();
			caret = (int32_t)text.length();
			modified = true;
		}
		break;

	case VkDelete:
		{
			std::wstring text = getText();
			if (!haveSelection())
			{
				if (caret < (int32_t)text.length())
				{
					text = text.substr(0, caret) + text.substr(caret + 1);
				}
			}
			else
			{
				text = text.substr(0, m_selectionStart) + text.substr(m_selectionEnd);
				caret = m_selectionStart;
			}
			
			m_caret = caret;

			setText(text);
			deselect();
			update();
		}
		break;

	default:
		break;
	}

	if (modified)
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
	const wchar_t ch = event->getCharacter();

	// Ignore tab if we don't want all character inputs.
	if (!m_acceptTab && ch == L'\t')
		return;

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
				caret = m_selectionStart;
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
			{
				text = text.substr(0, m_selectionStart) + ch + text.substr(m_selectionEnd);
				caret = m_selectionStart;
			}

			++caret;
		}

		deselect();

		// Ensure caret position is clamped within text limits.
		caret = std::min< int32_t >(caret, (int32_t)text.length());
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
		if (event->getVirtualKey() == VkA)
			selectAll();
		else if (event->getVirtualKey() == VkC)
			copy();
		else if (event->getVirtualKey() == VkX)
			cut();
		else if (event->getVirtualKey() == VkV)
			paste();
	}
}

void Edit::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rcInner = getInnerRect();
	const StyleSheet* ss = getStyleSheet();
	const FontMetric fm = canvas.getFontMetric();

	if (isEnable(true))
		canvas.setBackground(ss->getColor(this, m_hover ? L"background-color-hover" : L"background-color"));
	else
		canvas.setBackground(ss->getColor(this, L"background-color-disabled"));
	canvas.fillRect(rcInner);

	if (m_borderColor.a != 0)
		canvas.setForeground(m_borderColor);
	else
		canvas.setForeground(ss->getColor(this, L"border-color"));
	canvas.drawRect(rcInner);

	const std::wstring text = getText();

	canvas.setForeground(ss->getColor(this, isEnable(true) ? L"color" : L"color-disabled"));

	const int32_t h = fm.getHeight();
	const int32_t y = (rcInner.getHeight() - h) / 2;
	int32_t x = pixel(4_ut);
	int32_t caretX = 0;

	for (int32_t i = 0; i < text.length(); ++i)
	{
		const int32_t w = fm.getAdvance(text[i], 0);

		if (i >= m_selectionStart && i < m_selectionEnd)
		{
			canvas.setBackground(ss->getColor(this, L"background-color-selection"));
			canvas.fillRect(Rect(
				x, rcInner.top + 1,
				x + w, rcInner.bottom - 1
			));
		}

		const wchar_t chs[2] = { text[i], 0 };
		canvas.drawText(Point(x, y), chs);

		if (i == m_caret)
			caretX = x;

		x += w;
	}

	if (m_caret >= text.length())
		caretX = x;

	const bool caretVisible = hasFocus() && !m_readOnly && m_caretBlink;
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
