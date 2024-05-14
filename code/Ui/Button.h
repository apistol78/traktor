/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

/*! Button
 * \ingroup UI
 */
class T_DLLCLASS Button : public Widget
{
	T_RTTI_CLASS;

public:
	constexpr static uint32_t WsDefaultButton = WsUser;
	constexpr static uint32_t WsToggle = (WsUser << 1);

	bool create(Widget* parent, const std::wstring& text, uint32_t style = WsNone);

	virtual Size getPreferredSize(const Size& hint) const override;

	virtual Size getMaximumSize() const override;

private:
	bool m_pushed = false;
	bool m_hover = false;

	void eventMouseTrack(MouseTrackEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventPaint(PaintEvent* event);
};

}
