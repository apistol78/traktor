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
#include "Ui/ColorPicker/ColorControl.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

const uint32_t c_width = 32;
const uint32_t c_height = 24;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ColorControl", ColorControl, Widget)

bool ColorControl::create(Widget* parent, int style)
{
	if (!Widget::create(parent, style))
		return false;

	addEventHandler< PaintEvent >(this, &ColorControl::eventPaint);

	const int32_t width = dpi96(c_width);
	const int32_t height = dpi96(c_height);

	m_previewImage = new drawing::Image(drawing::PixelFormat::getR8G8B8(), width, height);
	m_previewBitmap = new ui::Bitmap(width, height);

	setColor(Color4ub(0, 0, 0));
	return true;
}

void ColorControl::setColor(const Color4ub& color)
{
	const int32_t width = dpi96(c_width);
	const int32_t height = dpi96(c_height);

	m_color = color;
	for (int32_t y = 0; y < height; ++y)
	{
		for (int32_t x = 0; x < width; ++x)
		{
			Color4ub checkerColor = (((x >> 2) & 1) ^ ((y >> 2) & 1)) ? Color4ub(180, 180, 180) : Color4ub(80, 80, 80);
			Color4ub previewColor = lerp(checkerColor, color, color.a / 255.0f);

			float rgba[4];
			previewColor.getRGBA32F(rgba);

			m_previewImage->setPixel(x, y, Color4f(rgba));
		}
	}

	m_previewBitmap->copyImage(m_previewImage);
	update();
}

Color4ub ColorControl::getColor() const
{
	return m_color;
}

Size ColorControl::getPreferredSize(const Size& hint) const
{
	const int32_t width = dpi96(c_width);
	const int32_t height = dpi96(c_height);
	return Size(width, height);
}

void ColorControl::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();

	canvas.drawBitmap(
		Point(0, 0),
		Point(0, 0),
		m_previewBitmap->getSize(),
		m_previewBitmap
	);

	event->consume();
}

	}
}
