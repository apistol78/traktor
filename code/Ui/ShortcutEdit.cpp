/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/ShortcutEdit.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const Unit c_preferedWidth = 100_ut;
const Unit c_preferedHeight = 18_ut;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ShortcutEdit", ShortcutEdit, Widget)

ShortcutEdit::ShortcutEdit()
:	m_keyState(0)
,	m_virtualKey(VkNull)
{
}

bool ShortcutEdit::create(Widget* parent, int32_t keyState, VirtualKey virtualKey, uint32_t style)
{
	if (!Widget::create(parent, style))
		return false;

	m_keyState = keyState;
	m_virtualKey = virtualKey;

	addEventHandler< KeyDownEvent >(this, &ShortcutEdit::eventKeyDown);
	addEventHandler< PaintEvent >(this, &ShortcutEdit::eventPaint);
	addEventHandler< FocusEvent >(this, &ShortcutEdit::eventFocus);

	return true;
}

std::wstring ShortcutEdit::getText() const
{
	if (m_virtualKey == VkNull)
		return L"";

	std::wstring keyDesc = L"";

	if (m_keyState & ui::KsCommand)
	{
#if defined(__APPLE__)
		keyDesc = L"\x2318";
#else
		keyDesc = L"Ctrl";
#endif
	}
	else if (m_keyState & ui::KsControl)
		keyDesc = keyDesc.empty() ? L"Ctrl" : keyDesc + L"+Ctrl";
	if (m_keyState & ui::KsMenu)
		keyDesc = keyDesc.empty() ? L"Alt" : keyDesc + L"+Alt";
	if (m_keyState & ui::KsShift)
		keyDesc = keyDesc.empty() ? L"Shift" : keyDesc + L"+Shift";

	std::wstring keyName = ui::Application::getInstance()->translateVirtualKey(m_virtualKey);
	return keyDesc.empty() ? keyName : keyDesc + L", " + keyName;
}

Size ShortcutEdit::getPreferredSize(const Size& hint) const
{
	return Size(
		pixel(c_preferedWidth),
		pixel(c_preferedHeight)
	);
}

void ShortcutEdit::set(int32_t keyState, VirtualKey virtualKey)
{
	m_keyState = keyState;
	m_virtualKey = virtualKey;
}

int32_t ShortcutEdit::getKeyState() const
{
	return m_keyState;
}

VirtualKey ShortcutEdit::getVirtualKey() const
{
	return m_virtualKey;
}

void ShortcutEdit::eventKeyDown(KeyDownEvent* event)
{
	VirtualKey virtualKey = event->getVirtualKey();

	m_keyState = event->getKeyState();
	if (m_keyState & ui::KsControl)
	{
		m_keyState &= ~ui::KsControl;
		m_keyState |= ui::KsCommand;
	}

	m_virtualKey = virtualKey;

	update();

	ContentChangeEvent changeEvent(this);
	raiseEvent(&changeEvent);

	event->consume();
}

void ShortcutEdit::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect rc = getInnerRect();
	const StyleSheet* ss = getStyleSheet();

	if (!hasFocus())
		canvas.setBackground(ss->getColor(this, L"background-color"));
	else
		canvas.setBackground(ss->getColor(this, L"background-color-focus"));

	canvas.fillRect(rc);

	if (!hasFocus())
		canvas.setForeground(ss->getColor(this, L"color"));
	else
		canvas.setForeground(ss->getColor(this, L"color-focus"));

	canvas.drawText(rc.inflate(pixel(-8_ut), 0), getText(), AnLeft, AnCenter);

	event->consume();
}

void ShortcutEdit::eventFocus(FocusEvent* event)
{
	update();
}

	}
}
