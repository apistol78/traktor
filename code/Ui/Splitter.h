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
	bool create(Widget* parent, bool vertical = true, int position = 100, bool relative = false, int border = 16);

	virtual void update(const Rect* rc = 0, bool immediate = false) override;

	virtual Size getMinimumSize() const override;

	virtual Size getPreferredSize(const Size& hint) const override;

	virtual Size getMaximumSize() const override;

	void setPosition(int position);

	int getPosition() const;

	Ref< Widget > getLeftWidget() const;

	Ref< Widget > getRightWidget() const;

private:
	bool m_vertical;
	int m_position;
	bool m_negative;
	bool m_relative;
	int m_border;
	bool m_drag;

	int getAbsolutePosition() const;

	void setAbsolutePosition(int position);

	void eventMouseMove(MouseMoveEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventSize(SizeEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}

