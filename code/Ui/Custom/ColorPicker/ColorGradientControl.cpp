#include "Ui/Bitmap.h"
#include "Ui/Custom/ColorPicker/ColorEvent.h"
#include "Ui/Custom/ColorPicker/ColorGradientControl.h"
#include "Ui/Custom/ColorPicker/ColorUtilities.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ColorGradientControl", ColorGradientControl, Widget)

bool ColorGradientControl::create(Widget* parent, int style, const Color4ub& color)
{
	if (!Widget::create(parent, style))
		return false;

	addEventHandler< MouseButtonDownEvent >(this, &ColorGradientControl::eventButtonDown);
	addEventHandler< MouseButtonUpEvent >(this, &ColorGradientControl::eventButtonUp);
	addEventHandler< MouseMoveEvent >(this, &ColorGradientControl::eventMouseMove);
	addEventHandler< PaintEvent >(this, &ColorGradientControl::eventPaint);

	m_gradientBitmap = new Bitmap(256, 256);

	setColor(color, true);
	update();

	return true;
}

Size ColorGradientControl::getPreferedSize() const
{
	return Size(256, 256);
}

void ColorGradientControl::setColor(const Color4ub& color, bool updateCursor)
{
	float hsv[3];
	RGBtoHSV(color, hsv);

	m_hue = hsv[0];

	if (updateCursor)
	{
		m_cursor.x = int(hsv[2] * 255.0f);
		m_cursor.y = int((1.0f - hsv[1]) * 255.0f);
	}

	updateGradientImage();
}

Color4ub ColorGradientControl::getColor() const
{
	return m_gradientBitmap->getPixel(m_cursor.x, m_cursor.y);
}

void ColorGradientControl::updateGradientImage()
{
	Color4ub color;
	float hsv[3];

	for (int y = 0; y < 256; ++y)
	{
		for (int x = 0; x < 256; ++x)
		{
			hsv[0] = m_hue;
			hsv[1] = 1.0f - float(y) / 255.0f;
			hsv[2] = float(x) / 255.0f;

			HSVtoRGB(hsv, color);

			m_gradientBitmap->setPixel(x, y, color);
		}
	}
}

void ColorGradientControl::eventButtonDown(MouseButtonDownEvent* event)
{
	m_cursor = event->getPosition();
	
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

	m_cursor = event->getPosition();
	
	if (m_cursor.x < 0)
		m_cursor.x = 0;
	if (m_cursor.x > 255)
		m_cursor.x = 255;
	if (m_cursor.y < 0)
		m_cursor.y = 0;
	if (m_cursor.y > 255)
		m_cursor.y = 255;

	ColorEvent colorEvent(this, getColor());
	raiseEvent(&colorEvent);

	update();
}

void ColorGradientControl::eventPaint(PaintEvent* event)
{
	Canvas& canvas = event->getCanvas();

	canvas.drawBitmap(
		Point(0, 0),
		Point(0, 0),
		Size(256, 256),
		m_gradientBitmap
	);

	Color4ub color = m_gradientBitmap->getPixel(m_cursor.x, m_cursor.y);

	int average = (color.r + color.g + color.b) / 3;
	if (average < 128)
		canvas.setForeground(Color4ub(255, 255, 255));
	else
		canvas.setForeground(Color4ub(0, 0, 0));

	canvas.drawCircle(m_cursor, 5);

	event->consume();
}

		}
	}
}
