/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
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

namespace traktor
{
	namespace ui
	{

/*! Slider
 * \ingroup UI
 */
class T_DLLCLASS Slider : public Widget
{
	T_RTTI_CLASS;

public:
	Slider();

	bool create(Widget* parent, int32_t style = WsNone);

	void setRange(int32_t minValue, int32_t maxValue);

	void setValue(int32_t value);

	int32_t getValue() const;

	virtual Size getPreferredSize(const Size& hint) const override final;

	virtual Size getMaximumSize() const override final;

private:
	Range< int32_t > m_range;
	int32_t m_value;
	bool m_drag;

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}

