/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/MathUtils.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/ColorPicker/ColorEvent.h"
#include "Ui/ColorPicker/ColorSliderControl.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ColorSliderControl", ColorSliderControl, Widget)

bool ColorSliderControl::create(Widget* parent, int style, IGradient* gradient)
{
	if (!Widget::create(parent, style))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &ColorSliderControl::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &ColorSliderControl::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &ColorSliderControl::eventMouseMove);
	addEventHandler< PaintEvent >(this, &ColorSliderControl::eventPaint);

	const int32_t sliderWidth = dpi96(24);
	const int32_t sliderHeight = dpi96(256);

	m_gradient = gradient;
	m_gradientImage = new drawing::Image(drawing::PixelFormat::getR8G8B8(), sliderWidth, sliderHeight);
	m_gradientBitmap = new Bitmap(sliderWidth, sliderHeight);
	m_marker = 0;

	updateGradient();
	update();

	return true;
}

void ColorSliderControl::setMarker(int32_t marker)
{
	m_marker = marker;
	update();
}

Size ColorSliderControl::getPreferredSize(const Size& hint) const
{
	const int32_t sliderWidth = dpi96(24);
	const int32_t sliderHeight = dpi96(256);
	return Size(sliderWidth, sliderHeight);
}

void ColorSliderControl::updateGradient()
{
	const int32_t sliderWidth = dpi96(24);
	const int32_t sliderHeight = dpi96(256);

	for (int y = 0; y < sliderHeight; ++y)
	{
		Color4ub color = m_gradient->get((y * 256) / sliderHeight);
		for (int x = 0; x < sliderWidth; ++x)
		{
			Color4ub checkerColor = (((x >> 2) & 1) ^ ((y >> 2) & 1)) ? Color4ub(180, 180, 180) : Color4ub(80, 80, 80);
			Color4ub gradientColor = lerp(checkerColor, color, color.a / 255.0f);

			float rgba[4];
			gradientColor.getRGBA32F(rgba);

			m_gradientImage->setPixelUnsafe(x, y, Color4f(rgba));
		}
	}

	m_gradientBitmap->copyImage(m_gradientImage);
}

void ColorSliderControl::eventButtonDown(MouseButtonDownEvent* event)
{
	const int32_t sliderHeight = dpi96(256);
	int32_t y = event->getPosition().y;
	Color4ub color = m_gradient->get((y * 256) / sliderHeight);
	ColorEvent colorEvent(this, color);
	raiseEvent(&colorEvent);
	setCapture();
}

void ColorSliderControl::eventButtonUp(MouseButtonUpEvent* event)
{
	releaseCapture();
}

void ColorSliderControl::eventMouseMove(MouseMoveEvent* event)
{
	if (!hasCapture())
		return;

	const int32_t sliderHeight = dpi96(256);
	m_marker = clamp< int32_t >((event->getPosition().y * 256) / sliderHeight, 0, 255);

	Color4ub color = m_gradient->get(m_marker);
	ColorEvent colorEvent(this, color);
	raiseEvent(&colorEvent);

	update();
}

void ColorSliderControl::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect innerRect = getInnerRect();

	canvas.drawBitmap(
		Point(0, 0),
		Point(0, 0),
		m_gradientBitmap->getSize(),
		m_gradientBitmap
	);

	const int32_t y = dpi96(m_marker);

	canvas.setBackground(Color4ub(0, 0, 0, 255));
	canvas.fillRect(Rect(
		innerRect.left, y - 1,
		innerRect.right, y + 1
	));

	event->consume();
}

	}
}
