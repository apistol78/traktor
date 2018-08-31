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

	addEventHandler< KeyDownEvent >(this, &Edit::eventKeyDown);
	addEventHandler< KeyEvent >(this, &Edit::eventKey);
	addEventHandler< PaintEvent >(this, &Edit::eventPaint);
	addEventHandler< TimerEvent >(this, &Edit::eventTimer);

	startTimer(500);
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

Size Edit::getPreferedSize() const
{
	const int32_t height = getFont().getPixelSize() + dpi96(4) * 2;
	return Size(dpi96(200), height);
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

	default:
		break;
	}
}

void Edit::eventKey(KeyEvent* event)
{
	wchar_t ch = event->getCharacter();
	std::wstring text = getText();

	if (ch != 8)
	{
		if (m_caret >= text.length())
			text += ch;
		else
			text = text.substr(0, m_caret) + ch + text.substr(m_caret);
		++m_caret;
	}
	else
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
		setText(text);

	update();
}

void Edit::eventPaint(PaintEvent* event)
{
	const StyleSheet* ss = Application::getInstance()->getStyleSheet();
	Canvas& canvas = event->getCanvas();
	
	Rect rcInner = getInnerRect();
	
	canvas.setBackground(ss->getColor(this, L"background-color"));
	canvas.fillRect(rcInner);

	canvas.setForeground(ss->getColor(this, L"border-color"));
	canvas.drawRect(rcInner);

	std::wstring text = getText();

	canvas.setForeground(ss->getColor(this, L"color"));

	int32_t h = getFont().getPixelSize();
	int32_t x = 0;
	int32_t y = (rcInner.getHeight() - h) / 2;
	int32_t caretX = 0;

	for (int32_t i = 0; i < text.length(); ++i)
	{
		wchar_t chs[2] = { text[i], 0 };
		canvas.drawText(Point(x, y), chs);

		if (i == m_caret)
			caretX = x;

		x += canvas.getTextExtent(chs).cx;
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
	m_caretBlink = !m_caretBlink;
	update();
}

	}
}
