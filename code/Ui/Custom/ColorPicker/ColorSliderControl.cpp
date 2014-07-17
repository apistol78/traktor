#include "Ui/Bitmap.h"
#include "Ui/Custom/ColorPicker/ColorEvent.h"
#include "Ui/Custom/ColorPicker/ColorSliderControl.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{
			namespace
			{

const int c_sliderWidth = 24;

			}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ColorSliderControl", ColorSliderControl, Widget)

bool ColorSliderControl::create(Widget* parent, int style, IGradient* gradient)
{
	if (!Widget::create(parent, style))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &ColorSliderControl::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &ColorSliderControl::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &ColorSliderControl::eventMouseMove);
	addEventHandler< PaintEvent >(this, &ColorSliderControl::eventPaint);

	m_gradient = gradient;
	m_gradientBitmap = new Bitmap(c_sliderWidth, 256);

	updateGradient();
	update();

	return true;
}

Size ColorSliderControl::getPreferedSize() const
{
	return Size(c_sliderWidth, 256);
}

void ColorSliderControl::updateGradient()
{
	for (int y = 0; y < 256; ++y)
	{
		Color4ub color = m_gradient->get(y);
		for (int x = 0; x < c_sliderWidth; ++x)
		{
			Color4ub checkerColor = (((x >> 2) & 1) ^ ((y >> 2) & 1)) ? Color4ub(180, 180, 180) : Color4ub(80, 80, 80);
			Color4ub gradientColor = lerp(checkerColor, color, color.a / 255.0f);
			m_gradientBitmap->setPixel(x, y, gradientColor);
		}
	}
}

void ColorSliderControl::eventButtonDown(MouseButtonDownEvent* event)
{
	int32_t y = event->getPosition().y;
	T_ASSERT (y >= 0 && y < 256);

	Color4ub color = m_gradient->get(y);

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

	int32_t y = event->getPosition().y;
	if (y < 0)
		y = 0;
	if (y > 255)
		y = 255;

	Color4ub color = m_gradient->get(y);

	ColorEvent colorEvent(this, color);
	raiseEvent(&colorEvent);
}

void ColorSliderControl::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();

	canvas.drawBitmap(
		Point(0, 0),
		Point(0, 0),
		Size(c_sliderWidth, 256),
		m_gradientBitmap
	);

	event->consume();
}

		}
	}
}
