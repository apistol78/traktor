/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Range.h"
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

/*! RangeSlider
 * \ingroup UI
 */
class T_DLLCLASS RangeSlider : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, uint32_t style = WsNone);

	void setLimits(const Range< int32_t >& limits);

	void setRange(const Range< int32_t >& range);

	const Range< int32_t >& getRange() const;

	virtual Size getPreferredSize(const Size& hint) const override final;

	virtual Size getMaximumSize() const override final;

private:
	Range< int32_t > m_limits = { 0, 100 };
	Range< int32_t > m_range = { 0, 100 };
	int32_t m_drag = -1;
	bool m_hover = false;

	void eventMouseTrack(MouseTrackEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventPaint(PaintEvent* event);
};

}
