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

/*! Scroll bar.
 * \ingroup UI
 */
class T_DLLCLASS ScrollBar : public Widget
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsHorizontal = 0,
		WsVertical = WsUser
	};

	ScrollBar();

	bool create(Widget* parent, int32_t style = WsHorizontal);

	void setRange(int32_t range);

	int32_t getRange() const;

	void setPage(int32_t page);

	int32_t getPage() const;

	void setPosition(int32_t position);

	int32_t getPosition() const;

	virtual Size getPreferredSize(const Size& hint) const override;

	virtual Size getMaximumSize() const override;

private:
	bool m_vertical;
	int32_t m_range;
	int32_t m_page;
	int32_t m_position;
	int32_t m_trackOffset;

	void eventMouseButtonDown(MouseButtonDownEvent* event);

	void eventMouseButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}

