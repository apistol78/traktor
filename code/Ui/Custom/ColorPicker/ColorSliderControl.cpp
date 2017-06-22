/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/MathUtils.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Custom/ColorPicker/ColorEvent.h"
#include "Ui/Custom/ColorPicker/ColorSliderControl.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ColorSliderControl", ColorSliderControl, Widget)

bool ColorSliderControl::create(Widget* parent, int style, IGradient* gradient)
{
	if (!Widget::create(parent, style))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &ColorSliderControl::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &ColorSliderControl::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &ColorSliderControl::eventMouseMove);
	addEventHandler< PaintEvent >(this, &ColorSliderControl::eventPaint);

	const int32_t sliderWidth = scaleBySystemDPI(24);
	const int32_t sliderHeight = scaleBySystemDPI(256);

	m_gradient = gradient;
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

Size ColorSliderControl::getPreferedSize() const
{
	const int32_t sliderWidth = scaleBySystemDPI(24);
	const int32_t sliderHeight = scaleBySystemDPI(256);
	return Size(sliderWidth, sliderHeight);
}

void ColorSliderControl::updateGradient()
{
	const int32_t sliderWidth = scaleBySystemDPI(24);
	const int32_t sliderHeight = scaleBySystemDPI(256);
	for (int y = 0; y < sliderHeight; ++y)
	{
		Color4ub color = m_gradient->get((y * 256) / sliderHeight);
		for (int x = 0; x < sliderWidth; ++x)
		{
			Color4ub checkerColor = (((x >> 2) & 1) ^ ((y >> 2) & 1)) ? Color4ub(180, 180, 180) : Color4ub(80, 80, 80);
			Color4ub gradientColor = lerp(checkerColor, color, color.a / 255.0f);
			m_gradientBitmap->setPixel(x, y, gradientColor);
		}
	}
}

void ColorSliderControl::eventButtonDown(MouseButtonDownEvent* event)
{
	const int32_t sliderHeight = scaleBySystemDPI(256);
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

	const int32_t sliderHeight = scaleBySystemDPI(256);
	m_marker = clamp< int32_t >((event->getPosition().y * 256) / sliderHeight, 0, 255);

	Color4ub color = m_gradient->get(m_marker);
	ColorEvent colorEvent(this, color);
	raiseEvent(&colorEvent);

	update();
}

void ColorSliderControl::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();
	Rect innerRect = getInnerRect();

	canvas.drawBitmap(
		Point(0, 0),
		Point(0, 0),
		m_gradientBitmap->getSize(),
		m_gradientBitmap
	);

	int32_t y = scaleBySystemDPI(m_marker);

	canvas.setBackground(Color4ub(0, 0, 0, 255));
	canvas.fillRect(Rect(
		innerRect.left, y - 1,
		innerRect.right, y + 1
	));

	event->consume();
}

		}
	}
}
