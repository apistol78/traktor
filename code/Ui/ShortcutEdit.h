/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Shortcut edit control.
 * \ingroup UI
 */
class T_DLLCLASS ShortcutEdit : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, int32_t keyState, VirtualKey virtualKey, uint32_t style = WsNone);

	virtual std::wstring getText() const override;

	virtual Size getPreferredSize(const Size& hint) const override;

	void set(int32_t keyState, VirtualKey virtualKey);

	int32_t getKeyState() const;

	VirtualKey getVirtualKey() const;

private:
	int32_t m_keyState = 0;
	VirtualKey m_virtualKey = VkNull;

	void eventKeyDown(KeyDownEvent* event);

	void eventPaint(PaintEvent* event);

	void eventFocus(FocusEvent* event);
};

}
