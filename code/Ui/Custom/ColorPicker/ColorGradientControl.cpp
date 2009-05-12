#include "Ui/Custom/ColorPicker/ColorGradientControl.h"
#include "Ui/Custom/ColorPicker/ColorUtilities.h"
#include "Ui/Custom/ColorPicker/ColorEvent.h"
#include "Ui/Bitmap.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/PaintEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.ColorGradientControl", ColorGradientControl, Widget)

bool ColorGradientControl::create(Widget* parent, int style, const Color& color)
{
	if (!Widget::create(parent, style))
		return false;

	addButtonDownEventHandler(createMethodHandler(this, &ColorGradientControl::eventButtonDown));
	addButtonUpEventHandler(createMethodHandler(this, &ColorGradientControl::eventButtonUp));
	addMouseMoveEventHandler(createMethodHandler(this, &ColorGradientControl::eventMouseMove));
	addPaintEventHandler(createMethodHandler(this, &ColorGradientControl::eventPaint));

	m_gradientBitmap = gc_new< Bitmap >(256, 256);

	setColor(color, true);
	update();

	return true;
}

Size ColorGradientControl::getPreferedSize() const
{
	return Size(256, 256);
}

void ColorGradientControl::setColor(const Color& color, bool updateCursor)
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

Color ColorGradientControl::getColor() const
{
	return m_gradientBitmap->getPixel(m_cursor.x, m_cursor.y);
}

void ColorGradientControl::addColorSelectEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiColorSelect, eventHandler);
}

void ColorGradientControl::updateGradientImage()
{
	Color color;
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

void ColorGradientControl::eventButtonDown(Event* event)
{
	m_cursor = checked_type_cast< MouseEvent* >(event)->getPosition();
	
	ColorEvent colorEvent(this, 0, getColor());
	raiseEvent(EiColorSelect, &colorEvent);

	update();

	setCapture();
}

void ColorGradientControl::eventButtonUp(Event* event)
{
	releaseCapture();
}

void ColorGradientControl::eventMouseMove(Event* event)
{
	if (!hasCapture())
		return;

	m_cursor = checked_type_cast< MouseEvent* >(event)->getPosition();
	
	if (m_cursor.x < 0)
		m_cursor.x = 0;
	if (m_cursor.x > 255)
		m_cursor.x = 255;
	if (m_cursor.y < 0)
		m_cursor.y = 0;
	if (m_cursor.y > 255)
		m_cursor.y = 255;

	ColorEvent colorEvent(this, 0, getColor());
	raiseEvent(EiColorSelect, &colorEvent);

	update();
}

void ColorGradientControl::eventPaint(Event* event)
{
	PaintEvent* paintEvent = checked_type_cast< PaintEvent* >(event);
	Canvas& canvas = paintEvent->getCanvas();

	canvas.drawBitmap(
		Point(0, 0),
		Point(0, 0),
		Size(256, 256),
		m_gradientBitmap
	);

	Color color = m_gradientBitmap->getPixel(m_cursor.x, m_cursor.y);

	int average = (color.r + color.g + color.b) / 3;
	if (average < 128)
		canvas.setForeground(Color(255, 255, 255));
	else
		canvas.setForeground(Color(0, 0, 0));

	canvas.drawCircle(m_cursor, 5);

	paintEvent->consume();
}

		}
	}
}
