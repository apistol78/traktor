/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/ColorPicker/ColorEvent.h"
#include "Ui/ColorPicker/ColorGradientControl.h"
#include "Ui/ColorPicker/ColorUtilities.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ColorGradientControl", ColorGradientControl, Widget)

bool ColorGradientControl::create(Widget* parent, uint32_t style, const Color4ub& primaryColor)
{
	if (!Widget::create(parent, style | WsFocus))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &ColorGradientControl::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &ColorGradientControl::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &ColorGradientControl::eventMouseMove);
	addEventHandler< PaintEvent >(this, &ColorGradientControl::eventPaint);

	m_gradientImage = new drawing::Image(drawing::PixelFormat::getR8G8B8(), 256, 256);
	m_gradientBitmap = new Bitmap(256, 256);

	setPrimaryColor(primaryColor);
	update();

	return true;
}

Size ColorGradientControl::getPreferredSize(const Size& hint) const
{
	const int32_t size = pixel(256_ut);
	return Size(size, size);
}

void ColorGradientControl::setPrimaryColor(const Color4ub& color)
{
	float rgba[4];
	color.getRGBA32F(rgba);

	float hsv[3];
	RGBtoHSV(Color4f(rgba), hsv);

	m_hue = hsv[0];
	updateGradientImage();
}

void ColorGradientControl::setCursorColor(const Color4ub& color)
{
	float rgba[4];
	color.getRGBA32F(rgba);

	float hsv[3];
	RGBtoHSV(Color4f(rgba), hsv);
	
	m_cursor.x = int(hsv[2] * 255.0f);
	m_cursor.y = int((1.0f - hsv[1]) * 255.0f);
}

Color4ub ColorGradientControl::getColor() const
{
	Color4f clr;
	m_gradientImage->getPixelUnsafe(m_cursor.x, m_cursor.y, clr);
	return clr.toColor4ub();
}

void ColorGradientControl::updateGradientImage()
{
	Color4f color;
	float hsv[3];

	for (int y = 0; y < 256; ++y)
	{
		for (int x = 0; x < 256; ++x)
		{
			hsv[0] = m_hue;
			hsv[1] = 1.0f - float(y) / 255.0f;
			hsv[2] = float(x) / 255.0f;

			HSVtoRGB(hsv, color);

			m_gradientImage->setPixelUnsafe(x, y, color);
		}
	}

	m_gradientBitmap->copyImage(m_gradientImage);
}

Point ColorGradientControl::clientToGradient(const Point& client) const
{
	const Rect innerRect = getInnerRect();
	const Point pt = (client * Size(256, 256)) / innerRect.getSize();
	return Point(
		clamp(pt.x, 0, 255),
		clamp(pt.y, 0, 255)
	);
}

Point ColorGradientControl::gradientToClient(const Point& gradient) const
{
	const Rect innerRect = getInnerRect();
	return (gradient * innerRect.getSize()) / Size(256, 256);
}

void ColorGradientControl::eventButtonDown(MouseButtonDownEvent* event)
{
	m_cursor = clientToGradient(event->getPosition());

	ColorEvent colorEvent(this, getColor());
	raiseEvent(&colorEvent);

	update();
	setCapture();
}

void ColorGradientControl::eventButtonUp(MouseButtonUpEvent* event)
{
	releaseCapture();
}

void ColorGradientControl::eventMouseMove(MouseMoveEvent* event)
{
	if (!hasCapture())
		return;

	m_cursor = clientToGradient(event->getPosition());

	ColorEvent colorEvent(this, getColor());
	raiseEvent(&colorEvent);

	update();
}

void ColorGradientControl::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	const int32_t size = pixel(256_ut);

	canvas.drawBitmap(
		Point(0, 0),
		Size(size, size),
		Point(0, 0),
		Size(256, 256),
		m_gradientBitmap
	);

	const Color4ub color = getColor();
	const int32_t average = (color.r + color.g + color.b) / 3;
	if (average < 128)
		canvas.setForeground(Color4ub(255, 255, 255));
	else
		canvas.setForeground(Color4ub(0, 0, 0));

	canvas.drawCircle(gradientToClient(m_cursor), pixel(5_ut));

	event->consume();
}

}
