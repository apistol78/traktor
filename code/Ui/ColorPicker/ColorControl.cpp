/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/ColorPicker/ColorControl.h"

namespace traktor::ui
{
	namespace
	{

const Unit c_width = 32_ut;
const Unit c_height = 24_ut;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ColorControl", ColorControl, Widget)

bool ColorControl::create(Widget* parent, uint32_t style)
{
	if (!Widget::create(parent, style | WsFocus))
		return false;

	addEventHandler< SizeEvent >(this, &ColorControl::eventSize);
	addEventHandler< PaintEvent >(this, &ColorControl::eventPaint);

	const int32_t width = pixel(c_width);
	const int32_t height = pixel(c_height);

	m_previewImage = new drawing::Image(drawing::PixelFormat::getR8G8B8(), width, height);
	m_previewBitmap = new ui::Bitmap(width, height);

	setColor(Color4ub(0, 0, 0));
	return true;
}

void ColorControl::setColor(const Color4ub& color)
{
	m_color = color;
	updatePreview();
	update();
}

Color4ub ColorControl::getColor() const
{
	return m_color;
}

Size ColorControl::getPreferredSize(const Size& hint) const
{
	const int32_t width = pixel(c_width);
	const int32_t height = pixel(c_height);
	return Size(width, height);
}

void ColorControl::updatePreview()
{
	const Size sz = getInnerRect().getSize();

	if (m_previewImage)
	{
		if (m_previewImage->getWidth() != sz.cx || m_previewImage->getHeight() != sz.cy)
		{
			m_previewImage = nullptr;
			m_previewBitmap = nullptr;
		}
	}

	if (!m_previewImage)
	{
		m_previewImage = new drawing::Image(drawing::PixelFormat::getR8G8B8(), sz.cx, sz.cy);
		m_previewBitmap = new ui::Bitmap(sz.cx, sz.cy);
	}

	for (int32_t y = 0; y < sz.cy; ++y)
	{
		for (int32_t x = 0; x < sz.cx; ++x)
		{
			const Color4ub checkerColor = (((x >> 2) & 1) ^ ((y >> 2) & 1)) ? Color4ub(180, 180, 180) : Color4ub(80, 80, 80);
			const Color4ub previewColor = lerp(checkerColor, m_color, m_color.a / 255.0f);

			float rgba[4];
			previewColor.getRGBA32F(rgba);

			m_previewImage->setPixel(x, y, Color4f(rgba));
		}
	}

	m_previewBitmap->copyImage(m_previewImage);
}

void ColorControl::eventSize(SizeEvent* event)
{
	updatePreview();
	update();
}

void ColorControl::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();

	canvas.drawBitmap(
		Point(0, 0),
		Point(0, 0),
		m_previewBitmap->getSize(this),
		m_previewBitmap
	);

	event->consume();
}

}
