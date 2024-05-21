/*
 * TRAKTOR
 * Copyright (c) 2023-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/ToolBar/ToolBar.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class MiniButton;

/*! Caption bar control.
 * \ingroup UI
 */
class T_DLLCLASS CaptionBar : public ToolBar
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, uint32_t style = WsNone);

	virtual Size getPreferredSize(const Size& hint) const override;

private:
	Ref< MiniButton > m_buttonMinimize;
	Ref< MiniButton > m_buttonMaximizeOrRestore;
	Ref< MiniButton > m_buttonClose;
	Point m_mousePosition;
	Rect m_parentRect;
	bool m_haveCapture = false;

	void eventButtonClick(ButtonClickEvent* event);

	void eventMouseButtonDown(MouseButtonDownEvent* event);

	void eventMouseButtonUp(MouseButtonUpEvent* event);

	void eventMouseDoubleClick(MouseDoubleClickEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventSize(SizeEvent* event);
};

}
