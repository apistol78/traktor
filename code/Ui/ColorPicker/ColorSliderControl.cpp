/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
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

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ColorSliderControl", ColorSliderControl, Widget)

bool ColorSliderControl::create(Widget* parent, uint32_t style, IGradient* gradient)
{
	if (!Widget::create(parent, style))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &ColorSliderControl::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &ColorSliderControl::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &ColorSliderControl::eventMouseMove);
	addEventHandler< MouseTrackEvent >(this, &ColorSliderControl::eventMouseTrack);
	addEventHandler< PaintEvent >(this, &ColorSliderControl::eventPaint);

	const int32_t sliderWidth = pixel(24_ut);
	const int32_t sliderHeight = pixel(256_ut);

	m_gradient = gradient;
	m_gradientImage = new drawing::Image(drawing::PixelFormat::getR8G8B8(), sliderWidth, sliderHeight);
	m_gradientBitmap = new Bitmap(sliderWidth, sliderHeight);
	m_marker = 0;
	m_hover = false;

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
	const int32_t sliderWidth = pixel(24_ut);
	const int32_t sliderHeight = pixel(256_ut);
	return Size(sliderWidth, sliderHeight);
}

void ColorSliderControl::updateGradient()
{
	const int32_t sliderWidth = pixel(24_ut);
	const int32_t sliderHeight = pixel(256_ut);

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
	const int32_t sliderHeight = pixel(256_ut);
	m_marker = clamp< int32_t >((event->getPosition().y * 256) / sliderHeight, 0, 255);

	const Color4ub color = m_gradient->get(m_marker);

	ColorEvent colorEvent(this, color);
	raiseEvent(&colorEvent);

	setCapture();
	update();
}

void ColorSliderControl::eventButtonUp(MouseButtonUpEvent* event)
{
	releaseCapture();
}

void ColorSliderControl::eventMouseMove(MouseMoveEvent* event)
{
	if (hasCapture())
	{
		const int32_t sliderHeight = pixel(256_ut);
		m_marker = clamp< int32_t >((event->getPosition().y * 256) / sliderHeight, 0, 255);

		const Color4ub color = m_gradient->get(m_marker);

		ColorEvent colorEvent(this, color);
		raiseEvent(&colorEvent);
	}
	update();
}

void ColorSliderControl::eventMouseTrack(MouseTrackEvent* event)
{
	m_hover = event->entered();
	update();
}

void ColorSliderControl::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const Rect innerRect = getInnerRect();

	canvas.drawBitmap(
		Point(0, 0),
		Point(0, 0),
		m_gradientBitmap->getSize(this),
		m_gradientBitmap
	);

	const int32_t y = pixel(Unit(m_marker));
	const int32_t m = pixel(1_ut);

	canvas.setBackground(Color4ub(0, 0, 0, 255));
	canvas.fillRect(Rect(
		innerRect.left, y - m,
		innerRect.right, y + m
	));

	if (m_hover && !hasCapture())
	{
		const Point mousePosition = getMousePosition();
		canvas.setForeground(Color4ub(0, 0, 0, 128));
		canvas.drawLine(Point(innerRect.left, mousePosition.y), Point(innerRect.right, mousePosition.y));
	}

	event->consume();
}

}
