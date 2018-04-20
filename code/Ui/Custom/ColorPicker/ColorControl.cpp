/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Custom/ColorPicker/ColorControl.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const uint32_t c_width = 32;
const uint32_t c_height = 24;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ColorControl", ColorControl, Widget)

bool ColorControl::create(Widget* parent, int style)
{
	if (!Widget::create(parent, style))
		return false;

	addEventHandler< PaintEvent >(this, &ColorControl::eventPaint);

	const int32_t width = dpi96(c_width);
	const int32_t height = dpi96(c_height);

	m_preview = new ui::Bitmap(width, height);
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
			m_preview->setPixel(x, y, previewColor);
		}
	}

	update();
}

Color4ub ColorControl::getColor() const
{
	return m_color;
}

Size ColorControl::getPreferedSize() const
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
		m_preview->getSize(),
		m_preview
	);

	event->consume();
}

		}
	}
}
