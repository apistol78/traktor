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

/*! Color slider control.
 * \ingroup UI
 */
class T_DLLCLASS ColorSliderControl : public Widget
{
	T_RTTI_CLASS;

public:
	struct IGradient : public Object
	{
		virtual Color4ub get(int at) const = 0;
	};

	bool create(Widget* parent, int style, IGradient* gradient);

	void setMarker(int32_t marker);

	virtual Size getPreferredSize(const Size& hint) const override;

	void updateGradient();

private:
	Ref< IGradient > m_gradient;
	Ref< drawing::Image > m_gradientImage;
	Ref< Bitmap > m_gradientBitmap;
	int32_t m_marker;

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventPaint(PaintEvent* event);
};

	}
}

