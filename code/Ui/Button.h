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

namespace traktor
{
	namespace ui
	{

/*! Button
 * \ingroup UI
 */
class T_DLLCLASS Button : public Widget
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsDefaultButton	= WsUser,
		WsToggle = (WsUser << 1)
	};

	Button();

	bool create(Widget* parent, const std::wstring& text, int style = WsNone);

	virtual void setText(const std::wstring& text) override;

	virtual Size getPreferredSize(const Size& hint) const override;

	virtual Size getMaximumSize() const override;

private:
	Size m_preferedSize;
	bool m_pushed;
	bool m_hover;

	void eventMouseTrack(MouseTrackEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}

