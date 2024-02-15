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
	namespace drawing
	{

class Image;

	}

	namespace ui
	{

class Bitmap;

/*! Color gradient control.
 * \ingroup UI
 */
class T_DLLCLASS ColorGradientControl : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, int style, const Color4ub& color);

	virtual Size getPreferredSize(const Size& hint) const override;

	void setPrimaryColor(const Color4ub& color);

	void setCursorColor(const Color4ub& color);

	Color4ub getColor() const;

private:
	float m_hue;
	Point m_cursor;
	Ref< drawing::Image > m_gradientImage;
	Ref< Bitmap > m_gradientBitmap;

	void updateGradientImage();

	Point clientToGradient(const Point& client) const;

	Point gradientToClient(const Point& gradient) const;

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}

