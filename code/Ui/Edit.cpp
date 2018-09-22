/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <cwctype>
#include "Core/Log/Log.h"
#include "Ui/Application.h"
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

void Edit::setSelection(int from, int to)
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

void Edit::setBorderColor(const Color4ub& borderColor)
{
}

void Edit::setText(const std::wstring& text)
{
	Widget::setText(text);
	update();
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
	if (mx >= x)
	{
		m_caret = -1;

		std::wstring text = getText();
		FontMetric fm = getFontMetric();

		for (int32_t i = 0; i < text.length(); ++i)
		{
			int32_t a = fm.getAdvance(text[i], 0);
			if (mx >= x && mx <= x + a)
			{
				if (mx <= x + a / 2)
					m_caret = i;
				else
					m_caret = i + 1;
				break;
			}
			x += a;
		}
		if (m_caret < 0)
			m_caret = int32_t(text.length());
	}
	else
		m_caret = 0;
}

void Edit::eventKeyDown(KeyDownEvent* event)
{
	switch (event->getVirtualKey())
	{
	case VkLeft:
		{
			m_caret = std::max< int32_t >(m_caret - 1 , 0);
			update();
		}
		break;

	case VkRight:
		{
			std::wstring text = getText();
			m_caret = std::min< int32_t >(m_caret + 1, text.length());
			update();
		}
		break;

	case VkHome:
		{
			m_caret = 0;
			update();
		}
		break;
		
	case VkEnd:
		{
			std::wstring text = getText();
			m_caret = text.length();
			update();
		}
		break;

	default:
		break;
	}
}

void Edit::eventKey(KeyEvent* event)
{
	wchar_t ch = event->getCharacter();
	std::wstring text = getText();

	if (ch != 8 && ch != 127)
	{
		if (m_caret >= text.length())
			text += ch;
		else
			text = text.substr(0, m_caret) + ch + text.substr(m_caret);
		++m_caret;
	}
	else if (ch == 8)
	{
		if (m_caret > 0)
		{
			text = text.substr(0, m_caret - 1) + text.substr(m_caret);
			--m_caret;
		}
	}

	// Ensure caret position is clamped within text limits.
	m_caret = std::min< int32_t >(m_caret, text.length());
	m_caret = std::max< int32_t >(m_caret, 0);

	if (m_validator == nullptr || m_validator->validate(text) != EditValidator::VrInvalid)
	{
		setText(text);

		ContentChangeEvent contentChangeEvent(this);
		raiseEvent(&contentChangeEvent);
	}

	update();
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

	canvas.setForeground(ss->getColor(this, isEnabled() ? L"color" : L"color-disabled"));

	int32_t h = fm.getHeight();
	int32_t x = dpi96(4);
	int32_t y = dpi96(4);
	int32_t caretX = 0;

	for (int32_t i = 0; i < text.length(); ++i)
	{
		wchar_t chs[2] = { text[i], 0 };
		canvas.drawText(Point(x, y), chs);

		if (i == m_caret)
			caretX = x;

		x += fm.getAdvance(text[i], 0);
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
