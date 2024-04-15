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

/*! Horizontal or vertical splitter.
 * \ingroup UI
 */
class T_DLLCLASS Splitter : public Widget
{
	T_RTTI_CLASS;

public:
	Splitter();

	/*! Create splitter control.
	 *
	 * \param parent Parent widget.
	 * \param vertical Orientation of splitter.
	 * \param position Initial position of splitter.
	 * \param relative If position is scaled relatively when Splitter is resized.
	 * \param border Clamping border, distance from extents in pixels.
	 */
	bool create(Widget* parent, bool vertical = true, Unit position = 100_ut, bool relative = false, Unit border = 16_ut);

	virtual void update(const Rect* rc = nullptr, bool immediate = false) override;

	virtual Size getMinimumSize() const override;

	virtual Size getPreferredSize(const Size& hint) const override;

	virtual Size getMaximumSize() const override;

	void setVertical(bool vertical);

	void setPosition(Unit position);

	Unit getPosition() const;

	Ref< Widget > getLeftWidget() const;

	Ref< Widget > getRightWidget() const;

private:
	bool m_vertical;
	Unit m_position;
	bool m_negative;
	bool m_relative;
	Unit m_border;
	bool m_drag;

	int32_t getAbsolutePosition() const;

	void setAbsolutePosition(int32_t position);

	void eventMouseMove(MouseMoveEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventSize(SizeEvent* event);

	void eventPaint(PaintEvent* event);
};

}
